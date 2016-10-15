#ifndef __MAIN_H__
#define __MAIN_H__

#define Version 0x0101
#define VersionLocation 0x0FFFF-RESET_VECTOR-3
__root static const int x@VersionLocation = Version;

#define ON 1
#define OFF 0
#define display 5
#define number 11
#define letter 4
#define pause 100
#define DCO1MHZ
#define KeyTemp BIT4
#define KeyHumi BIT3
#define LEDTemp BIT0
#define LEDHumi BIT5
#define DOT BIT4

#define ODIR 0x08
#define TDIR 0x04
#define HDIR 0x02

#define LEDT_POUT                                                       (P2OUT)
#define LEDT_PDIR                                                       (P2DIR)
#define LEDT_PIN                                                      (LEDTemp)
#define LEDT_ON                                         (LEDT_POUT |= LEDT_PIN)
#define LEDT_OFF                                       (LEDT_POUT &= ~LEDT_PIN)
#define LEDT_TOGGLE                                     (LEDT_POUT ^= LEDT_PIN)

#define LEDH_POUT                                                       (P2OUT)
#define LEDH_PDIR                                                       (P2DIR)
#define LEDH_PIN                                                      (LEDHumi)
#define LEDH_ON                                         (LEDH_POUT |= LEDH_PIN)
#define LEDH_OFF                                       (LEDH_POUT &= ~LEDH_PIN)
#define LEDH_TOGGLE                                     (LEDH_POUT ^= LEDH_PIN)

// Common anode LEDs required
/*
  a
  -
f| |b
  - g
e| |c
  -
  d

a = 3.0
b = 3.1
c = 3.2
d = 3.3
e = 3.4
f = 3.5
g = 3.6
*/

const char NumberOUT[number] = {
// a      b      c      d      e      f      g
(0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20       ), //0
(       0x02 | 0x04                            ), //1
(0x01 | 0x02 |        0x08 | 0x10 |        0x40), //2
(0x01 | 0x02 | 0x04 | 0x08 |               0x40), //3
(       0x02 | 0x04 |               0x20 | 0x40), //4
(0x01 |        0x04 | 0x08 |        0x20 | 0x40), //5
(0x01 |        0x04 | 0x08 | 0x10 | 0x20 | 0x40), //6
(0x01 | 0x02 | 0x04                            ), //7
(0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40), //8
(0x01 | 0x02 | 0x04 | 0x08 |        0x20 | 0x40), //9
(0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00 | 0x00), //empty
};

const char LetterOUT[letter] = {
// a      b      c      d      e      f      g
(0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40), //B
(0x01 | 0x02 | 0x04 |        0x10 | 0x20 | 0x40), //A
(                     0x08 | 0x10 | 0x20 | 0x40), //t
(0x01 |               0x08 | 0x10 | 0x20 | 0x40), //E
};

const char NumberDIR[display] = {
 0x00, //Dummy
 0x01, //-
 0x02, //H
 0x02, //T
 0x04, //O
};

unsigned long i;
unsigned int j;
unsigned char numberP;
unsigned int Display;

unsigned int humidity;
unsigned int temperature;
unsigned int batteryLow;
unsigned int to;
unsigned int tt;
unsigned int th;
unsigned int ho;
unsigned int ht;
unsigned int hh;

#ifndef pause
unsigned int pause;
#endif

enum states {NORMAL, SET_HOUR, SET_MINUTE};

void configureClocks();
void delaylong(unsigned int k);
void delay(unsigned int k);
void delay_us(unsigned long k);
void sleep(unsigned int time);
void displayTemp();
void displayHumi();
void displayBAt();
void GetData();

#endif