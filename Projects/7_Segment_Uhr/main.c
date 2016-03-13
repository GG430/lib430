/**********************************************************************************************\
* Filename:     Main.c                                                                         *
* Description:  7SEGUHR                                                                        *
*                                      MSP430G2553                                             *
*                              /|\  -----------------                                          *
*                               |  |                 |                                         *
*                               ---|DVCC         DVSS|--|                                      *
*                                  |                 |                                         *
*                                 -|P1.0          XIN|---- 32.768kHz                           *
*                                  |                 |   o                                     *
*                                 -|P1.1         XOUT|----                                     *
*                                  |                 |          /|\                            *
*                                 -|P1.2          TST|----       |        o   o RED            *
*                                  |                 |           |        Long Cable           *
*                       M  |--/ ---|P1.3          RST|----R56k----        o   o                *
*                                  |                 |                                         *
*                       H  |--/ ---|P1.4         P1.7|--SDA                                    *
*                                  |                 |                                         *
*                       SD <-------|P1.5         P1.6|--SCL                                    *
*                                  |                 |                                         *
*                               b--|P3.1         P3.7|-                                        *
*                                  |                 |                                         *
*                               a--|P3.0         P3.6|--g                                      *
*                                  |                 |                                         *
*                              HT--|P2.0         P3.5|--f                                      *
*                                  |                 |                                         *
*                              HO--|P2.1         P2.5|--DP                                     *
*                                  |                 |                                         *
*                              MT--|P2.2         P2.4|--P                                      *
*                                  |                 |                                         *
*                               c--|P3.2         P2.3|-MO                                      *
*                                  |                 |                                         *
*                               d--|P3.3         P3.4|-e                                       *
*                                  |                 |                                         *
*                                   -----------------                                          *
*                                                                                              *
* Device:   MSP430G2553                                                                        *
* Version:  1.0.2                                                                              *
* Compiler: IAR Embedded Workbench IDE V.6.40.1                                                *
*                                                                                              *
* COPYRIGHT:                                                                                   *
* Author:   Gerald Gradl                                                                       *
* Date:     02/2015                                                                            *
*                                                                                              *
* This program is free software: you can redistribute it and/or modify it under the terms of   *
* the GNU General Public License as published by the Free Software Foundation, either          *
* version 3 of the License, or (at your option) any later version.                             *
*                                                                                              *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;    *
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    *
* See the GNU General Public License for more details.                                         *
*                                                                                              *
* You should have received a copy of the GNU General Public License along with this program.   *
* If not, see <http://www.gnu.org/licenses/>.                                                  *
\**********************************************************************************************/

//#include  "msp430g2553.h"
#include <msp430.h>

#define Version 0x0102
#define VersionLocation 0x0FFFF-RESET_VECTOR-3
__root static const int x@VersionLocation = Version;

#define ON 1
#define OFF 0
#define display 5
#define number 10
#define OnTime 100   //us
#define DCO8MHZ

#define MinuteODIR 0x08
#define MinuteTDIR 0x04
#define HourODIR 0x02
#define HourTDIR 0x01
#define ColonDIR 0x20

#define cAnode
//#define cCathode
#define MEMS
//#define QUARTZ

//#define ACLKDEBUG


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
};

const char NumberDIR[display] = {
 0x00, //Dummy
 0x01, //HT
 0x02, //HO
 0x02, //MT
 0x04, //MO
};

unsigned long i;
unsigned int j;
unsigned char HourT;
unsigned char HourO;
unsigned char MinuteT;
unsigned char MinuteO;
unsigned char Second;
unsigned char numberP;
unsigned char displayP;
unsigned int count;

#ifndef OnTime
unsigned int OnTime;
#endif

enum states {NORMAL, SET_HOUR, SET_MINUTE};

void configureClocks();
void delaylong(unsigned int k);
void delay(unsigned int k);
void delay_us(unsigned long k);
void sleep(unsigned int time);
void displayTime();


