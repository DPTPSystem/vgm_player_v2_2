/**********************************************************
* DPTP System - VGM Player 2.0 YM2612 + SN76489
* E-mail: don_peter@freemail.hu
* PIC18F452
* SEGA GAME GEAR, SEGA MESTER SYSTEM, SEGA MEGA DRIVE
* 44100Hz megzsak�t�sban kiszolg�lva, 22.7uS
**********************************************************/
#include <p18f452.h>
#include "header.h"
//#include "pcm.h" // m1.vgm

/**********************************************************
* Sonic the hedgehog
* 119 kbps nem j�
* 117 kbps nem j�
* 96 kbps nem j�
* 94 kbps nem j�
* 92 kbps nem j�
* 90 kbps nem j�
* 77 kbps nem j�
* 75 kbps nem j�
* 70 kbps nem j�
* 69 kbps nem j� 
* 61 kbps nem j�
* 38 kbps majdnem j�
* 32 kbps majdnem j�
* 9 kbps j�
* 7 kbps j� (nincs PCM)
* 3 kbps j� (nincs PCM)
**********************************************************/

/**********************************************************
* Butals: paws of fury
* 21 kbps - 5 kbps, mind j� (nincs PCM)
**********************************************************/

/********************************************
*	Definici�k								*
********************************************/
// SPI
#define SPI_SO_TRIS		TRISCbits.TRISC5
#define SPI_SCK_TRIS	TRISCbits.TRISC3
#define SPI_SI			TRISCbits.TRISC4

// SN76489AN
#define SN_OSC_E		LATCbits.LATC2
#define SN_WE			LATBbits.LATB0
#define SN_PORT			LATD
#define SN_OE			LATBbits.LATB1

// YM2612
#define YM_OSC_E		LATCbits.LATC2
#define YM_IC			LATAbits.LATA0
#define YM_CS			LATCbits.LATC0
#define YM_WR			LATBbits.LATB4
#define YM_RD			LATBbits.LATB5
#define YM_A0			LATBbits.LATB3
#define YM_A1			LATBbits.LATB2

#define LED2			LATEbits.LATE1
#define LED1			LATEbits.LATE2

// 25D40 - 2Mbit - 256Kbyte
#define MEM_E			LATCbits.LATC1
//-- A 25LC640 EEPROM �ltal elfogadott parancsok
//----------- Parancsok 25D40 mem�ri�hoz -----------------------//
#define CMD_WRSR  		0x01		// St�tuszregiszter �r�sa
#define CMD_WRITE 		0x02		// �r�s a megadott c�mt�l kezd�d�en
#define CMD_READ  		0x03		// Olvas�s a megadott c�mt�l kezd�d�en
#define CMD_FREAD		0x0B		// Gyors olvas�s, WinBond 25Q64, 64byte
#define CMD_WRDI  		0x04		// Letiltja az �r�st
#define CMD_RDSR  		0x05		// St�tuszregiszter olvas�sa
#define CMD_WREN  		0x06		// Enged�lyezi az �r�st
#define CMD_Erase		0x60		// Flash t�rl�se
#define CMD_BLCK64Kb	0xD8		// 1 blokk t�rl�se (~150ms - ~2s), 1 blokk 64Kb
#define PMEMBLOCK		256
#define BufferArraySize 256

#define GOMB			PORTAbits.RA1

volatile union16 MemCim;					//16bit-es struct v�ltoz�k
volatile union16 PCMMemAdr;
volatile unsigned long pcmBufferPositionStart = 0;
volatile unsigned int waitSamples = 0;
volatile unsigned char SampleNext = 1;
volatile float Oszto = 2.04;	// Sebess�g v�ltoztat�shoz ezt kell �ll�tani
#include "ym2612.h"

volatile unsigned char ResultOK=0;
volatile unsigned char Sample = 0;

volatile unsigned long DataSizePC;				// PC-�t�l �rkez� adat m�rete
volatile unsigned char Buffer[BufferArraySize];	// Be�rkezett adat t�rol�
volatile unsigned char Buffer2[BufferArraySize];// Be�rkezett adat t�rol�
volatile unsigned char bUffEr = 1;				// Alap�rtelmezett buffer be�ll�tva (Buffer[BufferArraySize])
volatile unsigned int  BufferI=0;				// Buffer index
volatile unsigned int  BufferSize=0;			// Buffer m�rete
volatile unsigned int  Error=0;					// Hiba sz�mol�sa
volatile unsigned long DataSize=0;				// Be�rkez� adat teljes m�rete

// PCM t�bl�zat haszn�lat�hoz
volatile unsigned int PCMJumpIndex = 0;
volatile unsigned int JumpTableE0[100];			// PCM ugr� t�bla

