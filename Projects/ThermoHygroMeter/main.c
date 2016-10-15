/**********************************************************************************************\
* Filename:     main.c                                                                         *
* Description:  Thermo/Hygrometer                                                              *
*                                      MSP430G2553                                             *
*                              /|\  -----------------                                          *
*                               |  |                 |                                         *
*                               ---|DVCC         DVSS|--|                                      *
*                                  |                 |                                         *
*                         TSIC -> -|P1.0          XIN|---- 32.768kHz                           *
*                                  |                 |   o                                     *
*                        TSIC PWR -|P1.1         XOUT|----                                     *
*                                  |                 |          /|\                            *
*                                 -|P1.2          TST|----       |        o   o RED            *
*                                  |                 |           |        Long Cable           *
*                  SENSOR  |--/ ---|P1.3          RST|----R56k----        o   o                *
*                                  |                 |                                         *
*                 TEMP/RH  |--/ ---|P1.4         P1.7|--SDA                                    *
*                                  |                 |                                         *
*                                 -|P1.5         P1.6|--SCL                                    *
*                                  |                 |                                         *
*                               b--|P3.1         P3.7|-                                        *
*                                  |                 |                                         *
*                               a--|P3.0         P3.6|--g                                      *
*                          DEGC    |                 |                                         *
*                     |----|<|-----|P2.0         P3.5|--f                                      *
*                                  |                 |     RH%                                 *
*                              HO--|P2.1         P2.5|-----|>|----|                            *
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
* Version:  1.0.1                                                                              *
* Compiler: IAR Embedded Workbench IDE V.6.40.1                                                *
*                                                                                              *
* COPYRIGHT:                                                                                   *
* Author:   Gerald Gradl                                                                       *
* Date:     10/2016                                                                            *
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

#include <msp430.h>
#include "main.h"
#include "hdc1080.h"

void main(void)
{
/******************************************************************************/
// Init
/******************************************************************************/
  WDTCTL = WDTPW + WDTHOLD;     		// watchdog timer off
  configureClocks();

  P3DIR = 0x00+BIT7;
  P3OUT = 0x00;
  P2OUT = 0xFF-HDIR-TDIR-ODIR;
  P2DIR |= DOT+HDIR+TDIR+ODIR+BIT6+BIT7;
  P1REN |= KeyTemp + KeyHumi;          // P1REN = pullup/down enable
  P1OUT |= KeyTemp + KeyHumi;          // P1OUT = define pullup
  P1IFG = 0;
  P1IES |= KeyTemp + KeyHumi;          // set high to low transition for Key interrupt
  P1IE  |= KeyTemp + KeyHumi;          // enable Key interrupt
  P1REN |= BIT0 + BIT1 + BIT2 + BIT5;
  P1OUT |= BIT0 + BIT1 + BIT2 + BIT5;
  P2DIR |= LEDTemp+LEDHumi;
  P2OUT |= LEDTemp+LEDHumi;

  LEDT_OFF;
  LEDH_OFF;

  __delay_cycles(10000);

  HDC_1080_setup(0);

/******************************************************************************/
// Mainloop
/******************************************************************************/

  __enable_interrupt();

  while(1)
  {
      __bis_SR_register(LPM4_bits + GIE);
      _NOP();

      batteryLow = HDC_1080_get(CONFIG);

      if (batteryLow)
      {
       __disable_interrupt();
      LEDT_OFF;
      LEDH_OFF;
      P1IFG &= ~(KeyTemp + KeyHumi);
      for (long i = 4000;i>0;i--)
            {
            displayBAt();
            }
       __enable_interrupt();
      }
      else
      {

      if (Display == KeyTemp)
      {
      __disable_interrupt();
      LEDT_ON;
      for (long i = 2000;i>0;i--)
            {
            displayTemp();
            }
      LEDT_OFF;
      P1IFG &= ~(KeyTemp + KeyHumi);
      __enable_interrupt();
      }
      else if (Display == KeyHumi)
      {
      __disable_interrupt();
      LEDH_ON;
      for (long i = 2000;i>0;i--)
            {
            displayHumi();
            }
      LEDH_OFF;
      P1IFG &= ~(KeyTemp + KeyHumi);
      __enable_interrupt();
      }
      }
  }
} // main()

