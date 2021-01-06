#include <Arduino.h>
#include <PacketSerial.h>
#include <NewPing.h>
#include <Servo.h>

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
    unsigned long lastTriggered = 0;
};

Servo servo;
PacketSerial packetSerial1;
Sensor sensors[SENSOR_COUNT];
unsigned long pauseUntil = 0;

void sendCapabilities()
{
    Serial.print("capabilities,");
    Serial.print((SERVO_PIN != -1)); // Has motor
    for(int i = 0; i < SENSOR_COUNT; i++)
    {
        Serial.print(",");
        Serial.print(sensors[i].offset);
    }
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting!");

    // Set up servo and sensor(s)
    servo.attach(SERVO_PIN);
    sensors[0].sensor = new NewPing(SENSOR_1_TRIGGER, SENSOR_1_ECHO);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
    while(Serial.available())
    {
        String line = Serial.readStringUntil('\n');
        if(line == "capabilities")
        {
            sendCapabilities();
        }
    }

    // Go through all ultrasonic sensors
    for(unsigned char i = 0; i < SENSOR_COUNT; i++)
    {
        // Check if it's time to ping
        if(sensors[i].enable && (sensors[i].lastTriggered + sensors[i].interval < millis()))
        {
            sensors[i].lastTriggered = millis();
            //TODO switch to event-based code
            unsigned int microseconds = sensors[i].sensor->ping();
            Serial.print("Sensor ");
            Serial.print(i);
            Serial.print(" got ");
            Serial.println(microseconds);
        }
    }
}