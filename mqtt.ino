#include "MeterDataDescriptor.h"
#include "globals.h"

extern MeterDataDescriptor MeterDataDescriptors[];

bool mqttReconnect()
{
    int MQTT_RECONNECT_RETRIES = 0;

    while (!mqttClient.connected() && MQTT_RECONNECT_RETRIES < MQTT_MAX_RECONNECT_TRIES)
    {
        MQTT_RECONNECT_RETRIES++;

        if (mqttClient.connect(HOSTNAME, MQTT_USER, MQTT_PASS))
        {
            mqttClient.publish("hass/status", "p1 meter alive: "HOSTNAME);
        }
        else
        {
            delay(5000);
        }
    }

    if (MQTT_RECONNECT_RETRIES >= MQTT_MAX_RECONNECT_TRIES)
    {
        return false;
    }

    return true;
}


void sendDataToBroker()
{
    auto meterData=MeterDataDescriptors;
    while (meterData->Code!=nullptr)
    {
        if (meterData->Send)
        {
            if (meterData->ValueString!=nullptr)
            {
#ifdef DEBUG
                Serial.printf("Sending: %s value: %s\n",meterData->Name,meterData->ValueString);        
#endif
                if (mqttClient.publish(meterData->Name,meterData->ValueString, false))
                  meterData->Send=false;
            }
            else
              meterData->Send=false;

        }
        meterData++;
    }
}
