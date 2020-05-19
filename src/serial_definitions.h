#ifndef SERIAL_DEFINITIONS_H
#define SERIAL_DEFINITIONS_H

#include <stdint.h>

enum PacketType : uint8_t
{
    INIT, // Sent by the ESP32 to initialize communication
    CAPABILITIES, // Sent by the Mega in response to initialization
    SERVO_POS, // Sent by the ESP32 to set a servo position
    POLL_RATE, // Sent the ESP32 to set a poll rate for an ultrasonic sensor
    SENSOR_READING, // Sent by the Mega upon a sensor reading or timeout
    PAUSE_INPUT // Sent to pause input for 1 second for non-COBS debug logging
};

struct PacketHeader
{
    PacketType type;
};

struct CapabilitiesPacket
{
    bool hasMotor;
    uint8_t sensorCount;
};

struct UltrasonicPlacement
{
    uint32_t offset;
};

struct ServoPosPacket
{
    uint8_t position;
};

struct PollRatePacket
{
    uint8_t sensor;
    bool enable;
    uint32_t interval;
    uint32_t offset;
};

struct SensorReadingPacket
{
    uint8_t sensor;
    uint8_t microseconds;
};

#endif //SERIAL_DEFINITIONS_H