#define PMBlock			64						// 1 bloc 64Kbyte
#define PMWbyte			8						// 1 �r�s 8 byte
#define PMemAddrStart	0x3000					// Program mem�ria �res hely�nek kezdete
#define MaxPCMData		0x0000					// 16Kbyte (0x4000)
volatile unsigned int BuffIndex = 0;
volatile unsigned int PCMDataSize = 0;
volatile unsigned char PCMLoad = 0;

volatile unsigned long long Timer0Count = 0; 			// 1 egys�g 200nS

#include "function.h"
#include "interrupt.h"

void main(void)
{
	unsigned char Upload;
	unsigned int i,x=0;
	unsigned int temp = 0;
	unsigned int Maradek=0, Pack=0, PackCount=0;
	unsigned char EraseBlock = 0;
	
		// Csak akkor kell, ha f�programban akarunk lej�tszani
		/*unsigned float wait = 0;
		unsigned char Sample = 0;
		unsigned char YM_address = 0;
		unsigned char YM_data = 0;
		unsigned char osztas = 0, maradek = 0;

		unsigned char tmp;*/
	
	/*unsigned int wait = 0;
	unsigned char Sample = 0;
	unsigned char YM_address = 0;
	unsigned char YM_data = 0;
	unsigned char osztas = 0, maradek = 0;
	
	const float sampleRate = 44100;
	const float WAIT60TH = 1000 / (sampleRate/735);
	const float WAIT50TH = 1000 / (sampleRate/882);
	float cachedWaitTime = 0;
	const unsigned char uS = 22;	// 1uS == uS, kim�rve szk�ppal
	unsigned int preCalced8nDelays[16];
	unsigned int preCalced7nDelays[16];
	*/
	
	memset(Buffer, '\0', sizeof(Buffer));	//T�r�lj�k a buffer tartalm�t
	memset(Buffer2, '\0', sizeof(Buffer2));	//T�r�lj�k a buffer tartalm�t

	MCUInit();
	// LTC6903 init SN76489N -  // 3.58MHz - 0xBD24
	// YM2612 - // 7.67 MHz - 0xCE40
	OSC_Init();
	init_usart();

	// VGM Play
	PSG_Silence();
	
	// Eprom olvas�sa
	PCMLoad = EepromRead();
	if(PCMLoad==0xFF) PCMLoad = 0;

	MemCim.value = 63;
	PCMMemAdr.value = 0;

	LATA &= 0x0F;
	LATD = 0;
	
	// Miel�tt elind�tjuk a megszak�t�st, ments�k el 0xE0 parcskor �rkezett PCM adat ugr�sokat egy t�bl�ba
	//memset(JumpTableE0, '\0', sizeof(JumpTableE0));
	//PCMJumpSave();
	
	T2CONbits.TMR2ON = 1;
	
	/*for(i = 0; i<16; i++)
    {
      if(i == 0)
      {
        preCalced8nDelays[i] = 0;
        preCalced7nDelays[i] = 1;
      }
      else
      {
        preCalced8nDelays[i] = ((1000.0 / (sampleRate/(float)i))*1000)/uS;
        preCalced7nDelays[i] = ((1000.0 / (sampleRate/(float)i+1))*1000)/uS;
      }
    }*/
    
    /*******************************************************************
    * Tesztel�shez k�sz�tettem egy timer0-�s rutint, hogy megtudjam
    * mennyi ideig tart egy mintav�telez�s.
    * M�rt adat: 
    *
    ********************************************************************/
    /*HighIntTimer0();
    Timer0Count = 0;
    MemCim.value = 0x259;
    T0CONbits.TMR0ON = 1;
    
    for(i=0; i<1000; i++)
    Sample = MemRead(MemCim);	// 68,736uS alatt fut le (0.64 megzsak�t�s)
    
    //WaveSample();	// (0.98 megzsak�t�s) 105,25uS alatt fut le alap esetben, ha nem kell feldolgozni
    				// 0x52-es k�ddal, vagy van kimeneti vez�rl�s legal�bb 1 esetben
    				// (1.01 megzsak�t�s) 108,47uS
    				
    // 0x8F parancs (1.14 megzsak�t�s) 122,436uS
    
    // 0xE0 parancs (4.6 megzsak�t�s) 494,04uS
   
    T0CONbits.TMR0ON = 0;
    Upload = 0;
    */
    
    // Osszcill�szk�pos m�r�s
    //for(;;)
    //{
    	//Sample = MemRead(MemCim);			// 64.4uS/2 = 32.2uS
    	
    	// Saj�t optimaliz�lt v�ltozat
    	// Sample = MemReadFast(MemCim);	// 34.2uS/2 = 17.1uS
    	
    	// Minden v�rakoztat�s �s sallang n�lk�l
    	// Sample = MemReadFast(MemCim);	// 22.4uS/2 = 11.2uS
    	// WaveSample();					// 62uS/2 = 31uS
    	
    	// Alap parancs n�lk�l
    	// 1. Optimaliz�lt olvas�ssal 69uS/2 = 34.5uS alatt fut le
    	//WaveSample();					// 99.2uS/2 = 49.6uS
    	
    	    	
    	//Sample = SPI(0x00);			// 8.2uS/2 = 4.1uS
    	    	
    	/*/ SPI(0x00) fuggv�ny n�lk�l	// 4.2uS/2 = 2.1uS
    	SSPSTATbits.BF = 0;
		SSPCON1bits.WCOL = 0;
		Sample = SSPBUF;
		SSPBUF = 0x00;
		while(!SSPSTATbits.BF);
		Sample = SSPBUF;
		*/
    	
    	
    	//WaitForMem();				// 20.2uS/2 = 10.1uS - �tszervezve 10.6uS/2 = 5.3uS
    	
    	// WaitForMem() f�ggv�ny ugr�s n�lk�l 8.4uS/2 = 4.2uS
    	/*do {
			MEM_E = 0;                      	 //kiadjuk a Chip Enable jelet
									
			SSPSTATbits.BF = 0;			// t�rli foglalts�g jelz� �lapot�t a st�tusz regiszterben
			SSPCON1bits.WCOL = 0;		// t�rli az esetleges �r�s �tk�z�s hibajelz�t
			tmp = SSPBUF;				// t�rli a BF jelz�bitet
			SSPBUF = CMD_RDSR;				// kirakja a kimen� adatot az SSPBUF regiszterbe
			while(!SSPSTATbits.BF);		// megv�rjuk a busz ciklus v�g�t 
						
			SSPSTATbits.BF = 0;			// t�rli foglalts�g jelz� �lapot�t a st�tusz regiszterben
			SSPCON1bits.WCOL = 0;		// t�rli az esetleges �r�s �tk�z�s hibajelz�t
			tmp = SSPBUF;				// t�rli a BF jelz�bitet
			SSPBUF = 0x00;				// kirakja a kimen� adatot az SSPBUF regiszterbe
			while(!SSPSTATbits.BF);		// megv�rjuk a busz ciklus v�g�t 
			
			MEM_E = 1;                   		 //megsz�ntetj�k a Chip Enable jelet
		} while (SSPBUF & 0x01); 
		*/
		
		//LED1 = !LED1;
	//}
    /*******************************************************************
    �jabb optimaliz�l�s - 2023-02-08.
    SPI olvas�s �tszervez�se, amely az alap olvas�i adatokat a k�vetkez� 
    k�ppen m�dos�totta:
   	Sample = MemRead(MemCim);			// 64.4uS/2 = 32.2uS
   	Sample = MemReadFast(MemCim);		// 34.2uS/2 = 17.1uS
   	WaveSample();						// 99.2uS/2 =  49.6uS - optimaliz�ltan 69uS/2 = 34.5uS
   	Az olvas�si ciklusokat �sszevontam �gy nincs f�ggv�nyugr�s ezzel fel�re
   	cs�kkentve a sz�ks�ges olvas�si id�t
   	K�vetkez� lehet�s�g m�g a VGM f�jl ki�rt�kel�s�nek �tszervez�se, 
   	f�k�nt az olvas�sokn�l. Olvas�sok id�tartama m�g minden ciklus eset�ben
   	tov�bbi 4.8uS id�vel cs�kkenthet�. 
   	0x61-es parancs eset�ben ez 9.6uS/2 = 4.8uS
   	0xE0-�s parancs eset�ben 19.2uS/2 = 9.6uS
   	
   	Update - 2023-02-09.
   	SPI olvas�st a minim�lisra reduk�ltam, nincs mem�ri�ra v�rakoz�s �s
   	semmi, ami tov�bb lass�tan� a forgalmat. Sebess�g �gy maximaliz�lt.
   	Sample = MemReadFast(MemCim);		// 22.4uS/2 = 11.2uS
   	WaveSample();						// 62uS/2 = 31uS
   	Megszak�t�sba a nyers adatokkal is kipr�b�ltam, de a PCM adatok eset�ben
   	m�g mindig nem el�gs�ges a sebess�g
   	Megszak�t�sba pakolva mindent egy semleges 0x00-�s c�mmel, 140uS/2 = 70uS a lefut�s
   	ami durva, mert 44.5/2 = 22.7uS-n�l nem lehetne t�bb. (ennyi a megszak�t�si ablak)
   	Ha a megszak�t�sban waitSamples = WaveSample(); fuggv�ny van �s az a 
   	MemReadFast(MemCim)-es WaveSample()-t h�vja meg, akkor a lefut�s 158.9uS/2 = 79.45uS
   	Ket� k�zt 18.9uS/2 = 9.45uS id� r�s van, de enn�l nem tudom, hogy lehetne m�g t�bbet 
   	kighozni a PIC-b�l, max assembly programmal.
   	�mlesztett k�ddal 147.8uS/2 = 73.9uS
   	
   	F�program 2.4uS/2 = 1.2uS egy ciklus ha UART felt�telek benne vannak, ha csak a
   	LED billegtet�se, akkor 600nS/2 = 300nS
   	F�programba �tpakolva a teljes k�dot, gyorsabb, de az id�z�t�s ebben az
   	esetbe nem teljesen megoldhat�, mert 2 hangminta k�zt eltelt id� att�l 
   	sz�m�t, hogy mennyi id� alatt �rt�kel�dik ki a teljes VGM strukt�ra
   	�s ebb�l le kellene vonni az alap id�z�t�st.
   	****************************************************************************
   	* Nagyon fontos: 2023-02-09. Este
   	* Nem vettem �szre (nem �rtem mi�rt nem t�nt fel kor�bban) a m�r�sek k�zben, 
   	* hogy amit m�rek az k�t jel vagy is a LED egyszer vil�g�t, egyszer nem. 
   	* A peri�dikus jelem, amit m�rek azt minden esetben 2 lefut�s eredm�nye. 
   	* Teh�t minden m�r�sem osztani kell kett�vel �s ez lesz az egyszeri lefut�s ideje.
   	****************************************************************************
   	Megszak�t�sban az if felt�teles verzi� 96.59uS/2 = 48.3uS 0x00-�s paranccsal.
   	
   	Nos t�bbedj�re is oda jutok, hogy a program b�rhogy optimaliz�lom, nem k�pes
   	22.7uS alatt vagy is egy megszak�t�si ablak alatt v�gbe menni. Emiatt lass�
   	a zene lej�tsz�sa. A program jelenleg az �sszes optimaliz�ssal egyben is
   	~ 51uS ideig dolgozik 1 hangmint�n, pedig csak 22.7uS id� �ll rendelkez�s�re. 
   	
   	2023.02.10. Utols� agyal�som eredm�nye �s ezzel lehet is z�rni a projektet.
   	Teljes �tszervez�s kapcs�n arar jutottam, hogy a m�r�sek alapj�n, ha csak 
   	az adatok kik�ld�s�st tenn�m a 22.7uS-os megszak�t�sba, akkor sem lenne j�
   	az eredm�ny, mert f�programomban tesztelt VGM ki�rt�kel�si strukt�ra lefut�sa
   	meghaladja a megszak�t�si id�ablakot. Minden er�fesz�t�sem ellen�re sem tudom
   	31uS al� vinni a VGM feldolgoz�s idej�t. Ezzel egy�rtelm�en bebizonyosodott,
   	hogy a PIC18F452 nem k�pes a VGM feldolgoz�sra, ha az adatokban PCM adat is
   	van. �gy a progjektet ezzel le is z�rom. Tov�bbiakban m�s MCU-ra fejlesztek
   	tov�bb.
   	- PCM adatok PIC flash-be ment�s�t kikapcsoltam, mert azt megszak�t�sban nem
   	tudja kiszolg�lni. PCM n�lk�li VGM-ek eset�n haszn�lhat� a program, tal�n
   	kisebb PCM adatok eset�n is elfogadhat�, de semmi k�p nem t�k�letes.
    */
    
    /*******************************************************************
    K�s�bbre, ha elfelejten�m:
    A timer2 �s UART megszak�t�s nem megy egyben mert a timer2 megszak�t�s 
    olyan gyors, hogy a f�program nem tud lefutni egyszer sem, vagy az MCU
    vagy a szervez�s hib�s, de �gy ebben a form�ban csak, akkor m�k�dik, ha
    GOMB-al ind�tjuk a f�programot, teh�t a felt�lt�s (UART) megszak�t�s
    mindaddig el�rhet�, ameddig nincs elind�tva a timer2 megszak�t�s vagy is
    a lej�tsz�s. Majd lehet egyszer �jra kell szervezni, b�r STM32F103RF
    j� v�laszt�snak t�nt egy m�sodlagos jobb verzi� elk�sz�t�s�shez.
    2023-01-18. n�ztem �s szerveztem �jra a programot, hogy m�k�dj�n.
    PCM adatok lej�tsz�sr�ra a PIC sebess�ge (40MHz) nem elegend�.
    
    - kieg�sz�tve ezt annyiban, hogy az �jabb m�r�sek alapj�n a megszak�t�s
    nem k�pes 22.6uS alatt v�gbe menni mert a benne l�v� utas�t�sok PCM adatot
    nem tartalmaz� VGM eset�ben is ~94-150uS id� alatt megy v�gbe, majd ezt
    k�vet�en szabadulhat csak fel a megszak�t�s, de mivel kifutott az id�b�l
    egyb�l a feldolgoz�s v�g�n �jabb megszak�t�sba fordul bele. �gy timer2
    megszak�t�s miatt a f�program nem tud egyszer sem lefutni
    
    - A program 256byte-os adatcsomagokkal dolgozik, teh�t a C SHARP
    programban is �tt kell �rni vagy m�dos�tani + 1 param�ter be�ll�t�s�val.
    
    2023-01-20. Norm�l PCM adatot nem tartalmaz� VGM zen�kn�l �tlagosan 
    ~94uS id�be ker�l egy-egy parancs ki�rt�kel�se, de vannak esetek,
    mikor t�bb parancs is j�n egym�st k�vet�en, akkor ez az id� ~120-150uS
    id�t is ig�nyelhet, �gy kiz�rt, hogy az el�re be�ll�tott 22.6uS-os
    megszak�t�s id�ben v�gezni tudjon. Ebben az esetben m�g �lvezhet� a lej�tsz�s.
    - PCM adat eset�n f�gg�en mekkora ez az adat ~330uS-ra n� a visszat�r�si
    id�, amely m�r hallhat�an hib�s lej�tsz�st eredm�nyez. (ny�jtja a hangokat
    , lelass�l a lej�tsz�s)
    Teh�t ez a mikrokontroller a maga
    40MHz-es �rajel�vel �s az utasit�sonk�nt 4 illetve ugr�sokn�l 8 �rajelet
    is ig�nyelhetnek, amelyek �nmagukban is sok id�t em�sztenek fel.
    1 �rajel 25nS
    1 utas�t�s 4 �rajel vagy is ~100nS ideig tart
    1 ugr�s 8 �rajel vagy is ~200nS
    ********************************************************************/
	while (1) 	// 2.4uS/2 = 1.2uS
	{

		if(!GOMB)
		{
			while(!GOMB);
			//Reset();
			T2CONbits.TMR2ON = 1;
		}
		
		if(Buffer[0] == 'd') {
			Buffer[0] == '\0';
			T2CONbits.TMR2ON = 0;
			PSG_Silence();

			DataSize = 0;
			BufferSize = 0;
			BufferI=0;
			
			write_uart(0x10);	// Kik�ldj�k az OK visszajelz�st az upload k�dot
			while(DataSize<3);	// Amig nem j�n meg a 3 adat
			// PC-s adat hossza
			DataSizePC = Buffer[2];
			DataSizePC = DataSizePC<<8 | Buffer[1];
			DataSizePC = DataSizePC<<8 | Buffer[0];
			Maradek = DataSizePC%BufferArraySize;	// Hossz v�ge, ha nem pontosan egyel� BufferArraySize
			Pack = DataSizePC/BufferArraySize;		// Adat csomagok sz�ma

			memset(Buffer, '\0', sizeof(Buffer));	//T�r�lj�k a buffer tartalm�t
			memset(Buffer2, '\0', sizeof(Buffer2));	//T�r�lj�k a buffer tartalm�t
			Upload=1;
			bUffEr=1;
			DataSize = 0;
			BufferSize = 0;
			BufferI=0;
			PackCount=0;
			MemCim.value = 0;
			
			//MemErase();
			EraseBlock = (DataSizePC/0xFFFF)>0 ? ((DataSizePC/0xFFFF)+1) : 0;
			for(i=0; i<=EraseBlock; ++i)
			{
				MemBlockErase64Kb(MemCim);
				MemCim.value += 0xFFFF;
			}
			MemCim.value = 0;
			
			write_uart(0xF0);

			while(Upload){
					
			 	if(Error && BufferSize==BufferArraySize){		// Ha hiba van �jra bek�rj�k az adatcsomagot
					Error = 0;									// Error-t null�zuk
					DataSize -= BufferArraySize;				// 64-et kivonunk a m�retb�l
					x++;										// Sz�moljuk h�ny hiba volt
					PackCount--;
					TXREG = 0x01;								// kik�ldj�k a hiba, ism�tl� k�dot
					while(!PIR1bits.TXIF);
				}else if(BufferSize==BufferArraySize || (PackCount == Pack && Maradek == BufferSize)){		// Ha minden rendben van
					
					if((PackCount == Pack && Maradek == BufferSize)){ temp = Maradek; } else { temp = BufferArraySize; }
					// Be�rjuk az �j adatokat a mem�ri�ba

					////// K�t buffer t�lt�s�nek �r�ny�t�sa, szervez�se /////
					bUffEr = !bUffEr;
					BufferSize=0;
					BufferI=0;
					PackCount++;
					if(PackCount<=Pack){
						TXREG = 0x02;				// kik�ldjuk a tov�bb k�dot
						while(!PIR1bits.TXIF);
					}
					//////////////////////////////////////////////////////////
		
					if(!bUffEr){
						FastMemWrite(MemCim, Buffer);
					}else{
						FastMemWrite(MemCim, Buffer2);
					}
					
					// Adat ki�r�sa AddresCounter, 
					MemCim.value += temp;
				
					if(DataSizePC == DataSize && PackCount >= Pack){ 
						Upload = 0; 
						bUffEr = 1; 				// Alap�rtelmezett buffer haszn�lata
						BufferSize=0;
						BufferI=0;
						TXREG = 0x02;				// kik�ldjuk a tov�bb k�dot
						while(!PIR1bits.TXIF);
						
						MemCim.value = 0;
						memset(Buffer, '\0', sizeof(Buffer));	//T�r�lj�k a buffer tartalm�t
						memset(Buffer2, '\0', sizeof(Buffer2));	//T�r�lj�k a buffer tartalm�t
					}// Kil�p�nk �r�sm�db�l

				}
					
			}//while
			MemCim.value = 63;
			PCMMemAdr.value = 0;
			//Reset();
			T2CONbits.TMR2ON = 1;
			EepromWrite(0x00);
			PCMLoad = 0;
		}
		LED1 = !LED1;
						
		
		//if(LED1==1) LED1 = 0; else LED1 = 1; 	// 1.3uS ciklussal egy�tt
		
		/*_asm	
			 Ide: btg LATE,1, ACCESS
             bra  Ide
		_endasm*/
   
   		/*if(waitSamples>0)
			waitSamples--;
		
		if(waitSamples==0 && SampleNext == 1)
		{	
			SampleNext=0;
			MemCim.value++;
			//waitSamples = WaveSample();
			
			///////////////////////////////////////////////////////
			MEM_E = 0;
			
			//SPI(CMD_READ);
			tmp = SSPBUF;
			SSPBUF = CMD_READ;
			while(!SSPSTATbits.BF);
				
			//SPI(MemCim.low);
			tmp = SSPBUF;
			SSPBUF = MemCim.low;
			while(!SSPSTATbits.BF);
			
			//SPI(MemCim.high);
			tmp = SSPBUF;
			SSPBUF = MemCim.high;
			while(!SSPSTATbits.BF);
			
			//SPI(MemCim.hh);
			tmp = SSPBUF;
			SSPBUF = MemCim.hh;
			while(!SSPSTATbits.BF);
			
			//temp = SPI(0x00);
			temp = SSPBUF;
			SSPBUF = 0x00;
			while(!SSPSTATbits.BF);
			
			MEM_E = 1;
			
			Sample = (SSPBUF);
			switch(Sample) //VGM commands
			{
				case 0x4F:
					MemCim.value++;
					SN76489_SendByte(0x06);
					SN76489_SendByte(MemReadFast(MemCim));
					waitSamples =  0;
					break;
				case 0x50:
					MemCim.value++;
					SN76489_SendByte(MemReadFast(MemCim));
					waitSamples =  0;
					break;
				case 0x52:
				case 0x53:
					MemCim.value++;
					//YM_address = MemReadFast(MemCim);
					
					
					MEM_E = 0;
					
					//SPI(CMD_READ);
					tmp = SSPBUF;
					SSPBUF = CMD_READ;
					while(!SSPSTATbits.BF);
						
					//SPI(MemCim.low);
					tmp = SSPBUF;
					SSPBUF = MemCim.low;
					while(!SSPSTATbits.BF);
					
					//SPI(MemCim.high);
					tmp = SSPBUF;
					SSPBUF = MemCim.high;
					while(!SSPSTATbits.BF);
					
					//SPI(MemCim.hh);
					tmp = SSPBUF;
					SSPBUF = MemCim.hh;
					while(!SSPSTATbits.BF);
					
					//temp = SPI(0x00);
					temp = SSPBUF;
					SSPBUF = 0x00;
					while(!SSPSTATbits.BF);
					
					MEM_E = 1;
					
					YM_address = (SSPBUF);
						
					MemCim.value++;
					//YM_data = MemReadFast(MemCim);
					
				
					
					MEM_E = 0;
					
					//SPI(CMD_READ);
					tmp = SSPBUF;
					SSPBUF = CMD_READ;
					while(!SSPSTATbits.BF);
						
					//SPI(MemCim.low);
					tmp = SSPBUF;
					SSPBUF = MemCim.low;
					while(!SSPSTATbits.BF);
					
					//SPI(MemCim.high);
					tmp = SSPBUF;
					SSPBUF = MemCim.high;
					while(!SSPSTATbits.BF);
					
					//SPI(MemCim.hh);
					tmp = SSPBUF;
					SSPBUF = MemCim.hh;
					while(!SSPSTATbits.BF);
					
					//temp = SPI(0x00);
					temp = SSPBUF;
					SSPBUF = 0x00;
					while(!SSPSTATbits.BF);
					
					MEM_E = 1;
					
					YM_data = (SSPBUF);
						
					YM2612_Send(YM_address, YM_data, Sample);
					waitSamples =  0;
					break;
				case 0x61:
					// Mintav�telez�s v�rakoztat�sa
					wait = 0;
					for ( i = 0; i < 2; i++ )
					{
						MemCim.value++;
						//wait += ( (unsigned int)(MemReadFast(MemCim)) << ( 8 * i ));
						
						MEM_E = 0;
						
						//SPI(CMD_READ);
						tmp = SSPBUF;
						SSPBUF = CMD_READ;
						while(!SSPSTATbits.BF);
							
						//SPI(MemCim.low);
						tmp = SSPBUF;
						SSPBUF = MemCim.low;
						while(!SSPSTATbits.BF);
						
						//SPI(MemCim.high);
						tmp = SSPBUF;
						SSPBUF = MemCim.high;
						while(!SSPSTATbits.BF);
						
						//SPI(MemCim.hh);
						tmp = SSPBUF;
						SSPBUF = MemCim.hh;
						while(!SSPSTATbits.BF);
						
						//temp = SPI(0x00);
						temp = SSPBUF;
						SSPBUF = 0x00;
						while(!SSPSTATbits.BF);
						
						MEM_E = 1;
						
						wait += ( (unsigned int)((SSPBUF)) << ( 8 * i ));
					}
					waitSamples =  wait/Oszto;
					break;
				case 0x62:
					waitSamples =  735.0/Oszto;
					break;
				case 0x63:
					waitSamples =  882.0/Oszto;
					break;
				case 0x67:
					// PCM adat kezel�se
					MemCim.value++;	// Skip 0x66
					MemCim.value++; // Skip data type
					for ( i = 0; i < 4; i++ )
					{
						MemCim.value++;
						PCMDataSize += ( (unsigned long)(MemReadFast(MemCim)) << ( 8 * i ));
					}
					// Ha van PCM adat �s ha belef�r�nk a programem�ri�ba
					if(PCMDataSize > 0 && PCMDataSize <= MaxPCMData && !PCMLoad)
					{
						// Program mem�ra t�rl�se \\
						//osztas = PCMDataSize/PMBlock;	// 64Kbyte-al t�rt�n� oszt�s eg�sz r�sze
						//maradek = PCMDataSize%PMBlock;	// Oszt�s ut�ni marad�k, ha van
						osztas = Osztas(PCMDataSize, PMBlock, 0);
						maradek = Osztas(PCMDataSize, PMBlock, 1);
						if(maradek>0) 
							osztas++;					// Ha van marad�k akkor oszt�st emeljuk 1-el
						for(i=0; i<osztas; i++)			// T�r�lj�k a sz�ks�ges ter�letet
							PICFlashBlockErase(PMemAddrStart+(i*PMBlock));
						// Program mem�ria t�rl�se V�GE //
						
						// Bet�ltj�k az adatokat majd be�rjuk programmem�ri�ba \\
						//osztas = PCMDataSize/BufferArraySize;	// 256Kbyte-al t�rt�n� oszt�s eg�sz r�sze
						//maradek = PCMDataSize%BufferArraySize;	// Oszt�s ut�ni marad�k, ha van
						osztas = Osztas(PCMDataSize, BufferArraySize, 0);
						maradek = Osztas(PCMDataSize, BufferArraySize, 1);
						if(maradek>0) 
							osztas++;
						// Ciklus ameddig adat v�g�re nem �r�nk
						for(x=0; x<osztas; x++){
							// Buffer felt�lt�se
							for(i=0; i<BufferArraySize; i++)
							{
								MemCim.value++;
								Buffer[i] = MemReadFast(MemCim);
							}
							// Buffer ki�r�sa PM-ba
							PICFlashBlockWrite(PMemAddrStart+(x*BufferArraySize), Buffer);
							memset(Buffer, '\0', sizeof(Buffer));	//T�r�lj�k a buffer tartalm�t
						}
						// Be�r�s V�GE
						
						BuffIndex=0;
						
						// EEPROM-ban jelezz�k, hogy PCM m�r volt bet�ltve
						EepromWrite(0x01);
						PCMLoad = 1;
					}
					else	// Ha nem f�r�nk bele a programem�ri�ba
					{
						// PCM kezd� adat c�me
						if(!PCMLoad)
						{
							pcmBufferPositionStart = MemCim.value;
							PCMMemAdr.value = pcmBufferPositionStart;
						}
						MemCim.value+=PCMDataSize;
					}
					waitSamples =  0;
					break;
				case 0x70:
				case 0x71:
				case 0x72:
				case 0x73:
				case 0x74:
				case 0x75:
				case 0x76:
				case 0x77:
				case 0x78:
				case 0x79:
				case 0x7A:
				case 0x7B:
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F:
					wait = (Sample & 0x0F)+1;
					waitSamples =  wait/Oszto;
					break;
				case 0x80:
				case 0x81:
				case 0x82:
				case 0x83:
				case 0x84:
				case 0x85:
				case 0x86:
				case 0x87:
				case 0x88:
				case 0x89:
				case 0x8A:
				case 0x8B:
				case 0x8C:
				case 0x8D:
				case 0x8E:
				case 0x8F:
					wait = Sample & 0x0F;
					YM_address = 0x2A;
					if(PCMLoad)
					{
						BuffIndex++;
						//YM_data = PICFlashReadByte(PMemAddrStart+BuffIndex);
						TBLPTR = PMemAddrStart+BuffIndex;
						_asm TBLWTPOSTINC _endasm;
		    			_asm TBLRDPOSTDEC _endasm;
						YM_data = TABLAT;
					}
					else
					{
						PCMMemAdr.value++;
						//YM_data = MemReadFast(PCMMemAdr);
						
						MEM_E = 0;
						
						//SPI(CMD_READ);
						tmp = SSPBUF;
						SSPBUF = CMD_READ;
						while(!SSPSTATbits.BF);
							
						//SPI(MemCim.low);
						tmp = SSPBUF;
						SSPBUF = PCMMemAdr.low;
						while(!SSPSTATbits.BF);
						
						//SPI(MemCim.high);
						tmp = SSPBUF;
						SSPBUF = PCMMemAdr.high;
						while(!SSPSTATbits.BF);
						
						//SPI(MemCim.hh);
						tmp = SSPBUF;
						SSPBUF = PCMMemAdr.hh;
						while(!SSPSTATbits.BF);
						
						//temp = SPI(0x00);
						temp = SSPBUF;
						SSPBUF = 0x00;
						while(!SSPSTATbits.BF);
						
						MEM_E = 1;
						
						YM_data = (SSPBUF);
						
					}	
					YM2612_Send(YM_address, YM_data, Sample);
					waitSamples =  wait/Oszto;
					break;
				case 0xE0:
					if(PCMLoad)
					{
						BuffIndex = 0;
						for (i = 0; i < 4; i++ )
						{
							MemCim.value++;
							//BuffIndex += ( (unsigned int)(MemReadFast(MemCim)) << ( 8 * i ));
							
							MEM_E = 0;
							
							//SPI(CMD_READ);
							tmp = SSPBUF;
							SSPBUF = CMD_READ;
							while(!SSPSTATbits.BF);
								
							//SPI(MemCim.low);
							tmp = SSPBUF;
							SSPBUF = MemCim.low;
							while(!SSPSTATbits.BF);
							
							//SPI(MemCim.high);
							tmp = SSPBUF;
							SSPBUF = MemCim.high;
							while(!SSPSTATbits.BF);
							
							//SPI(MemCim.hh);
							tmp = SSPBUF;
							SSPBUF = MemCim.hh;
							while(!SSPSTATbits.BF);
							
							//temp = SPI(0x00);
							temp = SSPBUF;
							SSPBUF = 0x00;
							while(!SSPSTATbits.BF);
							
							MEM_E = 1;
							
							BuffIndex += ( (unsigned int)((SSPBUF)) << ( 8 * i ));
						}
					}
					else
					{	
						PCMMemAdr.value = pcmBufferPositionStart;
						for (i = 0; i < 4; i++ )
						{
							MemCim.value++;
							//PCMMemAdr.value += ( (unsigned int)(MemReadFast(MemCim)) << ( 8 * i ));
														
							MEM_E = 0;
							
							//SPI(CMD_READ);
							tmp = SSPBUF;
							SSPBUF = CMD_READ;
							while(!SSPSTATbits.BF);
								
							//SPI(MemCim.low);
							tmp = SSPBUF;
							SSPBUF = MemCim.low;
							while(!SSPSTATbits.BF);
							
							//SPI(MemCim.high);
							tmp = SSPBUF;
							SSPBUF = MemCim.high;
							while(!SSPSTATbits.BF);
							
							//SPI(MemCim.hh);
							tmp = SSPBUF;
							SSPBUF = MemCim.hh;
							while(!SSPSTATbits.BF);
							
							//temp = SPI(0x00);
							temp = SSPBUF;
							SSPBUF = 0x00;
							while(!SSPSTATbits.BF);
							
							MEM_E = 1;
							
							PCMMemAdr.value += ( (unsigned int)((SSPBUF)) << ( 8 * i ));
						}
					}
					waitSamples =  0;
					break;
				case 0x66:
					MemCim.value=63;
					waitSamples =  0;
					break;
				default:
					waitSamples =  0;
					break;
			}
			
			///////////////////////////////////////////////////////
			
			SampleNext=1;
		}
		Delay_Us(2);
		LED2 = !LED2;*/
		//Delay_Ms(100);
		
	}
      
 }    

