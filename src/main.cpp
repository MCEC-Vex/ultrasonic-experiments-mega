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

void sendMeasurement(int sensor, unsigned int microseconds)
{
    Serial.print("measurement,");
    Serial.print(sensor);
    Serial.print(",");
    Serial.println(microseconds);
}

// From https://arduino.stackexchange.com/a/1237
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
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
        else if(line == "ping")
        {
            Serial.println("pong");
        }
        else if(line.startsWith("poll,"))
        {
            int sensor = getValue(line, ',', 1).toInt();
            if(sensor < 0 || sensor >= SENSOR_COUNT)
            {
                continue;
            }
            sensors[sensor].enable = getValue(line, ',', 2) == "true";
            sensors[sensor].interval = getValue(line, ',', 3).toInt();
            sensors[sensor].timingOffset = getValue(line, ',', 4).toInt();
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
            sendMeasurement(i, microseconds);
        }
    }
}