/**********************************************************************************************\
* Filename:     Main.c                                                                         *
* Description:  Sonnenuhr                                                                      *
*                                      MSP430G2553                                             *
*                              /|\  -----------------                                          *
*                               |  |                 |                                         *
*                               ---|DVCC         DVSS|--|                                      *
*                                  |                 |                                         *
*                              L---|P1.0          XIN|---- 32.768kHz from SiT1552              *
*                                  |                 |                                         *
*                              E---|P1.1         XOUT|-                                        *
*                                  |                 |          /|\                            *
*                              D---|P1.2          TST|-          |        o   o RED            *
*                                  |                 |           |        Long Cable           *
*                              S---|P1.3          RST|----R56k----        o   o                *
*                                  |                 |    Mode                                 *
*                                 -|P1.4         P1.7|----/ -----|                             *
*                                  |                 |    Set                                  *
*                                 -|P1.5         P1.6|----/ -----|                             *
*                                  |                 |                                         *
*                                 -|P2.0         P2.5|--|   Sun                                *
*                                  |                 |  |---|>|---|                            *
*                        Moon   |--|P2.1         P2.4|--|                                      *
*                      |--|<|---|  |                 |                                         *
*                               |--|P2.2         P2.3|-----||-----|                            *
*                                  |                 |                                         *
*                                   -----------------                                          *
*                                                                                              *
* The 12 LEDs for the clock are charlieplexed to P1.0 to P1.3                                  *
* 32.768kHz Quartz Crystal on XIN/XOUT                                                         *
* TimerA0 is used for the Capacitive Sensing                                                   *
* TimerA1 is used for the PWM of Sun and Moon LEDs                                             *
*                                                                                              *
* Device:   MSP430G2553                                                                        *
* Version:  1.2.2 - WDT divider 8192 for MEMS                                                  *
* Compiler: IAR Embedded Workbench IDE V.5.60.7                                                *
*                                                                                              *
* COPYRIGHT:                                                                                   *
* Author:   Gerald Gradl                                                                       *
* Date:     03/2014                                                                            *
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

#define Version 0x0122
#define VersionLocation 0x0FFFF-RESET_VECTOR-3
__root static const int x@VersionLocation = Version;

// Charlieplexed LEDs
#define LED_MASK 0x0F // 12 LEDs, 4 IO lines, n = 4, number of LEDS = n x (n-1)
#define LEDs 13       // 13 is a workaround for no 0 in clock from 1 to 12
// 1 - 0x01
// 2 - 0x02
// 3 - 0x04
// 4 - 0x08
// Port Direction
// Example: LED A: Output from P1.0 to P1.1, P1DIR = BIT0 = H and BIT1 = L, both output direction
const char led_dir[LEDs] = {
(0x00 | 0x00), //Dummy
(0x01 | 0x02), //A
(0x02 | 0x01), //B
(0x02 | 0x04), //C
(0x04 | 0x02), //D
(0x01 | 0x04), //E
(0x04 | 0x01), //F
(0x04 | 0x08), //G
(0x08 | 0x04), //H
(0x02 | 0x08), //I
(0x08 | 0x02), //J
(0x01 | 0x08), //K
(0x08 | 0x01)  //L
};

// Port Output
// Example: LED A, Output P1.0 = H, P1.1 = L
const char led_out[LEDs] = {
 0x00, //Dummy
 0x01, //A
 0x02, //B
 0x02, //C
 0x04, //D
 0x01, //E
 0x04, //F
 0x04, //G
 0x08, //H
 0x02, //I
 0x08, //J
 0x01, //K
 0x08, //L
};

unsigned long i;
unsigned int j;
unsigned char LED;
unsigned char hour;
unsigned char hour24;
unsigned char minute;
unsigned char second;
unsigned char LEDPointer;
unsigned char view;
unsigned char bitpointer;
unsigned char DEMOPointer;
unsigned char capsense;
unsigned char LEDState;
unsigned char timer;
unsigned char state;             // Normal, Hour, Minute, Demo

enum states {NORMAL, SET_HOUR, SET_MINUTE, DEMO};

void configureClocks();
void delaylong(unsigned int k);
void delay(unsigned int k);
void delay_us(unsigned long k);
void sleep(unsigned int time);
void update_view(char state);
void LEDOn();
void LEDOff();
void init_PWM_TimerA();
void SunOn();
void SunOff();
void MoonOn();
void MoonOff();
void HourView();
void MinuteView();
void measure_count(void);                   // Measures each capacitive sensor
void pulse_LED(void);                       // LED gradient routine
void CapTouch();
void measure_baseline(void);

/* Sensor settings*/
#define KEY_LVL    300                     // Defines threshold for a key press
/*Set to ~ half the max delta expected*/

