//PIC18F452 koncigur�ci�s regiszterek
#pragma config OSC      = HSPLL    // HS oscillator with PLL enabled/Clock frequency = (4 x FOSC)
#pragma config OSCS     = OFF      // Osc. kapcsol�

#pragma config BOR      = ON       // Alacsony fesz�lts�gn�l reset
#pragma config BORV     = 27       // 2.7V-n�l m�r resetel
#pragma config PWRT     = OFF      // Fesz�lts�g m�r� timer

#pragma config WDT      = OFF      // Watchdog bekapcsolva
#pragma config WDTPS    = 128      // 1:128 Watchdog id�z�t�s osztoja

#pragma config CCP2MUX  = ON       // PWM2 legyen az (RC1)

#pragma config STVR     = ON       // Werem t�lcsordul�s eset�n reset
#pragma config LVP      = OFF      // Alacsony fesz�lts�g� programoz�s enged�lyez�se
#pragma config DEBUG    = ON       // Debug, nyomonk�vet�s bekapcsolva

#pragma config CP0      = OFF      // K�d v�delem
#pragma config CP1      = OFF

#pragma config CPB      = OFF      // Boot block k�dv�delem
#pragma config CPD      = OFF      // Eeprom k�d v�delem

#pragma config WRT0     = OFF      // �r�s v�delem
#pragma config WRT1     = OFF

#pragma config WRTB     = OFF      // Boot block �r�sv�delem
#pragma config WRTC     = OFF      // Konfigur�ci�s regiszter �r�s v�delme
#pragma config WRTD     = OFF      // Eeprom �r�s v�delem

#pragma config EBTR0    = OFF      // T�bla olvas�si v�delem
#pragma config EBTR1    = OFF
#pragma config EBTRB    = OFF      // Boot block t�bla olvas�si v�delem

#define FOSC 40000000L

/*	K�sleltet�s	*/
void Delay_Ms(unsigned int ms){
   while(ms--) Delay1KTCYx(10);
}

/*	K�sleltet�s	18uS/ciklus*/
void Delay_Us(unsigned int us){
   while(us--) Delay10TCYx(8);
}

/*********************************************
+		Union deklar�l�sa
**********************************************/
/*- 16bit-es UNION - (SPI mem�ria c�mz�shez �s 2 b�jtos (16bit-es) adatok t�rol�s�hoz kell)*/
typedef union _union16 { 
  unsigned long value;
  struct {
	unsigned char hh;
    unsigned char high;
	unsigned char low;
  };
} union16;
