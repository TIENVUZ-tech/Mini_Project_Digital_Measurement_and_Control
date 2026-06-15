#ifndef FRAME_COMMON_H_
#define FRAME_COMMON_H_

#include <stdint.h>

// Frame format: SOF (1B) | TYPE (1B) | ID (1B) | LEN_H (1B) | LEN_L (1B) | PAYLOAD (0-64B)

#define FRAME_SOF           0xAA

#define FRAME_TYPE_CMD      0x0F // PC  -> MCU
#define FRAME_TYPE_RSP      0x1F // MCU -> PC

#define FRAME_MAX_PAYLOAD   64
#define FRAME_HEADER_SIZE   5 // SOF+TYPE+ID+LEN_H+LEN_L
#define FRAME_MAX_SIZE      (FRAME_HEADER_SIZE + FRAME_MAX_PAYLOAD)

// Group ID (GID) - 4-bit high of ID byte
#define GID_SYSTEM      0x0   // mode, status...
#define GID_SENSOR      0x1   // module Sensor
#define GID_ACTUATOR    0x2   // module Actuator
#define GID_PID         0x3   // module PID
#define GID_COMM        0xF 

// Message ID (MID) - 4-bit low of ID byte
// GID_SYSTEM
#define MID_SET_MODE        0x1
#define MID_STATUS_REPORT   0x2
#define MID_SET_SETPOINT    0x03

// GID_SENSOR
#define MID_SENSOR_REPORT   0x1

// GID_ACTUATOR
#define MID_MANUAL_ACTUATOR 0x1

// GID_PID
#define MID_SET_PID         0x1

// GID_COMM
#define MID_ECHO            0x1

// Response status
#define RSP_STATUS_OK       0x00U
#define RSP_STATUS_FAIL     0xFFU
// Struct frame
typedef struct __attribute__((packed)) {
    uint8_t  sof;
    uint8_t  type;
    uint8_t  id; // GID(4bit) | MID(4bit)
    uint8_t  len_h;
    uint8_t  len_l;
    uint8_t  payload[FRAME_MAX_PAYLOAD];
} Frame_t;

// Union frame
typedef union {
    Frame_t data;
    uint8_t bytes[sizeof(Frame_t)];
} FrameUnion_t;


// Merge 2 bytes (H, L) to uint16_t
static uint16_t Frame_BytesToU16(uint8_t high, uint8_t low)
{
    return (uint16_t)(((uint16_t)high << 8) | (uint16_t)low);
}

// Split uint16_t to 2 bytes
static void Frame_U16ToBytes(uint16_t value, uint8_t *high, uint8_t *low)
{
    *high = (uint8_t)(value >> 8);
    *low  = (uint8_t)(value & 0xFF);
}

// Merge GID + MID
static uint8_t Frame_MakeId(uint8_t gid, uint8_t mid)
{
    return (uint8_t)(((gid & 0x0F) << 4) | (mid & 0x0F));
}

// Split ID -> GID
static uint8_t Frame_GetGid(uint8_t id)
{
    return (uint8_t)((id >> 4) & 0x0F);
}

// Split ID -> MID
static uint8_t Frame_GetMid(uint8_t id)
{
    return (uint8_t)(id & 0x0F);
}

// Get length from a Frame_t
static uint16_t Frame_GetLength(const Frame_t *f)
{
    return Frame_BytesToU16(f->len_h, f->len_l);
}

// Set length into a Frame_t
static void Frame_SetLength(Frame_t *f, uint16_t len)
{
    Frame_U16ToBytes(len, &f->len_h, &f->len_l);
}

#endif
