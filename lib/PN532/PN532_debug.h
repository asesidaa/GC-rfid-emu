#ifndef __DEBUG_H__
#define __DEBUG_H__

//#define DEBUG

#include "Arduino.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL_DEBUG SerialUSB
#else
    #define SERIAL_DEBUG Serial
#endif

#ifdef DEBUG
#define DMSG(args...)       SERIAL_DEBUG.print(args)
#define DMSG_STR(str)       SERIAL_DEBUG.println(str)
#define DMSG_HEX(num)       SERIAL_DEBUG.print(' '); SERIAL_DEBUG.print((num>>4)&0x0F, HEX); SERIAL_DEBUG.print(num&0x0F, HEX)
#define DMSG_INT(num)       SERIAL_DEBUG.print(' '); SERIAL_DEBUG.print(num)
#else
#define DMSG(args...)
#define DMSG_STR(str)
#define DMSG_HEX(num)
#define DMSG_INT(num)
#endif

#endif
