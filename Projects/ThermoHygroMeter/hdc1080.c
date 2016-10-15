#include "msp430.h"
#include "hdc1080.h"
#include "i2c.h"

static unsigned int HDC1080_init[] = {HDC1080_ADDR<<1, 0x02, 0x00, 0x00};
static unsigned int HDC1080_get[] = {HDC1080_ADDR<<1, 0x00};
static unsigned int HDC1080_read_buffer[] = {0x00, 0x00, 0x00, 0x00};

unsigned int ack;
unsigned long temp;

void HDC_1080_setup(char setmode)
{
      //__delay_cycles(1000);
  
      P1OUT |= PIN_SCL+PIN_SDA;
      HDC1080_init[2] = setmode;
      i2c_write(4, HDC1080_init);
}

unsigned int HDC_1080_get(char trigger)
{
      HDC1080_get[1] = trigger;
      i2c_write(2, HDC1080_get);
      __delay_cycles(15000);
      ack = i2c_read(HDC1080_ADDR, 2, HDC1080_read_buffer);
      temp = (HDC1080_read_buffer[0]<<8)|HDC1080_read_buffer[1];
      if (trigger == HUMIDITY)
      {
      temp = temp & 0x0000FFFF;
      temp = (temp*100/0x10000);
      }
      else if (trigger == TEMPERATURE)
      {
      temp = ((temp*165/0x10000)-40);
      }
      else if (trigger == CONFIG)
      {}
      return temp;
}