#define DCO8MHZ
#define LightTime  5
#define MoonLight  368                     // TimerA PWM lower value
#define SunLight   368                     // TimerA PWM lower value

#define QRZ
//#define MEMS
#ifndef QRZ
#define QRZ
#endif

// Global variables for sensing
unsigned int base_cnt;
unsigned int meas_cnt;
unsigned int meas_cnt_temp;
int delta_cnt;
unsigned char key_press;
char key_pressed;
int cycles;

#define ON 1
#define OFF 0

void main(void)
{
/******************************************************************************/
// Init
/******************************************************************************/
  WDTCTL = WDTPW + WDTHOLD;     // watchdog timer off
  configureClocks();
  init_PWM_TimerA();

  hour = 6;
  hour24 = 6;
  minute = 1;					// Set to one because with 0 no LED is on which is confusing.
  second = 0;
  state = SET_HOUR;
  capsense=0;
  LEDState=OFF;
  timer=0;
  DEMOPointer = 1;

  P1REN = BIT6 + BIT7;          // P1REN = pullup/down enable
  P1OUT = BIT6 + BIT7;          // P1OUT = define pullup
  P1IFG = 0;
  P1IES = BIT6 + BIT7;          // set high to low transition for P1.6&7 interrupt
  P1IE  = BIT6 + BIT7;          // enable P1.6&7 interrupt

  P1REN |= BIT4 + BIT5;         // P1REN = pullup/down enable
  P1OUT |= BIT4 + BIT5;         // P1OUT = pullup
  P2REN |= BIT0;                // P2REN = pullup/down enable
  P2OUT |= BIT0;                // P2OUT = pullup
  //P2OUT |= BIT3;
  //P2DIR |= BIT3;
  P3REN = 0xFF;                 // Tie all P3 ports which are not available on N package
  P3OUT = 0xFF;

  WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTSSEL + WDTIS0; // watchdog counter mode, ACLK, /32768
  IFG1 &= ~WDTIFG;              // Clear WDT interrupt flag
  IE1 |= WDTIE;                 // WDT interrupt enable

  measure_baseline();


/******************************************************************************/
// Mainloop
/******************************************************************************/

  __enable_interrupt();

  while(1)
  {
      switch(state)
        {
          case NORMAL:
            if (LEDState==ON)
            {
                  HourView();
            }
            if (LEDState==ON && ((hour24>=7) && (hour24<18)))
                {
                  MoonOff();
                  SunOn();
                }
            if (LEDState==ON && ((hour24>=18) || (hour24<7)))
                {
                  SunOff();
                  MoonOn();
                }
            if (capsense)
            {
              CapTouch();
            }
            break;
          case SET_HOUR:
            capsense=OFF;
              MoonOff();
              SunOff();
              HourView();
            break;
          case SET_MINUTE:
            capsense=OFF;
              MoonOff();
              SunOff();
              MinuteView();
            break;
          case DEMO:
            if (DEMOPointer==1)
             {
               MoonOn();
               SunOff();
             }
             if (DEMOPointer==7)
             {
               MoonOff();
               SunOn();
             }
             j = 5000/DEMOPointer;
             for(i=0;i<j;i++)
             {
               unsigned int OnTime = 10*DEMOPointer;
               unsigned int OffTime = 120/DEMOPointer;
                 for(LEDPointer = 1;LEDPointer <= DEMOPointer;LEDPointer++)
                 {
                     LEDOn();
                     delay(OnTime);
                     LEDOff();
                     delay(OffTime);
                 }
             }
               DEMOPointer++;
               if (DEMOPointer==13)
               {
               DEMOPointer=1;
               }
            break;
        }
  }
} // main()


