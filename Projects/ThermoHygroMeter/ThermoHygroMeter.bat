@ECHO OFF
COLOR 71
CLS
TITLE MSPFlasher
ECHO.

ECHO Programming ThermoHygroMeter 430 txt
ECHO.

C:\ti\MSP430Flasher_1.3.7\MSP430Flasher.exe -n MSP430G2553 -e ERASE_MAIN -w ".\Release\Exe\ThermoHygroMeter.txt" -g -z [VCC]

ECHO.
SET /P ENTER=Hit Enter to Exit