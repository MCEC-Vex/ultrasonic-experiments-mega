#ifndef SERIAL_DEFINITIONS_H
#define SERIAL_DEFINITIONS_H

#include <stdint.h>

enum PacketType : uint8_t
{
    INIT, // Sent by the ESP32 to initialize communication
    CAPABILITIES, // Sent by the Mega in response to initialization
    SERVO_POS, // Sent by the ESP32 to set a servo position
    POLL_RATE, // Sent the ESP32 to set a poll rate for an ultrasonic sensor
    SENSOR_READING // Sent by the Mega upon a sensor reading or timeout
};

struct PacketHeader
{
    PacketType type;
};

struct CapabilitiesPacket
{
    bool hasMotor;
    unsigned char sensorCount;
};

struct UltrasonicPlacement
{
    float offset;
};

struct ServoPosPacket
{
    unsigned char position;
};

struct PollRatePacket
{
    unsigned char sensor;
    bool enable;
    unsigned long interval;
    unsigned long offset;
};

struct SensorReadingPacket
{
    unsigned char sensor;
    unsigned int microseconds;
};

#endif //SERIAL_DEFINITIONS_H