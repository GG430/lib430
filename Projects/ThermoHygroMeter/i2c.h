#ifndef __I2C_H__
#define __I2C_H__

//#define NO_STOP (0)
//#define STOP    (1)

#define PIN_SCL   (BIT6)
#define PIN_SDA   (BIT7)

unsigned int i2c_write(unsigned int bytes, const unsigned int* data);
unsigned int i2c_read(unsigned int addr, unsigned int bytes, unsigned int* data);

#endif