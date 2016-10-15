These are all the files required to program the MSP430G2553 for the Thermo/HygroMeter using I2C to communicate to HDC1080

The IDE used for this project is the free code size limited version of IAR V6.40.1
To program the release version of the code I use the MSP430Flasher V 1.3.3
If you have a FET connected just double click the .bat file and the code will be programmed

The JTAG connection on the board is as follows.

 PCB TOP
 
   TST RST
   
   VCC GND

MSP430Flasher Version 1.3.7: This version seems to be the last one before it changed to MSPFlasher with a newer msp430.dll
http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430Flasher/1_03_07_00/index_FDS.html

Version 1.3.7 of the Flasher still contains the msp430.dll version 3.5.1.1