void main(void)
{
/******************************************************************************/
// Init
/******************************************************************************/
  WDTCTL = WDTPW + WDTHOLD;     // watchdog timer off
  configureClocks();

#ifdef ACLKDEBUG

  P1DIR |= BIT0;
  P1SEL |= BIT0;
  while (1)
  {}
#endif

  P1REN |= BIT0 + BIT1 + BIT2 + BIT5 + BIT6 + BIT7;
  P3REN |= BIT7;

#ifdef cAnode
  P3DIR = 0x00;
  P3OUT = 0x00;
  P2OUT = 0x3F;
  P2DIR |= ColonDIR+HourTDIR+HourODIR+MinuteTDIR+MinuteODIR;
#endif

#ifdef cCathode
  P3DIR = 0xFF;
  P3OUT = 0x00;
  P2OUT = 0x20;
  P2DIR |= ColonDIR;
#endif

  P1REN |= BIT3 + BIT4;          // P1REN = pullup/down enable
  P1OUT = BIT3 + BIT4;          // P1OUT = define pullup
  P1IFG = 0;
  P1IES = BIT3 + BIT4;          // set high to low transition for P1.3&4 interrupt
  P1IE  = BIT3 + BIT4;          // enable P1.3&4 interrupt

  HourT = 0;
  HourO = 0;
  MinuteT = 0;
  MinuteO = 0;
  Second = 0;

  TACTL = TASSEL_2 + TACLR + TAIE;
  TACCTL0 = CCIE;

  WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTSSEL; // watchdog counter mode, ACLK, /32768
  IFG1 &= ~WDTIFG;              // Clear WDT interrupt flag
  IE1 |= WDTIE;                 // WDT interrupt enable

/******************************************************************************/
// Mainloop
/******************************************************************************/

  __enable_interrupt();

  while(1)
  {
     displayTime();
  }
} // main()


/******************************************************************************/
// Subroutines
/******************************************************************************/
void displayTime()
{
#ifdef cAnode
	numberP = MinuteO;
    P2OUT |= MinuteODIR;
    P3DIR |= NumberOUT[numberP];
      delay_us(OnTime);
    P2OUT &= ~MinuteODIR;
    P3DIR &= ~NumberOUT[numberP];

    numberP = MinuteT;
    P2OUT |= MinuteTDIR;
    P3DIR |= NumberOUT[numberP];
      delay_us(OnTime);
    P2OUT &= ~MinuteTDIR;
    P3DIR &= ~NumberOUT[numberP];

    numberP = HourO;
    P2OUT |= HourODIR;
    P3DIR |= NumberOUT[numberP];
      delay_us(OnTime);
    P2OUT &= ~HourODIR;
    P3DIR &= ~NumberOUT[numberP];

    numberP = HourT;
    P2OUT |= HourTDIR;
    P3DIR |= NumberOUT[numberP];
      delay_us(OnTime);
    P2OUT &= ~HourTDIR;
    P3DIR &= ~NumberOUT[numberP];
#endif

#ifdef cCathode
    numberP = MinuteO;
    P2DIR |= MinuteODIR;
    P3OUT |= NumberOUT[numberP];
      delay_us(OnTime);
    P2DIR &= ~MinuteODIR;
    P3OUT &= ~NumberOUT[numberP];

    numberP = MinuteT;
    P2DIR |= MinuteTDIR;
    P3OUT |= NumberOUT[numberP];
      delay_us(OnTime);
    P2DIR &= ~MinuteTDIR;
    P3OUT &= ~NumberOUT[numberP];

    numberP = HourO;
    P2DIR |= HourODIR;
    P3OUT |= NumberOUT[numberP];
      delay_us(OnTime);
    P2DIR &= ~HourODIR;
    P3OUT &= ~NumberOUT[numberP];

    numberP = HourT;
    P2DIR |= HourTDIR;
    P3OUT |= NumberOUT[numberP];
      delay_us(OnTime);
    P2DIR &= ~HourTDIR;
    P3OUT &= ~NumberOUT[numberP];
#endif

}

