#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "settings.h"
#include "globals.h"
#include "MeterDataDescriptor.h"
#include "TelegramContainer.h"
extern TelegramContainer telegram;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/***********************************
            Main Setup
 ***********************************/
void setup()
{
    // Initialize pins
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.begin(BAUD_RATE);
    Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2, true);

#ifdef DEBUG
    Serial.println("Booting - DEBUG mode on");
    blinkLed(2, 500);
    delay(500);
    blinkLed(2, 2000);
    // Blinking 2 times fast and two times slower to indicate DEBUG mode
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
#ifdef DEBUG
        Serial.println("Connection Failed! Rebooting...");
#endif
        delay(5000);
        ESP.restart();
    }
    delay(3000);
    auto blocks=InitializeMeterData();
    telegram.Initialize(blocks+4); // 4 extra vanwege startblock, 2 lege blocks en crc block
    setupOTA();

    mqttClient.setServer(MQTT_HOST, atoi(MQTT_PORT));
    blinkLed(5, 500); // Blink 5 times to indicate end of setup
#ifdef DEBUG
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
}

/***********************************
            Main Loop
 ***********************************/
void loop()
{
    long now = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
        blinkLed(20, 50); // Blink fast to indicate failed WiFi connection
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
#ifdef DEBUG
            Serial.println("Connection Failed! Rebooting...");
#endif
            delay(5000);
            ESP.restart();
        }
    }

    ArduinoOTA.handle();

    if (!mqttClient.connected())
    {
        if (now - LAST_RECONNECT_ATTEMPT > 5000)
        {
            LAST_RECONNECT_ATTEMPT = now;

            if (!mqttReconnect())
            {
#ifdef DEBUG
                Serial.println("Connection to MQTT Failed! Rebooting...");
#endif
                delay(5000);
                ESP.restart();
            }
            else
            {
                LAST_RECONNECT_ATTEMPT = 0;
            }
        }
    }
    else
    {
        mqttClient.loop();
    }

    // Check if we want a full update of all the data including the unchanged data.
    if (now - LAST_FULL_UPDATE_SENT > UPDATE_FULL_INTERVAL)
    {
        SendAll();
        LAST_FULL_UPDATE_SENT=millis();
    }

    if (now - LAST_UPDATE_SENT > UPDATE_INTERVAL)
    {
        if (readTelegram())
        {
            LAST_UPDATE_SENT = millis();
            sendDataToBroker();
        }
    }
}


/**
   Over the Air update setup
*/
void setupOTA()
{
    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            Serial.println("Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            Serial.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("End Failed");
        });

    ArduinoOTA.begin();
}