/******************************************************************************/
// Subroutines
/******************************************************************************/
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
    }

#ifdef MEMS
BCSCTL3 |= LFXT1S_3;                        // 32kHz external source for ACLK
#endif
#ifdef QRZ
BCSCTL3 &= ~XCAP0;                          // 10pF Caps
BCSCTL3 |= XCAP1;                           // 10pF Caps
#endif
//P1DIR |= BIT0;                            // Debug: ACLK @ P1.0
//P1SEL |= BIT0;                            // Debug: ACLK @ P1.0

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
  unsigned long i;
  
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
  
  delay = (((delay-14)/3)+1);

  for(i=0;i<delay;i++);
}

/******************************************************************************/
void clock(void)
{
  if (second == 60)
  {
    minute++;
    second = 0;
  }
  if (minute == 60)
  {
    hour++;
    hour24++;
    minute = 0;
    second = 0;
  }
  if (hour == 13)
  {
    hour = 1;
    minute = 0;
    second = 0;
  }
  if (hour24 == 24)
  {
  hour24 = 0;
  }
}

void LEDOn()
{
      P1DIR |= led_dir[LEDPointer];
      P1OUT |= led_out[LEDPointer];
}

void LEDOff()
{

     P1DIR &= (~LED_MASK);
     P1OUT &= (~LED_MASK);
}

/******************************************************************************/
void init_PWM_TimerA(void)
{
  //TA1CTL = TASSEL_2 + ID_3 + TACLR;                                              // SMCLK, /8, Clear Tar
  TA1CTL = TASSEL_2 + TACLR;                                                       // SMCLK, Clear Tar
  //TA1CTL = TASSEL_1 + TACLR;                                                     // SMCLK, Clear Tar
  TA1CCR0 = 512-1;                                                                 // PWM Period
  //Sonne
  TA1CCR2 = SunLight;                                                              // CCR1 PWM duty cycle
  TA1CCTL2 = OUTMOD_3;                                                             // CCR1 reset/set
  //Mond
  TA1CCR1 = MoonLight;                                                             // CCR1 PWM duty cycle
  TA1CCTL1 = OUTMOD_3;                                                             // CCR1 reset/set
  TA1CTL |= MC_1;
}

/******************************************************************************/
void SunOn(void)
{
  P2SEL2 &= ~BIT4 + ~BIT5;
  P2SEL |= BIT4 + BIT5;
  P2DIR |= BIT4 + BIT5;
}
void MoonOn(void)
{
  P2SEL2 &= ~BIT1 + ~BIT2;
  P2SEL |= BIT1 + BIT2;
  P2DIR |= BIT1 + BIT2;
}
void SunOff(void)
{
  P2SEL &= ~BIT4 + ~BIT5;
  P2OUT &= ~BIT4 + ~BIT5;
  //TA1CTL &= ~MC_1;
}
void MoonOff(void)
{
  P2SEL &= ~BIT1 + ~BIT2;
  P2OUT &= ~BIT1 + ~BIT2;
  //TA1CTL &= ~MC_1;
}

/******************************************************************************/
void MinuteView(void)
  {
       unsigned int OnTime = 10*minute;
       unsigned int OffTime = 120/minute;
         for(LEDPointer=1, bitpointer=1;LEDPointer <= 6;LEDPointer++)
         {
           if(minute & bitpointer)
           {
             LEDOn();
           }
             delay(OnTime);
           if(!(minute & bitpointer))
           {
             LEDOff();
           }
             delay(OffTime);

            // All LEDs Off
             P1DIR &= (~LED_MASK);
             P1OUT &= (~LED_MASK);

           bitpointer<<=1;
           if (bitpointer==64) bitpointer=1;
         }
  }

/******************************************************************************/
void HourView(void)
  {
    j = 5000/hour;
     for(i=0;i<j;i++)
     {
       unsigned int OnTime = 10*hour;
       unsigned int OffTime = 120/hour;
         for(LEDPointer = 1;LEDPointer <= hour;LEDPointer++)
         {
             LEDOn();
             delay(OnTime);
             LEDOff();
             delay(OffTime);
         }
     }
  }

