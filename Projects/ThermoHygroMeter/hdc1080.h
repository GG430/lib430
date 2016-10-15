#ifndef __HDC1080_H__
#define __HDC1080_H__

#define HDC1080_ADDR    (0x40)
#define TEMPERATURE     (0)
#define HUMIDITY        (1)
#define CONFIG          (2)
#define SERIAL1         (0xFB)
#define SERIAL2         (0xFC)
#define SERIAL3         (0xFD)
#define MANUFACTURER    (0xFE)
#define DEVICE          (0xFF)

#define HEATER          (0x20)
#define MODE            (0x10)  // 0 = Temp or Humi; 1 = Temp and Humi
#define TRES            (0x04)  // 0 = 14b; 1 = 11b
#define HRES11          (0x01)  // 0 = 14b; 1 = 11b
#define HRES8           (0x02)  // 2 = 8b

void HDC_1080_setup(char setmode);
unsigned int HDC_1080_get(char trigger);

#endif