/******************************************************************************/
// Subroutines
/******************************************************************************/
void displayTemp()
{
    numberP = to;
    P2OUT |= ODIR;
    P3DIR |= NumberOUT[numberP];
      delay(pause);
    P2OUT &= ~ODIR;
    P3DIR &= ~NumberOUT[numberP];

    numberP = tt;
    P2OUT |= TDIR;
    P3DIR |= NumberOUT[numberP];
      delay(pause);
    P2OUT &= ~TDIR;
    P3DIR &= ~NumberOUT[numberP];

    if (th == 0)
    {
    P2OUT |= HDIR;
    P3DIR |= NumberOUT[10];
      delay(pause);
    P2OUT &= ~HDIR;
    P3DIR &= ~NumberOUT[10];
    }
    else if (th == 1)
    {
    P2OUT |= HDIR;
    P3DIR |= NumberOUT[1];
      delay(pause);
    P2OUT &= ~HDIR;
    P3DIR &= ~NumberOUT[1];
    }
}
/******************************************************************************/
void displayHumi()
{
    numberP = ho;
    P2OUT |= ODIR;
    P3DIR |= NumberOUT[numberP];
      delay(pause);
    P2OUT &= ~ODIR;
    P3DIR &= ~NumberOUT[numberP];

    numberP = ht;
    P2OUT |= TDIR;
    P3DIR |= NumberOUT[numberP];
      delay(pause);
    P2OUT &= ~TDIR;
    P3DIR &= ~NumberOUT[numberP];

    if (th == 0)
    {
    P2OUT |= HDIR;
    P3DIR |= NumberOUT[10];
      delay(pause);
    P2OUT &= ~HDIR;
    P3DIR &= ~NumberOUT[10];
    }
    else if (hh == 1)
    {
    P2OUT |= HDIR;
    P3DIR |= NumberOUT[1];
      delay(pause);
    P2OUT &= ~HDIR;
    P3DIR &= ~NumberOUT[1];
    }
}
/******************************************************************************/
void displayBAt()
{
	// This displays BAt on the 7 segment LEDs but depending on the bat detect of the HDC1080 this is too much
  /*
    P2OUT |= ODIR;
    P3DIR |= LetterOUT[2];
      delay(pause);
    P2OUT &= ~ODIR;
    P3DIR &= ~LetterOUT[2];

    P2OUT |= TDIR;
    P3DIR |= LetterOUT[1];
      delay(pause);
    P2OUT &= ~TDIR;
    P3DIR &= ~LetterOUT[1];

    P2OUT |= HDIR;
    P3DIR |= LetterOUT[0];
      delay(pause);
    P2OUT &= ~HDIR;
    P3DIR &= ~LetterOUT[0];
  */

	// This displays __E on the 7 segment LEDs
    P2OUT |= ODIR;
    P3DIR |= LetterOUT[3];
      delay(pause);
    P2OUT &= ~ODIR;
    P3DIR &= ~LetterOUT[3];

}
/******************************************************************************/
void GetData()
{
      temperature = HDC_1080_get(TEMPERATURE);
      to = temperature %10;
      tt = temperature /10;
      th = temperature /100;
      humidity = HDC_1080_get(HUMIDITY);
      ho = humidity %10;
      ht = humidity /10;
      hh = humidity /100;
}
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
    } // if Cal not present
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

  delay = delay/5;

  for(i=0;i<delay;i++);
} // delay_us()

/******************************************************************************/


/******************************************************************************/
// Interrupt Routines
/******************************************************************************/

/******************************************************************************/
// P1 Interrupt Routine
/******************************************************************************/
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  delay_us(50000);
  GetData();

  if (P1IFG & KeyTemp)
  {
    Display = KeyTemp;
  }
  else if (P1IFG & KeyHumi)
  {
    Display = KeyHumi;
  }

  __bic_SR_register_on_exit(LPM4_bits);
  P1IFG &= ~(KeyTemp + KeyHumi);

}