/******************************************************************************/
void CapTouch(void)
{
    measure_count();

    meas_cnt_temp = meas_cnt;

	  	for(i=5; i>0; i--)
	  	  {
	    	 measure_count();
	      	 meas_cnt_temp = (meas_cnt+meas_cnt_temp)/2;
	      }

    meas_cnt = meas_cnt_temp;

    delta_cnt = base_cnt - meas_cnt;

    if (delta_cnt < 0)
      {
         base_cnt = (base_cnt+meas_cnt) >> 1;
      }
    else if (delta_cnt > KEY_LVL)
      {
          capsense = OFF;
          LEDState = ON;
      }
    else
      {
          base_cnt = base_cnt - 1;
          key_pressed = OFF;
      }
}

/******************************************************************************/
void measure_count(void)
{
    _DINT();                              	// Disable interrupts
    P2DIR &= ~BIT3;           			//
    P2SEL &= ~BIT3;           			//
    P2SEL2 |= BIT3;            			// Set target Pin Oscillator on P2.3
    TA0CTL = TASSEL_3 + ID_0 + TACLR;           // Select INCLK which is the pinosc in this case
    TA0CTL |= MC_2;				// Start Timer
    delay(15000);				// Make artificial delay to capture capacitive sensing oscillator edges while SMCLK runs TimerA

    TA0CTL &= ~MC_2;                    	// Halt Timer
    meas_cnt = TAR;                             // Current TimerA Register holds # of edges captures of pin oscillator
    P2SEL2 &= ~BIT3;           			// Clear target Pin Oscillator on P2.3
    _EINT();					// Re-enable interrupts
}

/******************************************************************************/
void measure_baseline(void)
{
  measure_count();              // Establish baseline capacitance
    base_cnt = meas_cnt;

  for(i=15; i>0; i--)           // Repeat and avg base measurement
  {
    measure_count();
      base_cnt = (meas_cnt+base_cnt)/2;
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
  second++;
  clock();
  if(capsense==OFF && LEDState==ON && !(hour24==7 || hour24==18))
  {
    timer++;
      if(timer==LightTime)
        {
          LEDState=OFF;
          capsense=ON;
          timer=0;
          MoonOff();
          SunOff();
          delay(1000);
        }
  }

  if(hour24==7 || hour24==18)
  {
	  LEDState=ON;
          capsense=OFF;
  }
  if((hour24==8 && minute==0 && second==0) || (hour24==19 && minute==0 && second==0))
  {
          LEDState=OFF;
          capsense=ON;
          timer=0;
          MoonOff();
          SunOff();
  }
  if(capsense==ON && ((hour24>=8 && hour24<18) || (hour24>=19 || hour24<7)))
  {
	  LEDState=OFF;
          MoonOff();
          SunOff();
  }
}

/******************************************************************************/
// Port 1 Interrupt
/******************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void ISR_Port1(void)
{
  unsigned int PIF;
     P1DIR &= (~LED_MASK);
     P1OUT &= (~LED_MASK);
  delay_us(200000);     // 200ms debounce
  PIF = P1IFG;
  P1IFG &= ~BIT6 & ~BIT7;
    if (PIF & BIT6)
    {
      if (state == NORMAL)
      {

      }
      else if (state == SET_HOUR)
      {
      hour++;
      hour24++;
        if (hour > 12)
        {
        hour = 1;
        }
        if (hour24 == 24)
        {
        hour24 = 0;
        }
      }
      else if (state == SET_MINUTE)
      {
      minute++;
        if (minute >= 60)
        {
        minute = 0;
        }
      // second = 0;
      }
      else {}
    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (PIF & BIT7)
    {
      state = (state+1)%4;     // auto increment state and check if state is >2 then automatically reset to 0
      if (state == NORMAL)
      {
        capsense=ON;
        LEDState=ON;
        MoonOff();
        SunOff();
      }
      else
      {
    	capsense=OFF;
      }
    }
    PIF = 0;
}