void configureClocks()
{
if (CALBC1_1MHZ != 0xFF && CALBC1_8MHZ != 0xFF && CALBC1_12MHZ != 0xFF && CALBC1_16MHZ != 0xFF)
    {                                           // Security feature for no DCO Cal present
      BCSCTL1 = 0;                                    // Bug Fix
      #ifdef DCO1MHZ                                  // Set DCO to 1MHz calibration
      BCSCTL1 = CALBC1_1MHZ;
      DCOCTL = CALDCO_1MHZ;
      #endif

      #ifdef DCO8MHZ                                  // Set DCO to 8MHz calibration
      BCSCTL1 = CALBC1_8MHZ;
      DCOCTL = CALDCO_8MHZ;
      #endif

      #ifdef DCO12MHZ                                 // Set DCO to 12MHz calibration
      BCSCTL1 = CALBC1_12MHZ;
      DCOCTL = CALDCO_12MHZ;
      #endif

      #ifdef DCO16MHZ                                 // Set DCO to 16MHz calibration
      BCSCTL1 = CALBC1_16MHZ;
      DCOCTL = CALDCO_16MHZ;
      #endif
    } // if Cal not present

#ifdef MEMS
BCSCTL3 |= LFXT1S_3;                        // 32kHz external source for ACLK
#endif

#ifdef QUARTZ
// BCSCTL3 = LFXT1S_0 + XCAP_3; // 32768KHz crystal, 12.5 pF
// BCSCTL3 = LFXT1S_0 + XCAP_2; // 32768KHz crystal, 10 pF
BCSCTL3 = LFXT1S_0 + XCAP_1; // 32768KHz crystal, 6 pF
// BCSCTL3 = LFXT1S_0 + XCAP_0; // 32768KHz crystal, 1 pF
#endif

//BCSCTL2 |= DIVS_1;

 do
  {
    BCSCTL3 &= ~LFXT1OF;                     // Clear OSCFault flag
    for (i = 0x1FFF; i > 0; i--);             // Time for flag to set
  }
 while(BCSCTL3 & LFXT1OF);

}
/******************************************************************************/
void delaylong(unsigned int k)
{
  unsigned int i,j;
  for(i=0;i<k;i++)
  {
    j=0;
    do
    j++;
    while(j<65000);
  }
}
/******************************************************************************/
void delay(unsigned int k)
{
  unsigned int i;
  for(i=0;i<k;i++);
}
/******************************************************************************/
void delay_us(unsigned long delay)
{

  #ifdef DCO1MHZ
  delay = 1;
  #endif

  #ifdef DCO8MHZ
  delay = delay*8;
  #endif

  #ifdef DCO12MHZ
  delay = delay*12;
  #endif

  #ifdef DCO16MHZ
  delay = delay*16;
  #endif

  while(--delay)
  {_NOP();}

} // delay_us()

/******************************************************************************/
void clock(void)
{
  if (Second == 60)
  {
    MinuteO++;
    Second = 0;
  }
  if (MinuteO == 10)
  {
	MinuteT++;
    MinuteO = 0;
  }
  if (MinuteT == 6)
  {
	HourO++;
	MinuteT = 0;
  }
  if (HourO == 10)
  {
  	HourT++;
  	HourO = 0;
  }
  if (HourT == 2 && HourO == 4)
  {
	HourO = 0;
	HourT = 0;
  }

}



/******************************************************************************/
// Interrupt Routines
/******************************************************************************/

/******************************************************************************/
// WDT Interrupt Routine
/******************************************************************************/
#pragma vector=WDT_VECTOR
__interrupt void ISR_WDT(void)
{
  IFG1 &= ~WDTIFG;              // Clear WDT interrupt flag
  Second++;
  P2OUT ^= ColonDIR;
  clock();
}

/******************************************************************************/
// TA Interrupt Routine
/******************************************************************************/
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
  switch( TA0IV )
  {
  case  2: // TACCR1
           break;
  case  4: // TACCR2
           break;
  case 10: // TAIFG (Overflow | TACCR0)

            if (!(P1IN & BIT3))
            {
              if (count == 20)
              {
                MinuteO++;
                Second = 0;
                  if (MinuteO == 10)
                  {
                    MinuteO = 0;
                    MinuteT++;
                  }
                  if (MinuteT == 6)
                  {
                    MinuteO = 0;
                    MinuteT = 0;
                  }
              }
              if (count == 100)
              {
                MinuteO++;
                Second = 0;
                  if (MinuteO == 10)
                  {
                    MinuteO = 0;
                    MinuteT++;
                  }
                  if (MinuteT == 6)
                  {
                    MinuteO = 0;
                    MinuteT = 0;
                  }
                count = 21;
              }
            }

            else if (!(P1IN & BIT4))
            {
              if (count == 20)
              {
                HourO++;
                if (HourO == 10)
                {
                  HourO = 0;
                  HourT++;
                }
                if (HourT == 2 && HourO == 4)
                {
                  HourO = 0;
                  HourT = 0;
                }
              }
              if (count == 100)
              {
                HourO++;
                if (HourO == 10)
                {
                  HourO = 0;
                  HourT++;
                }
                if (HourT == 2 && HourO == 4)
                {
                  HourO = 0;
                  HourT = 0;
                }
                count = 21;
              }
            }

            else if (P1IN & 0x18)
            {
            TACTL &= ~MC_1; //Stop timerA
            }
            count++;
           break;
 }
}

/******************************************************************************/
// P1 Interrupt Routine
/******************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  TACCR0 = 16000;
  TACTL |= MC_1 + TACLR; //Timer starts in up mode counting to TACCR0
  count = 0;
  P1IFG &= ~BIT3 + BIT4;
}