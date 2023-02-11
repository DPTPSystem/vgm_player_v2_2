/**********************************************************
* DPTP System - VGM Player 2.0 YM2612 + SN76489
* E-mail: don_peter@freemail.hu
* PIC18F452
* SEGA GAME GEAR, SEGA MESTER SYSTEM, SEGA MEGA DRIVE
* 44100Hz megzsakításban kiszolgálva, 22.7uS
**********************************************************/
#include <p18f452.h>
#include "header.h"
//#include "pcm.h" // m1.vgm

/**********************************************************
* Sonic the hedgehog
* 119 kbps nem jó
* 117 kbps nem jó
* 96 kbps nem jó
* 94 kbps nem jó
* 92 kbps nem jó
* 90 kbps nem jó
* 77 kbps nem jó
* 75 kbps nem jó
* 70 kbps nem jó
* 69 kbps nem jó 
* 61 kbps nem jó
* 38 kbps majdnem jó
* 32 kbps majdnem jó
* 9 kbps jó
* 7 kbps jó (nincs PCM)
* 3 kbps jó (nincs PCM)
**********************************************************/

/**********************************************************
* Butals: paws of fury
* 21 kbps - 5 kbps, mind jó (nincs PCM)
**********************************************************/

/********************************************
*	Definiciók								*
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
//-- A 25LC640 EEPROM által elfogadott parancsok
//----------- Parancsok 25D40 memóriához -----------------------//
#define CMD_WRSR  		0x01		// Státuszregiszter írása
#define CMD_WRITE 		0x02		// Írás a megadott címtõl kezdõdõen
#define CMD_READ  		0x03		// Olvasás a megadott címtõl kezdõdõen
#define CMD_FREAD		0x0B		// Gyors olvasás, WinBond 25Q64, 64byte
#define CMD_WRDI  		0x04		// Letiltja az írást
#define CMD_RDSR  		0x05		// Státuszregiszter olvasása
#define CMD_WREN  		0x06		// Engedélyezi az írást
#define CMD_Erase		0x60		// Flash törlése
#define CMD_BLCK64Kb	0xD8		// 1 blokk törlése (~150ms - ~2s), 1 blokk 64Kb
#define PMEMBLOCK		256
#define BufferArraySize 256

#define GOMB			PORTAbits.RA1

volatile union16 MemCim;					//16bit-es struct változók
volatile union16 PCMMemAdr;
volatile unsigned long pcmBufferPositionStart = 0;
volatile unsigned int waitSamples = 0;
volatile unsigned char SampleNext = 1;
volatile float Oszto = 2.04;	// Sebesség változtatáshoz ezt kell állítani
#include "ym2612.h"

volatile unsigned char ResultOK=0;
volatile unsigned char Sample = 0;

volatile unsigned long DataSizePC;				// PC-étõl érkezõ adat mérete
volatile unsigned char Buffer[BufferArraySize];	// Beérkezett adat tároló
volatile unsigned char Buffer2[BufferArraySize];// Beérkezett adat tároló
volatile unsigned char bUffEr = 1;				// Alapértelmezett buffer beállítva (Buffer[BufferArraySize])
volatile unsigned int  BufferI=0;				// Buffer index
volatile unsigned int  BufferSize=0;			// Buffer mérete
volatile unsigned int  Error=0;					// Hiba számolása
volatile unsigned long DataSize=0;				// Beérkezõ adat teljes mérete

// PCM táblázat használatához
volatile unsigned int PCMJumpIndex = 0;
volatile unsigned int JumpTableE0[100];			// PCM ugró tábla

#define PMBlock			64						// 1 bloc 64Kbyte
#define PMWbyte			8						// 1 írás 8 byte
#define PMemAddrStart	0x3000					// Program memória üres helyének kezdete
#define MaxPCMData		0x0000					// 16Kbyte (0x4000)
volatile unsigned int BuffIndex = 0;
volatile unsigned int PCMDataSize = 0;
volatile unsigned char PCMLoad = 0;

volatile unsigned long long Timer0Count = 0; 			// 1 egység 200nS

#include "function.h"
#include "interrupt.h"

void main(void)
{
	unsigned char Upload;
	unsigned int i,x=0;
	unsigned int temp = 0;
	unsigned int Maradek=0, Pack=0, PackCount=0;
	unsigned char EraseBlock = 0;
	
		// Csak akkor kell, ha fõprogramban akarunk lejátszani
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
	const unsigned char uS = 22;	// 1uS == uS, kimérve szkóppal
	unsigned int preCalced8nDelays[16];
	unsigned int preCalced7nDelays[16];
	*/
	
	memset(Buffer, '\0', sizeof(Buffer));	//Törõljük a buffer tartalmát
	memset(Buffer2, '\0', sizeof(Buffer2));	//Törõljük a buffer tartalmát

	MCUInit();
	// LTC6903 init SN76489N -  // 3.58MHz - 0xBD24
	// YM2612 - // 7.67 MHz - 0xCE40
	OSC_Init();
	init_usart();

	// VGM Play
	PSG_Silence();
	
	// Eprom olvasása
	PCMLoad = EepromRead();
	if(PCMLoad==0xFF) PCMLoad = 0;

	MemCim.value = 63;
	PCMMemAdr.value = 0;

	LATA &= 0x0F;
	LATD = 0;
	
	// Mielõtt elindítjuk a megszakítást, mentsük el 0xE0 parcskor érkezett PCM adat ugrásokat egy táblába
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
    * Teszteléshez készítettem egy timer0-ás rutint, hogy megtudjam
    * mennyi ideig tart egy mintavételezés.
    * Mért adat: 
    *
    ********************************************************************/
    /*HighIntTimer0();
    Timer0Count = 0;
    MemCim.value = 0x259;
    T0CONbits.TMR0ON = 1;
    
    for(i=0; i<1000; i++)
    Sample = MemRead(MemCim);	// 68,736uS alatt fut le (0.64 megzsakítás)
    
    //WaveSample();	// (0.98 megzsakítás) 105,25uS alatt fut le alap esetben, ha nem kell feldolgozni
    				// 0x52-es kóddal, vagy van kimeneti vezérlés legalább 1 esetben
    				// (1.01 megzsakítás) 108,47uS
    				
    // 0x8F parancs (1.14 megzsakítás) 122,436uS
    
    // 0xE0 parancs (4.6 megzsakítás) 494,04uS
   
    T0CONbits.TMR0ON = 0;
    Upload = 0;
    */
    
    // Osszcillószkópos mérés
    //for(;;)
    //{
    	//Sample = MemRead(MemCim);			// 64.4uS/2 = 32.2uS
    	
    	// Saját optimalizált változat
    	// Sample = MemReadFast(MemCim);	// 34.2uS/2 = 17.1uS
    	
    	// Minden várakoztatás és sallang nélkül
    	// Sample = MemReadFast(MemCim);	// 22.4uS/2 = 11.2uS
    	// WaveSample();					// 62uS/2 = 31uS
    	
    	// Alap parancs nélkül
    	// 1. Optimalizált olvasással 69uS/2 = 34.5uS alatt fut le
    	//WaveSample();					// 99.2uS/2 = 49.6uS
    	
    	    	
    	//Sample = SPI(0x00);			// 8.2uS/2 = 4.1uS
    	    	
    	/*/ SPI(0x00) fuggvény nélkül	// 4.2uS/2 = 2.1uS
    	SSPSTATbits.BF = 0;
		SSPCON1bits.WCOL = 0;
		Sample = SSPBUF;
		SSPBUF = 0x00;
		while(!SSPSTATbits.BF);
		Sample = SSPBUF;
		*/
    	
    	
    	//WaitForMem();				// 20.2uS/2 = 10.1uS - átszervezve 10.6uS/2 = 5.3uS
    	
    	// WaitForMem() függvény ugrás nélkül 8.4uS/2 = 4.2uS
    	/*do {
			MEM_E = 0;                      	 //kiadjuk a Chip Enable jelet
									
			SSPSTATbits.BF = 0;			// törli foglaltság jelzõ álapotát a státusz regiszterben
			SSPCON1bits.WCOL = 0;		// törli az esetleges írás ütközés hibajelzõt
			tmp = SSPBUF;				// törli a BF jelzõbitet
			SSPBUF = CMD_RDSR;				// kirakja a kimenõ adatot az SSPBUF regiszterbe
			while(!SSPSTATbits.BF);		// megvárjuk a busz ciklus végét 
						
			SSPSTATbits.BF = 0;			// törli foglaltság jelzõ álapotát a státusz regiszterben
			SSPCON1bits.WCOL = 0;		// törli az esetleges írás ütközés hibajelzõt
			tmp = SSPBUF;				// törli a BF jelzõbitet
			SSPBUF = 0x00;				// kirakja a kimenõ adatot az SSPBUF regiszterbe
			while(!SSPSTATbits.BF);		// megvárjuk a busz ciklus végét 
			
			MEM_E = 1;                   		 //megszüntetjük a Chip Enable jelet
		} while (SSPBUF & 0x01); 
		*/
		
		//LED1 = !LED1;
	//}
    /*******************************************************************
    Újabb optimalizálás - 2023-02-08.
    SPI olvasás átszervezése, amely az alap olvasái adatokat a következõ 
    képpen módosította:
   	Sample = MemRead(MemCim);			// 64.4uS/2 = 32.2uS
   	Sample = MemReadFast(MemCim);		// 34.2uS/2 = 17.1uS
   	WaveSample();						// 99.2uS/2 =  49.6uS - optimalizáltan 69uS/2 = 34.5uS
   	Az olvasási ciklusokat összevontam így nincs függvényugrás ezzel felére
   	csökkentve a szükséges olvasási idõt
   	Következõ lehetõség még a VGM fájl kiértékelésének átszervezése, 
   	fõként az olvasásoknál. Olvasások idõtartama még minden ciklus esetében
   	további 4.8uS idõvel csökkenthetõ. 
   	0x61-es parancs esetében ez 9.6uS/2 = 4.8uS
   	0xE0-ás parancs esetében 19.2uS/2 = 9.6uS
   	
   	Update - 2023-02-09.
   	SPI olvasást a minimálisra redukáltam, nincs memóriára várakozás és
   	semmi, ami tovább lassítaná a forgalmat. Sebesség így maximalizált.
   	Sample = MemReadFast(MemCim);		// 22.4uS/2 = 11.2uS
   	WaveSample();						// 62uS/2 = 31uS
   	Megszakításba a nyers adatokkal is kipróbáltam, de a PCM adatok esetében
   	még mindig nem elégséges a sebesség
   	Megszakításba pakolva mindent egy semleges 0x00-ás címmel, 140uS/2 = 70uS a lefutás
   	ami durva, mert 44.5/2 = 22.7uS-nél nem lehetne több. (ennyi a megszakítési ablak)
   	Ha a megszakításban waitSamples = WaveSample(); fuggvény van és az a 
   	MemReadFast(MemCim)-es WaveSample()-t hívja meg, akkor a lefutás 158.9uS/2 = 79.45uS
   	Ketõ közt 18.9uS/2 = 9.45uS idõ rés van, de ennél nem tudom, hogy lehetne még többet 
   	kighozni a PIC-bõl, max assembly programmal.
   	Ömlesztett kóddal 147.8uS/2 = 73.9uS
   	
   	Fõprogram 2.4uS/2 = 1.2uS egy ciklus ha UART feltételek benne vannak, ha csak a
   	LED billegtetése, akkor 600nS/2 = 300nS
   	Fõprogramba átpakolva a teljes kódot, gyorsabb, de az idõzítés ebben az
   	esetbe nem teljesen megoldható, mert 2 hangminta közt eltelt idõ attól 
   	számít, hogy mennyi idõ alatt értékelõdik ki a teljes VGM struktúra
   	és ebbõl le kellene vonni az alap idözítést.
   	****************************************************************************
   	* Nagyon fontos: 2023-02-09. Este
   	* Nem vettem észre (nem értem miért nem tûnt fel korábban) a mérések közben, 
   	* hogy amit mérek az két jel vagy is a LED egyszer világít, egyszer nem. 
   	* A periódikus jelem, amit mérek azt minden esetben 2 lefutás eredménye. 
   	* Tehát minden mérésem osztani kell kettõvel és ez lesz az egyszeri lefutás ideje.
   	****************************************************************************
   	Megszakításban az if feltételes verzió 96.59uS/2 = 48.3uS 0x00-ás paranccsal.
   	
   	Nos többedjére is oda jutok, hogy a program bárhogy optimalizálom, nem képes
   	22.7uS alatt vagy is egy megszakítási ablak alatt végbe menni. Emiatt lassú
   	a zene lejátszása. A program jelenleg az összes optimalizással egyben is
   	~ 51uS ideig dolgozik 1 hangmintán, pedig csak 22.7uS idõ áll rendelkezésére. 
   	
   	2023.02.10. Utolsó agyalásom eredménye és ezzel lehet is zárni a projektet.
   	Teljes átszervezés kapcsán arar jutottam, hogy a mérések alapján, ha csak 
   	az adatok kiküldésést tenném a 22.7uS-os megszakításba, akkor sem lenne jó
   	az eredmény, mert fõprogramomban tesztelt VGM kiértékelési struktúra lefutása
   	meghaladja a megszakítási idõablakot. Minden erõfeszítésem ellenére sem tudom
   	31uS alá vinni a VGM feldolgozás idejét. Ezzel egyértelmáen bebizonyosodott,
   	hogy a PIC18F452 nem képes a VGM feldolgozásra, ha az adatokban PCM adat is
   	van. Így a progjektet ezzel le is zárom. Továbbiakban más MCU-ra fejlesztek
   	tovább.
   	- PCM adatok PIC flash-be mentését kikapcsoltam, mert azt megszakításban nem
   	tudja kiszolgálni. PCM nélküli VGM-ek esetén használható a program, talán
   	kisebb PCM adatok esetén is elfogadható, de semmi kép nem tökéletes.
    */
    
    /*******************************************************************
    Késõbbre, ha elfelejteném:
    A timer2 és UART megszakítás nem megy egyben mert a timer2 megszakítás 
    olyan gyors, hogy a fõprogram nem tud lefutni egyszer sem, vagy az MCU
    vagy a szervezés hibás, de így ebben a formában csak, akkor mûködik, ha
    GOMB-al indítjuk a fõprogramot, tehát a feltõltés (UART) megszakítás
    mindaddig elérhetõ, ameddig nincs elindítva a timer2 megszakítás vagy is
    a lejátszás. Majd lehet egyszer újra kell szervezni, bár STM32F103RF
    jó választásnak tûnt egy másodlagos jobb verzió elkészítéséshez.
    2023-01-18. néztem és szerveztem újra a programot, hogy mûködjön.
    PCM adatok lejátszásrára a PIC sebessége (40MHz) nem elegendõ.
    
    - kiegészítve ezt annyiban, hogy az újabb mérések alapján a megszakítás
    nem képes 22.6uS alatt végbe menni mert a benne lévõ utasítások PCM adatot
    nem tartalmazó VGM esetében is ~94-150uS idõ alatt megy végbe, majd ezt
    követõen szabadulhat csak fel a megszakítás, de mivel kifutott az idõbõl
    egybõl a feldolgozás végén újabb megszakításba fordul bele. Így timer2
    megszakítás miatt a fõprogram nem tud egyszer sem lefutni
    
    - A program 256byte-os adatcsomagokkal dolgozik, tehát a C SHARP
    programban is átt kell írni vagy módosítani + 1 paraméter beállításával.
    
    2023-01-20. Normál PCM adatot nem tartalmazó VGM zenéknél átlagosan 
    ~94uS idõbe kerül egy-egy parancs kiértékelése, de vannak esetek,
    mikor több parancs is jön egymást követõen, akkor ez az idõ ~120-150uS
    idõt is igényelhet, így kizárt, hogy az elõre beállított 22.6uS-os
    megszakítás idõben végezni tudjon. Ebben az esetben még élvezhetõ a lejátszás.
    - PCM adat esetén fûggõen mekkora ez az adat ~330uS-ra nõ a visszatérési
    idõ, amely már hallhatóan hibás lejátszást eredményez. (nyûjtja a hangokat
    , lelassúl a lejátszás)
    Tehát ez a mikrokontroller a maga
    40MHz-es órajelével és az utasitásonként 4 illetve ugrásoknál 8 órajelet
    is igényelhetnek, amelyek önmagukban is sok idõt emésztenek fel.
    1 órajel 25nS
    1 utasítás 4 órajel vagy is ~100nS ideig tart
    1 ugrás 8 órajel vagy is ~200nS
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
			
			write_uart(0x10);	// Kiküldjük az OK visszajelzést az upload kódot
			while(DataSize<3);	// Amig nem jön meg a 3 adat
			// PC-s adat hossza
			DataSizePC = Buffer[2];
			DataSizePC = DataSizePC<<8 | Buffer[1];
			DataSizePC = DataSizePC<<8 | Buffer[0];
			Maradek = DataSizePC%BufferArraySize;	// Hossz vége, ha nem pontosan egyelõ BufferArraySize
			Pack = DataSizePC/BufferArraySize;		// Adat csomagok száma

			memset(Buffer, '\0', sizeof(Buffer));	//Törõljük a buffer tartalmát
			memset(Buffer2, '\0', sizeof(Buffer2));	//Törõljük a buffer tartalmát
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
					
			 	if(Error && BufferSize==BufferArraySize){		// Ha hiba van újra bekérjük az adatcsomagot
					Error = 0;									// Error-t nullázuk
					DataSize -= BufferArraySize;				// 64-et kivonunk a méretbõl
					x++;										// Számoljuk hány hiba volt
					PackCount--;
					TXREG = 0x01;								// kiküldjük a hiba, ismétlõ kódot
					while(!PIR1bits.TXIF);
				}else if(BufferSize==BufferArraySize || (PackCount == Pack && Maradek == BufferSize)){		// Ha minden rendben van
					
					if((PackCount == Pack && Maradek == BufferSize)){ temp = Maradek; } else { temp = BufferArraySize; }
					// Beírjuk az új adatokat a memóriába

					////// Két buffer töltésének írányítása, szervezése /////
					bUffEr = !bUffEr;
					BufferSize=0;
					BufferI=0;
					PackCount++;
					if(PackCount<=Pack){
						TXREG = 0x02;				// kiküldjuk a tovább kódot
						while(!PIR1bits.TXIF);
					}
					//////////////////////////////////////////////////////////
		
					if(!bUffEr){
						FastMemWrite(MemCim, Buffer);
					}else{
						FastMemWrite(MemCim, Buffer2);
					}
					
					// Adat kiírása AddresCounter, 
					MemCim.value += temp;
				
					if(DataSizePC == DataSize && PackCount >= Pack){ 
						Upload = 0; 
						bUffEr = 1; 				// Alapértelmezett buffer használata
						BufferSize=0;
						BufferI=0;
						TXREG = 0x02;				// kiküldjuk a tovább kódot
						while(!PIR1bits.TXIF);
						
						MemCim.value = 0;
						memset(Buffer, '\0', sizeof(Buffer));	//Törõljük a buffer tartalmát
						memset(Buffer2, '\0', sizeof(Buffer2));	//Törõljük a buffer tartalmát
					}// Kilépünk írásmódból

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
						
		
		//if(LED1==1) LED1 = 0; else LED1 = 1; 	// 1.3uS ciklussal együtt
		
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
					// Mintavételezés várakoztatása
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
					// PCM adat kezelése
					MemCim.value++;	// Skip 0x66
					MemCim.value++; // Skip data type
					for ( i = 0; i < 4; i++ )
					{
						MemCim.value++;
						PCMDataSize += ( (unsigned long)(MemReadFast(MemCim)) << ( 8 * i ));
					}
					// Ha van PCM adat és ha beleférünk a programemóriába
					if(PCMDataSize > 0 && PCMDataSize <= MaxPCMData && !PCMLoad)
					{
						// Program memóra törlése \\
						//osztas = PCMDataSize/PMBlock;	// 64Kbyte-al történõ osztás egész része
						//maradek = PCMDataSize%PMBlock;	// Osztás utáni maradék, ha van
						osztas = Osztas(PCMDataSize, PMBlock, 0);
						maradek = Osztas(PCMDataSize, PMBlock, 1);
						if(maradek>0) 
							osztas++;					// Ha van maradék akkor osztást emeljuk 1-el
						for(i=0; i<osztas; i++)			// Töröljük a szükséges területet
							PICFlashBlockErase(PMemAddrStart+(i*PMBlock));
						// Program memória törlése VÉGE //
						
						// Betöltjük az adatokat majd beírjuk programmemóriába \\
						//osztas = PCMDataSize/BufferArraySize;	// 256Kbyte-al történõ osztás egész része
						//maradek = PCMDataSize%BufferArraySize;	// Osztás utáni maradék, ha van
						osztas = Osztas(PCMDataSize, BufferArraySize, 0);
						maradek = Osztas(PCMDataSize, BufferArraySize, 1);
						if(maradek>0) 
							osztas++;
						// Ciklus ameddig adat végére nem érünk
						for(x=0; x<osztas; x++){
							// Buffer feltöltése
							for(i=0; i<BufferArraySize; i++)
							{
								MemCim.value++;
								Buffer[i] = MemReadFast(MemCim);
							}
							// Buffer kiírása PM-ba
							PICFlashBlockWrite(PMemAddrStart+(x*BufferArraySize), Buffer);
							memset(Buffer, '\0', sizeof(Buffer));	//Törõljük a buffer tartalmát
						}
						// Beírás VÉGE
						
						BuffIndex=0;
						
						// EEPROM-ban jelezzük, hogy PCM már volt betöltve
						EepromWrite(0x01);
						PCMLoad = 1;
					}
					else	// Ha nem férünk bele a programemóriába
					{
						// PCM kezdõ adat címe
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

