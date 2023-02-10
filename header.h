//PIC18F452 koncigurációs regiszterek
#pragma config OSC      = HSPLL    // HS oscillator with PLL enabled/Clock frequency = (4 x FOSC)
#pragma config OSCS     = OFF      // Osc. kapcsoló

#pragma config BOR      = ON       // Alacsony feszültségnél reset
#pragma config BORV     = 27       // 2.7V-nál már resetel
#pragma config PWRT     = OFF      // Feszültség mérõ timer

#pragma config WDT      = OFF      // Watchdog bekapcsolva
#pragma config WDTPS    = 128      // 1:128 Watchdog idõzítés osztoja

#pragma config CCP2MUX  = ON       // PWM2 legyen az (RC1)

#pragma config STVR     = ON       // Werem túlcsordulás esetén reset
#pragma config LVP      = OFF      // Alacsony feszültségû programozás engedélyezése
#pragma config DEBUG    = ON       // Debug, nyomonkövetés bekapcsolva

#pragma config CP0      = OFF      // Kód védelem
#pragma config CP1      = OFF

#pragma config CPB      = OFF      // Boot block kódvédelem
#pragma config CPD      = OFF      // Eeprom kód védelem

#pragma config WRT0     = OFF      // Írás védelem
#pragma config WRT1     = OFF

#pragma config WRTB     = OFF      // Boot block írásvédelem
#pragma config WRTC     = OFF      // Konfigurációs regiszter írás védelme
#pragma config WRTD     = OFF      // Eeprom írás védelem

#pragma config EBTR0    = OFF      // Tábla olvasási védelem
#pragma config EBTR1    = OFF
#pragma config EBTRB    = OFF      // Boot block tábla olvasási védelem

#define FOSC 40000000L

/*	Késleltetés	*/
void Delay_Ms(unsigned int ms){
   while(ms--) Delay1KTCYx(10);
}

/*	Késleltetés	18uS/ciklus*/
void Delay_Us(unsigned int us){
   while(us--) Delay10TCYx(8);
}

/*********************************************
+		Union deklarálása
**********************************************/
/*- 16bit-es UNION - (SPI memória címzéshez és 2 bájtos (16bit-es) adatok tárolásához kell)*/
typedef union _union16 { 
  unsigned long value;
  struct {
	unsigned char hh;
    unsigned char high;
	unsigned char low;
  };
} union16;
