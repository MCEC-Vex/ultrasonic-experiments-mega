#include <Arduino.h>
#include <PacketSerial.h>
#include <NewPing.h>
#include <Servo.h>
#include "serial_definitions.h"

#define SENSOR_COUNT 1
#define SERVO_PIN 10
#define SENSOR_1_TRIGGER 22
#define SENSOR_1_ECHO 23

struct Sensor
{
    NewPing* sensor;
    float offset = 0.0;
    bool enable = false;
    unsigned long interval = 0;
    unsigned long timingOffset = 0;
};

Servo servo;
PacketSerial packetSerial1;
Sensor sensors[SENSOR_COUNT];
unsigned long pauseUntil = 0;

void onPacketReceived(const uint8_t* buffer, size_t size)
{
    PacketHeader header;
    memcpy(&header, buffer, sizeof(PacketHeader));

    Serial.print("Got packet: ");
    Serial.println(header.type);

    switch(header.type)
    {
        case INIT:
        {
            // Send the "capabilities" packet in response
            PacketHeader header;
            header.type = CAPABILITIES;
            CapabilitiesPacket capabilities;
            capabilities.hasMotor = (SERVO_PIN != -1);
            capabilities.sensorCount = SENSOR_COUNT;
            UltrasonicPlacement placement[SENSOR_COUNT];
            placement[0].offset = 0.0;

            uint8_t sendBuffer[sizeof(PacketHeader) + sizeof(CapabilitiesPacket) + (sizeof(UltrasonicPlacement) * SENSOR_COUNT)];
            memcpy(sendBuffer, &header, sizeof(PacketHeader));
            memcpy(sendBuffer + sizeof(PacketHeader), &capabilities, sizeof(CapabilitiesPacket));
            memcpy(sendBuffer + sizeof(PacketHeader) + sizeof(CapabilitiesPacket), &placement, sizeof(UltrasonicPlacement) * SENSOR_COUNT);

            packetSerial1.send(sendBuffer, sizeof(PacketHeader) + sizeof(CapabilitiesPacket) + (sizeof(UltrasonicPlacement) * SENSOR_COUNT));
            
            digitalWrite(LED_BUILTIN, LOW);
            break;
        }
        case SERVO_POS:
        {
            // Move the servo to the specified position
            ServoPosPacket servoPos;
            memcpy(&servoPos, buffer + sizeof(PacketHeader), sizeof(ServoPosPacket));
            servo.write(servoPos.position);
            break;
        }
        case POLL_RATE:
        {
            Serial.println("Adjusting poll rate");
            // Update the poll rate for the specified sensor
            PollRatePacket pollRate;
            memcpy(&pollRate, buffer + sizeof(PacketHeader), sizeof(PollRatePacket));
            if(pollRate.sensor >= SENSOR_COUNT)
            {
                break;
            }

            sensors[pollRate.sensor].enable = pollRate.enable;
            sensors[pollRate.sensor].interval = pollRate.interval;
            sensors[pollRate.sensor].timingOffset = pollRate.offset;
            break;
        }
        case PAUSE_INPUT:
        {
            pauseUntil = millis() + 1000;
            break;
        }
        default:
            break;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting!");

    // Set up servo and sensor(s)
    servo.attach(SERVO_PIN);
    sensors[0].sensor = new NewPing(SENSOR_1_TRIGGER, SENSOR_1_ECHO);

    // Set up serial for packet transfer
    Serial1.begin(115200);
    packetSerial1.setStream(&Serial1);
    packetSerial1.setPacketHandler(onPacketReceived);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{

    if(pauseUntil != 0 && millis() < pauseUntil)
    {
        while(Serial1.available())
        {
            Serial1.read();
        }
    }
    else
    {
        packetSerial1.update();
    }

    // Go through all ultrasonic sensors
    for(unsigned char i = 0; i < SENSOR_COUNT; i++)
    {
        // Check if it's time to ping
        if(sensors[i].enable && (sensors[i].interval % (millis() + sensors[i].timingOffset)) == 0)
        {
            //TODO switch to event-based code
            unsigned int microseconds = sensors[i].sensor->ping();

            // Send the packet
            PacketHeader header;
            header.type = SENSOR_READING;
            SensorReadingPacket reading;
            reading.microseconds = 11;
            reading.sensor = i;
            uint8_t sendBuffer[sizeof(PacketHeader) + sizeof(SensorReadingPacket)];
            memcpy(sendBuffer, &header, sizeof(PacketHeader));
            memcpy(sendBuffer + sizeof(PacketHeader), &reading, sizeof(SensorReadingPacket));

            packetSerial1.send(sendBuffer, sizeof(PacketHeader) + sizeof(SensorReadingPacket));

            //Serial.print("Sensor ");
            //Serial.print(i);
            //Serial.print(" got ");
            //Serial.println(microseconds);

            for(int j = 0; j < sizeof(PacketHeader) + sizeof(SensorReadingPacket); j++)
            {
                Serial.print(j);
                Serial.print(": ");
                Serial.println(sendBuffer[j], BIN);
            }
        }
    }
}