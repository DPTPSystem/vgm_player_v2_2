# MPLAB IDE generated this makefile for use with GNU make.
# Project: vgm.mcp
# Date: Fri Jun 19 19:15:10 2020

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

vgm.cof : main.o
	$(LD) /p18F452 "18f452_g.lkr" "main.o" /u_CRUNTIME /u_DEBUG /o"vgm.cof" /M"vgm.map" /W

main.o : main.c ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdio.h ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdlib.h ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/delays.h header.h main.c ym2612.h main.c function.h interrupt.h main.c ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18f452.h ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stdarg.h ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/stddef.h ../../../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.47/h/p18cxxx.h
	$(CC) -p=18F452 /i"C:\Program Files (x86)\Microchip\mplabc18\v3.47\h" "main.c" -fo="main.o" -D__DEBUG -D__MPLAB_DEBUGGER_PICKIT2=1 -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

clean : 
	$(RM) "main.o" "vgm.cof" "vgm.hex"

