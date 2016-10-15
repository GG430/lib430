#include "msp430.h"
#include "i2c.h"

#define SDA_lo P1DIR |= PIN_SDA; \
               P1OUT &= ~PIN_SDA;
#define SDA_hi P1OUT |= PIN_SDA; \
               P1DIR &= ~PIN_SDA;
#define SCL_lo P1DIR |= PIN_SCL; \
               P1OUT &= ~PIN_SCL;
#define SCL_hi P1OUT |= PIN_SCL; \
               P1DIR &= ~PIN_SCL;
#define SDA_sense (P1IN & PIN_SDA)

#define START SDA_lo; \
              SCL_lo;

#define STOP  SDA_lo; \
              SCL_hi; \
              SDA_hi;

static void read_byte(unsigned int *data, unsigned int ack);
static unsigned int write_byte(unsigned int data);

unsigned int i2c_write(unsigned int bytes, const unsigned int* data)
{
  unsigned int nack=0;
  
  START;
  while(bytes-- && (nack==0)) {
    nack = write_byte(*data++);
  }
  STOP;
  return nack;
}

unsigned int i2c_read(unsigned int addr, unsigned int bytes, unsigned int* data)
{
  unsigned char nack=0,i=0;
  
  START;
  nack = write_byte((addr<<1)|1);
  //nack = write_byte(addr);
  if (nack==0) {
    while(--bytes) {
      read_byte(&data[i++], 0);
    }
    read_byte(&data[i++], 1);
  }
  STOP;
  return nack;
}

static unsigned int write_byte(unsigned int data)
{
  unsigned char i, nack;
  
  for(i=0; i<8; i++) {
    if (data & 0x80) { SDA_hi; } else { SDA_lo; }
    data <<= 1;
    SCL_hi;
    SCL_lo;
  }
  SDA_hi;
  SCL_hi;
  nack = SDA_sense;
  SCL_lo;
  return nack;
}

static void read_byte(unsigned int *data, unsigned int ack)
{
  unsigned char i, tmp;
  
  tmp=0;
  for(i=0; i<8; i++) {
    SCL_hi;
    tmp <<= 1;
    tmp |= (SDA_sense!=0);
    SCL_lo;
  }
  
  if (ack) { SDA_hi; } else { SDA_lo; }
  SCL_hi;
  *data=tmp;
  SCL_lo;
  SDA_hi;
}
