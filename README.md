# P1Meter2Mqtt
This software provides Smart Meter data from Dutch (and other countries) Energy meters to an MQTT broker. The data is provided by a Serial connector on the meter known as a P1 connector.

## About this Project
I started this project because I wanted to supply this data to a homeassistant instance running on my FreeBSD server. I found one project from [bartwo] which uses an ESP32 which I happen to have, but it crashed. When looking at the code I decided to create my own because of all the issues I saw.

Full documentation of the P1 functionality can be found at: https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf

## Hardware
Hardware is very simple:
- A P1 cable. You can get these in various lengths at e.g. https://allekabels.nl . Search for RJ12 cable. They are sold as telephone cables. 
- A resistor R1 of 10K or 4K7. 10K Did not work for me, the signal got garbled. Replacing with 4K7 fixed it.
- An ESP32 development board. Mine was a DOIT.
- A power supply. A Dutch Smart Meter has an own power supply for the P1, but it can only provide 250 mA. According to documentation the ESP32 can draw up to 750 mA peak. My board whould not boot with the P1 power supply.

## Hardwware Setup

If you bought a standard RJ12 telephone cable it will have two RJ11 connectors. You wil have to cut it at the desired length. The colors of the wires are standardized:
- (1) White
- (2) Black
- (3) Red
- (4) Green
- (5) Yellow
- (6) Blue

There are also reverse connectors! You can check this by measuring the voltage between (1) and (6): (1) should be +5V.
Connect 3.3 V of the ESP32 to (2).
Connect 3.3 V of the ESP32 to One side of R1.
Connect the other side of R1 to (5) and to RX02 of the ESP32.
Connect (6) and (3) to the GND of the ESP32.

## Software

Make sure your Arduino IDE has the correct board selected. The sketch makes use of the PubSubClient MQTT client. See https://pubsubclient.knolleary.net/.
The settings.h contains some specific settings. You will have to adjust it for your network.

### Principle of operation

The smart meter sends the data in sets called telegrams. The consist of a prefix, a set of datablocks and a postfix with a CRC16. The sofware reads an entire telegram in memory, checks the CRC and decodes the blocks.
The blocks in the telegram are seperated by <CR><LF> . Each block starts with an identifier which identifies the metric and is followed by a value.

A telegram is read by the TelegramReader class in TelegramReader.h. While reading the position of the blocks within the telegram is stored. 
After a telegram has been read, all containing blocks are processed by the HandleBlock function in MeterDataDescriptors.ino
This in turn iterates over all known blocks in the MeterDataDescriptors array until it finds the same metric identifier.
It will then extract the value and mark the value inside the telegram so mqtt can send it. 

The MeterDataDescriptors array associates the metric identifier with the mqtt tagname. This array can easily be changed: new metrics can be added and/or other mqtt tags can be used. There are also a number of metrics I don't use. You can easily add them by assigning a mqtt tag and a handler object. 
If adding a completely new entry remeber to insert  BEFORE the last entry. The last entry functions as an end-marker.

The last blocks starting with "0-x:24.1.0" are special. The meter is part of a bus system. The x in the name specifies the unit of the meter on the bus. The "0-x:24.1.0" specifies which type of meter is on that unit. Usualy x=1 identifies the gas meter (type=3), but this is not always the case. This is handled by the HandleDeviceTypeClass. What I found so far is that 3=gas and 4=heat (like in cityheating). I have not been able to find the other type nr's so I just assume that another type is a water meter.

### What is not there

I have not added a secure encrypted MQTT connection. This requires the use of client certificates. It can be done, but it will require some work.

## HomeAssistant

An mqtt.yaml file is added to assist in integration with homeassitant. Add it by adding "mqtt: !include mqtt.yaml" to yout configuration.yaml. You can then configure the special energy page of homeassistant.

## Thanks to

I would like to thank [bartwo] for the general idea.




