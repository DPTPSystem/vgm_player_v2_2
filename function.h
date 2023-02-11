                    
void main(void);
void MCUInit(void);
void LowInterruptIniTimer2(void);
unsigned char SPI(unsigned char adat);
volatile unsigned int WaveSample(void);
volatile unsigned int WaveSampleIf(void);
void PCMJumpSave(void);

// 25LC640 Memória
void WaitForMem(void);									//Várakozás a SPI memóriára
void MemWrite(union16 MemCim, unsigned char pbuf);
unsigned char MemRead(union16 MemCim); 
unsigned char MemReadFast(union16 MemCim);	
void MemErase(void);
void MemBlockErase64Kb(union16 MemAddr);				
void FastMemWrite(union16 MemCim, unsigned char *pbuf);	//Memória írása
void FastMemRead(union16 MemCim, unsigned char *pbuf);	//Meria olvasása

// PIC18F452 program memóriájának törlése 64byte / block
void PICFlashBlockErase(unsigned int Addr);
// PIC18F452 program memória írása 64byte / block és 8 byte / írás
void PICFlashBlockWrite(unsigned int Addr, unsigned char *Data);
// PIC18F452 program memória olvasása, byte-os
static unsigned char PICFlashReadByte(unsigned int addr);

unsigned char Osztas(unsigned int Temp, unsigned int oszto, unsigned char Select);
unsigned char EepromRead (void);
void EepromWrite (unsigned char adat);

// UART
void init_usart(void);
void write_uart(unsigned char data);
unsigned char read_uart(void);

void MCUInit(void){
	TRISA = 0b00000010;			
	TRISB = 0b00000000;		
	TRISC = 0b10010000;		
	TRISD = 0;
	TRISE = 0;

	INTCON = 1;					// A interrupt tiltása

	RCONbits.IPEN = 1;			// Kétszintû megszakítási mód beállítása
	INTCONbits.GIEH = 1;		// A magas prioritású interrupt
    INTCONbits.GIEL = 1;		// Az alacsony prioritású interrupt
	INTCONbits.GIE = 1;			// Megszakítások engedélyezése
	
	PIE1bits.RCIE = 1;			// USART Receive Interrupt Enable bit
	IPR1bits.RCIP = 1;			// USART Receive Interrupt Priority bit
	
	PIE1bits.TXIE = 0;			// USART Transmit Interrupt disabled bit
	IPR1bits.TXIP = 0;			// USART Transmit Interrupt Priority bit

	ADCON1 = 0x07;				// Minden digitális
	ADCON0 = 0;

	CCP1CON = 0;				// Coperátorok kikapcsolva
	
	SN_OE = 0;					// Kimenet SN engedélyezése
	SN_OSC_E = 1;
	YM_OSC_E = 1;
	SN_WE = 1;
	MEM_E = 1;

	YM_IC = 1;
	YM_CS = 1;
	YM_WR = 1;
	YM_RD = 1;
	YM_A0 = 0;
	YM_A1 = 0;

	// SPI init
	SPI_SO_TRIS = 0;		//B0 láb SDI bemenet
	SPI_SCK_TRIS = 0;		//B1 láb SCK kimenet
	SPI_SI = 1;

	SSPSTATbits.SMP = 0;	//Mintavételezés idõzítése Master mód esetén 1: mintavételezés a kimeneti idõ végén, 0: mintavételezés a kimeneti idõ közepén). Slave módban kötelezõen nullába kell állítani.
	SSPSTATbits.CKE = 1;	//Órajel fázisának beállítása (1: az adatkimenet beállítása akkor történik,amikor az órajel aktív állapotból tétlenre vált, 0: az adatkimenet beállítása akkor történik, amikor az órajel tétlen állapotból aktív szintre vált)
	SSPSTATbits.BF = 0;		//A buffer foglaltságát jelzõ állapotbit  (1: az adatátvitel lezajlott, SSPBUF kiolvasható, 0: az adatbuffer üres)
	
	SSPCON1bits.WCOL = 	0;	//Ütközés detektálás írásnál (1: írást/adatküldést kíséreltünk meg, amikor nem lehetett, 0: nincs ütközés)
	SSPCON1bits.SSPOV = 0;	//Vételi túlcsordulás-jelzõ (1: újabb bájt érkezett, de SSPBUF meg nincs kiolvasva, 0: nincs túlfolyás)
	SSPCON1bits.SSPEN = 1;	//Master Szinkron Soros Port engedélyezése (1: engedélyezve, 0: letiltva)
	SSPCON1bits.CKP = 	0;	//Órajel polaritása (1: aktív állapotban alacsony szint, 0: aktív állapotban magas szint)
							//Master Szinkron Soros Port üzemmódjának beállítása:
							//0101 = SPI Slave mód, az SS láb használata nélkül
							//0100 = SPI Slave mód, amelynél az SS láb is használatban van
							//0011 = SPI Master mód, Timer2/2 órajel frekvenciával
							//0010 = SPI Master mód, Fosc/64 órajel frekvenciával
							//0001 = SPI Master mód, Fosc/16 órajel frekvenciával
							//0000 = SPI Master mód, Fosc/4 órajel frekvenciával
	SSPCON1bits.SSPM0 = 0;
	SSPCON1bits.SSPM1 = 0;
	SSPCON1bits.SSPM2 = 0;
	SSPCON1bits.SSPM3 = 0;
	LowInterruptIniTimer2();
}

// Idõméréshez látrehozott megszakítás
void HighIntTimer0(void)
{
	// Számoláshoz, 8bit-es mód
	// (FOSC/4) / Prescaler / TMR0H TMR0L
    // (40M/4) / 	2		/ 1
	INTCON2bits.TMR0IP = 1;		// Timer0 megszakítás magas prioritású legyen
	
	T0CONbits.TMR0ON = 0;		// Timer0 kikapcsolva
	T0CONbits.T08BIT = 1;		// 8 bites mód kiválasztás
	T0CONbits.T0CS = 0;			// Meghajtás belsõ órajelrõl
	T0CONbits.T0SE = 0;			// Külsõ órajel fel- vagy lefutó élének választása
	T0CONbits.PSA = 0;			// Elõosztás bekapcsolása
	//Elõosztási arány beállítása (000 = 1:2, 001 = 1:4, 010 = 1:8, 011 = 0:16,
								// 100 = 1:32, 101 = 1:64, 110 = 1:128, 111 = 1:256)
	T0CONbits.T0PS2 = 0;		// Elõosztás 1:2 osztásra állítása
	T0CONbits.T0PS1 = 0;
	T0CONbits.T0PS0 = 0;
	INTCONbits.TMR0IE = 1;			// Timer tiltása
	//TMR0H = 0xCD;
	// 0x01 107.4uS/2 = 53.7uS - Szkóppal mérve
	// 0xFF 7.8uS/2 = 3.9uS - Szkóppal mérve
	TMR0L = 0xFF;
}

//Alacsony piorítású interrupt indítása
void LowInterruptIniTimer2(void){
	/*********************************************
	+		TIMER2 & INTERRUPT INIT
	+	Timer2 low interrupt
	**********************************************/
    // Timer2
    // (FOSC/4) / Prescaler / PR2 / Postscaler
    // (40M/4) / 	1		/ 112 / 1
    // Actual Interrupt Time : 11,2 uS 
    // |-|_| egy teljes hullám 22.6 uS, 
	T2CONbits.TOUTPS0 = 0;
	T2CONbits.TOUTPS1 = 0;
	T2CONbits.TOUTPS2 = 0;
	T2CONbits.TOUTPS3 = 0;
	T2CONbits.T2CKPS0 = 0;
	T2CONbits.T2CKPS1 = 0;
	PR2 = 226;				// 44100 Hz (226)
	IPR1bits.TMR2IP = 0;	// Alacsony megszakítás engedélyezése
	PIR1bits.TMR2IF = 0;
	PIE1bits.TMR2IE = 1;	// TMR2 engedélyezõ, 0 = Enabled
	T2CONbits.TMR2ON = 0;
}

unsigned char SPI(unsigned char adat){
	unsigned char tmp;  
	SSPSTATbits.BF = 0;			// törli foglaltság jelzõ álapotát a státusz regiszterben
	SSPCON1bits.WCOL = 0;		// törli az esetleges írás ütközés hibajelzõt
	tmp = SSPBUF;				// törli a BF jelzõbitet
	SSPBUF = adat;				// kirakja a kimenõ adatot az SSPBUF regiszterbe
	while(!SSPSTATbits.BF);		// megvárjuk a busz ciklus végét 
	return (SSPBUF);			// a vett bájttal térünk vissza
}

void OSC_Init(void)
{
	//unsigned short data = 0;
	// LTC6903 init SN76489N -  //3.58MHz
	//short data = 0xBD24; //(11 << 12) | (841 << 2);
	//data = LTC6903_SetFrequency(3580000);
	// 1011 1101 0000 0000 - 0xBD00

	// 1011 0000 0000 0000
	// 		1101 0010 0100 - BD24 (841)
	//      1100 1111 1100 - BCFC
	/*SN_OSC_E = 0;
	SPI(data>>8);
	SPI(data);
	SN_OSC_E = 1;*/

	// YM2612 7.67 MHz - CE40
	short data = 0xCE40; //(12 << 12) | (912 << 2);
	YM_OSC_E = 0;
	SPI(data>>8);
	SPI(data);
	YM_OSC_E = 1;
}
void WaitForMem(void) {
	/*
	unsigned int flag;
	do {
		MEM_E = 0;                      	 //kiadjuk a Chip Enable jelet
		SPI(CMD_RDSR);                       //Státuszregiszter olvasása parancs
		flag = SPI(0x00);                    //Olvasunk, de valamit akkor is küldeni kell!
		MEM_E = 1;                   		 //megszüntetjük a Chip Enable jelet
	} while (flag & 0x01); 
	*/
	unsigned char tmp;
	do {
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
}

// Lefutás fuggvényen belül: 8.4uS/2 = 4.2uS + 5*4.2uS = 29.4uS/2 = 14.7uS
// Fuggvény hívása + 4.8uS/2 = 2.4uS. Összesen: 34.2uS/2 = 17.2uS
// Függvénybõl minden sallangot kivettem ami nem kell
// Csak a legszükségesebb részek maradtak
unsigned char MemReadFast(union16 MemCim)
{
	unsigned char tmp;
	
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
	tmp = SSPBUF;
	SSPBUF = 0x00;
	while(!SSPSTATbits.BF);
	
	MEM_E = 1;
	return (SSPBUF);
}

void MemWrite(union16 MemCim, unsigned char pbuf)
{
	WaitForMem();
	MEM_E = 0;
	SPI(CMD_WREN);
	MEM_E = 1;
	Delay_Ms(1);
	MEM_E = 0;
	SPI(CMD_WRITE);
	SPI(MemCim.low);
	SPI(MemCim.high);
	SPI(MemCim.hh);
	SPI(pbuf);
	MEM_E = 1;
}

unsigned char MemRead(union16 MemCim) 
{
	unsigned char temp = 0;
	WaitForMem();
	MEM_E = 0;
	SPI(CMD_READ);
	SPI(MemCim.low);
	SPI(MemCim.high);
	SPI(MemCim.hh);
	temp = SPI(0x00);
	MEM_E = 1;
	return temp;
}

void MemErase(void){
	WaitForMem();
	MEM_E = 0;
	SPI(CMD_WREN);
	MEM_E = 1;
	Delay_Ms(1);
	MEM_E = 0;
	SPI(CMD_Erase);
	MEM_E = 1;
	WaitForMem();
}
void MemBlockErase64Kb(union16 MemAddr)
{
	WaitForMem();
	MEM_E = 0;
	SPI(CMD_WREN);
	MEM_E = 1;
	Delay_Ms(1);
	MEM_E = 0;
	SPI(CMD_BLCK64Kb);
	SPI(MemAddr.low);
	SPI(MemAddr.high);
	SPI(MemAddr.hh);
	MEM_E = 1;
	WaitForMem();
}
void FastMemWrite(union16 MemCim, unsigned char *pbuf) {
	unsigned int i;
	WaitForMem();                         //Várunk, ha az EEPROM elfoglalt
	MEM_E = 0;
	SPI(CMD_WREN);                       //Írás újraengedélyezése
	MEM_E = 1;
	Delay_Ms(1);
	MEM_E = 0;
	SPI(CMD_WRITE);                        //Adatblokk írása
	SPI(MemCim.low);
	SPI(MemCim.high);
	SPI(MemCim.hh);
	for (i=0; i < PMEMBLOCK; i++) {
		SPI(*pbuf++);
	}
	MEM_E = 1;
}

void FastMemRead(union16 MemCim, unsigned char *pbuf) {
	unsigned int i;
	unsigned char x = 4;
	do
	{
		WaitForMem();                         //Várunk, ha az EEPROM elfoglalt
		MEM_E = 0;
		SPI(CMD_FREAD);                         //Adatblokk olvasása
		SPI(MemCim.low);
		SPI(MemCim.high);
		SPI(MemCim.hh);
		for (i=0; i < 64; i++) {				// Maximum 64byte (W25Q64)
			*pbuf++=SPI(0x00);                   //Olvasunk, de valamit akkor is küldeni kell!
		}
		MEM_E = 1;
		MemCim.value += 64;
		x--;
	}while(x<4);
}

void init_usart(void){
	TRISC = 0b10010000;
	TXSTA = 0b00100100;
	RCSTA = 0b00010000;
	//Baud rate kiszámolása (pl:9600 baud rate)
	/*((FOSC/Desired Baud Rate)/64) – 1
	= ((40000000/9600)/64) – 1
	= 64*/
	// 115200 BR - 21
	// 128000 BR - 19 (11kb/s)
	// 256000 BR - 9
	// 403200 BR - 5
	// 614400 BR - 3
	// 1228800 BR - 1 (65kb/s)
	// 2457600 BR  - 0
	SPBRG = 19;
	RCSTAbits.SPEN = 1;
}
void write_uart(unsigned char data){
	TXREG = data;
	while(!PIR1bits.TXIF);
}
unsigned char read_uart(void){
	RCSTAbits.CREN = 1;
	while(!PIR1bits.RCIF);
	return RCREG;
}
	
volatile unsigned int WaveSample(void)
{
	unsigned float wait = 0;
	unsigned char Sample = 0;
	unsigned char YM_address = 0;
	unsigned char YM_data = 0;
	unsigned int i = 0, x = 0;
	unsigned char osztas = 0, maradek = 0;
	
	unsigned char temp = 0;
	unsigned char tmp;
	
	// Sample = MemRead(MemCim);		// 64.4uS/2 = 32.2uS alatt olvas 1 byte-ot
	// Sample = MemReadFast(MemCim);	// 34.2uS/2 = 17.1uS alatt olvas 1 byte-ot
	// Nyers adat lekérdezés			// 29.4uS/2 = 14.7uS alatt olvas 1 byte-ot
	
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
			return 0;
		case 0x50:
			MemCim.value++;
			SN76489_SendByte(MemReadFast(MemCim));
			return 0;
		case 0x52:
			/*MemCim.value++;
			YM_address = MemReadFast(MemCim);
			MemCim.value++; 
			YM_data = MemReadFast(MemCim);
			YM2612_Send(YM_address, YM_data, Sample);
			return 0;*/
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
			return 0;
		/*case 0x58:
		case 0x59:
			// 0x58, 0x59 CDM - NeoGeo 2610
			MemCim.value++;
			YM_address = MemReadFast(MemCim);
			MemCim.value++;
			YM_data = MemReadFast(MemCim);
			YM2610_Send(YM_address, YM_data, Sample);
			break;*/
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
			return wait/Oszto;
		case 0x62:
			return 735.0/Oszto;
		case 0x63:
			return 882.0/Oszto;
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
			return 0;
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
			return wait/Oszto;
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
			return wait/Oszto;
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
			return 0;
		case 0x66:
			MemCim.value=63;
			return 0;
		default:
			return 0;
	}
	return 0;
}

volatile unsigned int WaveSampleIf(void)
{
	unsigned float wait = 0;
	unsigned char Sample = 0;
	unsigned char YM_address = 0;
	unsigned char YM_data = 0;
	unsigned int i = 0, x = 0;
	unsigned char osztas = 0, maradek = 0;	
	unsigned char temp = 0;
	unsigned char tmp;
	
	MEM_E = 0;
	tmp = SSPBUF;
	SSPBUF = CMD_READ;
	while(!SSPSTATbits.BF);
	tmp = SSPBUF;
	SSPBUF = MemCim.low;
	while(!SSPSTATbits.BF);
	tmp = SSPBUF;
	SSPBUF = MemCim.high;
	while(!SSPSTATbits.BF);
	tmp = SSPBUF;
	SSPBUF = MemCim.hh;
	while(!SSPSTATbits.BF);
	temp = SSPBUF;
	SSPBUF = 0x00;
	while(!SSPSTATbits.BF);	
	MEM_E = 1;	
	Sample = (SSPBUF);
	if(Sample==0x4F)
	{		
		MemCim.value++;
		SN76489_SendByte(0x06);
		SN76489_SendByte(MemReadFast(MemCim));
		return 0;
	}
	else if(Sample==0x50)
	{
		MemCim.value++;
		SN76489_SendByte(MemReadFast(MemCim));
		return 0;
	}
	else if(Sample==0x52 || Sample==0x53)
	{
		MemCim.value++;
		MEM_E = 0;
		tmp = SSPBUF;
		SSPBUF = CMD_READ;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.low;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.high;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.hh;
		while(!SSPSTATbits.BF);
		temp = SSPBUF;
		SSPBUF = 0x00;
		while(!SSPSTATbits.BF);
		MEM_E = 1;
		YM_address = (SSPBUF);
		
		MemCim.value++;
		MEM_E = 0;
		
		tmp = SSPBUF;
		SSPBUF = CMD_READ;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.low;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.high;
		while(!SSPSTATbits.BF);
		tmp = SSPBUF;
		SSPBUF = MemCim.hh;
		while(!SSPSTATbits.BF);
		temp = SSPBUF;
		SSPBUF = 0x00;
		while(!SSPSTATbits.BF);
		
		MEM_E = 1;
		
		YM_data = (SSPBUF);
			
		YM2612_Send(YM_address, YM_data, Sample);
		return 0;
	}			
	else if(Sample==0x61)
	{
		wait = 0;
		for ( i = 0; i < 2; i++ )
		{
			MemCim.value++;	
			MEM_E = 0;
			tmp = SSPBUF;
			SSPBUF = CMD_READ;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = MemCim.low;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = MemCim.high;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = MemCim.hh;
			while(!SSPSTATbits.BF);
			temp = SSPBUF;
			SSPBUF = 0x00;
			while(!SSPSTATbits.BF);
			MEM_E = 1;
			wait += ( (unsigned int)((SSPBUF)) << ( 8 * i ));
		}
		return wait/Oszto;
	}	
	else if(Sample==0x62)
	{
		return 735.0/Oszto;
	}		
	else if(Sample==0x63)
	{
		return 882.0/Oszto;
	}		
	else if(Sample==0x67)
	{
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
		return 0;
	}
	else if(Sample>=0x70 && Sample<=0x7F)
	{	
		wait = (Sample & 0x0F)+1;
		return wait/Oszto;
	}
	else if(Sample>=0x80 && Sample<=0x8F)
	{	
		wait = Sample & 0x0F;
		YM_address = 0x2A;
		if(PCMLoad)
		{
			BuffIndex++;
			TBLPTR = PMemAddrStart+BuffIndex;
			_asm TBLWTPOSTINC _endasm;
   			_asm TBLRDPOSTDEC _endasm;
			YM_data = TABLAT;
		}
		else
		{
			PCMMemAdr.value++;
			MEM_E = 0;
			tmp = SSPBUF;
			SSPBUF = CMD_READ;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = PCMMemAdr.low;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = PCMMemAdr.high;
			while(!SSPSTATbits.BF);
			tmp = SSPBUF;
			SSPBUF = PCMMemAdr.hh;
			while(!SSPSTATbits.BF);
			temp = SSPBUF;
			SSPBUF = 0x00;
			while(!SSPSTATbits.BF);
			MEM_E = 1;
			YM_data = (SSPBUF);
		}	
		YM2612_Send(YM_address, YM_data, Sample);
		return wait/Oszto;
	}	
	else if(Sample==0xE0)
	{
		if(PCMLoad)
		{
			BuffIndex = 0;
			for (i = 0; i < 4; i++ )
			{
				MemCim.value++;
				MEM_E = 0;
				tmp = SSPBUF;
				SSPBUF = CMD_READ;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.low;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.high;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.hh;
				while(!SSPSTATbits.BF);
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
				MEM_E = 0;
				tmp = SSPBUF;
				SSPBUF = CMD_READ;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.low;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.high;
				while(!SSPSTATbits.BF);
				tmp = SSPBUF;
				SSPBUF = MemCim.hh;
				while(!SSPSTATbits.BF);
				temp = SSPBUF;
				SSPBUF = 0x00;
				while(!SSPSTATbits.BF);
				MEM_E = 1;
				PCMMemAdr.value += ( (unsigned int)((SSPBUF)) << ( 8 * i ));
			}
		}
		return 0;
	}	
	else if(Sample==0x66)
	{
		MemCim.value=63;
		return 0;
	}		
	else
	{
		return 0;
	}
}
void PCMJumpSave(void)
{
	unsigned char Sample = 0;
	unsigned int i = 0, x = 0;
	unsigned char Flag = 1;
	
	while(Flag)
	{
		MemCim.value++;
		Sample = MemReadFast(MemCim);
		switch(Sample) //VGM commands
		{
			case 0x4F:
				MemCim.value++;
				break;
			case 0x50:
				MemCim.value++;
				break;
			case 0x52:
			case 0x53:
				MemCim.value++;
				MemCim.value++;
				break;
			case 0x61:
				MemCim.value++;
				MemCim.value++;
				break;
			case 0x62:
			case 0x63:
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
				MemCim.value+=PCMDataSize;
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
				break;
			case 0xE0:
				for (i = 0; i < 4; i++ )
				{
					MemCim.value++;
					JumpTableE0[BuffIndex] += ( (unsigned int)(MemReadFast(MemCim)) << ( 8 * i ));
				}	
				BuffIndex++;
				break;
			case 0x66:
				MemCim.value=63;
				BuffIndex=0;
				Flag = 0;
				break;
			default:
				break;
		}
	}
}


// PIC18F452 program memóriájának törlése 64byte / block
void PICFlashBlockErase(unsigned int Addr)
{
	TBLPTR=Addr;
	//----------------- Blokk törlés ----------------------------------
	EECON1bits.EEPGD=1;			// Flash Program vagy adat EEPROM memória vélasztó bit
								// 1 = Hozzáférés a flash program memóriához
								// 0 = Hozzáférés az EEPROM memóriához
	EECON1bits.CFGS=0;			// Flash Program / adat EEPROM vagy konfiguráció választó bit
								// 1 = Hozzáférés a konfigurációs nyilvántartásokhoz
								// 0 = Hozzáférés a Flash program vagy adat EEPROM memóriához
	EECON1bits.WREN=1;			// Flash Program / adat EEPROM írást engedélyezõ bit
								// 1 = Engedélyezi a Flash program / adat EEPROM írását
								// 0 = Gátolja a Flash program / adat EEPROM írását
	EECON1bits.FREE=1;			// flash törlésének engedélyezése
								// 1 = Törlés engedélyezése
								// 0 = Csak olvasás
	INTCONbits.GIE=0;			// Megszakítás letiltva
	EECON2=0x55;				// EEPROM nem fizikai adat regiszter, írás és törlési szekvencia
	EECON2=0xAA;
	EECON1bits.WR=1;			// Írást vezérlõ bit
								// 1 = kezdeményez adat EEPROM törlés / írás ciklust vagy a program memória törlési ciklust vagy írási ciklus
								// 0 = Write ciklus az EEPROM teljes
	while(EECON1bits.WR);		// Várakozunk az írás befejezéséig
	Nop();						// 1 órajel szünet
	INTCONbits.GIE=1;			// Megszakítás engedélyezve
	EECON1bits.WREN=0;			// Flash Program / adat EEPROM írást engedélyezõ bit
								// 1 = Engedélyezi a Flash program / adat EEPROM írását
								// 0 = Gátolja a Flash program / adat EEPROM írását
}

// PIC18F452 program memória írása 64byte / block és 8 byte / írás
void PICFlashBlockWrite(unsigned int Addr, unsigned char *Data)
{
	unsigned char i = 0;
	unsigned int K = 0;
	TBLPTR=Addr;
	EECON1bits.WREN=0;				// Flash Program / adat EEPROM írást engedélyezõ bit
									// 1 = Engedélyezi a Flash program / adat EEPROM írását
									// 0 = Gátolja a Flash program / adat EEPROM írását
	for(K=0;K<BufferArraySize;K=K+PMWbyte)     
	{
		for(i=0;i<PMWbyte;i++)
		{
		   TABLAT = Data[K+i];
		   _asm TBLWTPOSTINC _endasm;
		}
		_asm TBLRDPOSTDEC _endasm;
		EECON1bits.EEPGD=1;			// Flash Program vagy adat EEPROM memória vélasztó bit
									// 1 = Hozzáférés a flash program memóriához
									// 0 = Hozzáférés az EEPROM memóriához
		EECON1bits.CFGS=0;			// Flash Program / adat EEPROM vagy konfiguráció választó bit
									// 1 = Hozzáférés a konfigurációs nyilvántartásokhoz
									// 0 = Hozzáférés a Flash program vagy adat EEPROM memóriához
		EECON1bits.WREN=1;			// Flash Program / adat EEPROM írást engedélyezõ bit
									// 1 = Engedélyezi a Flash program / adat EEPROM írását
									// 0 = Gátolja a Flash program / adat EEPROM írását
		INTCONbits.GIE=0;			// Megszakítás letiltva
		EECON2=0x55;				// EEPROM nem fizikai adat regiszter, írás és törlési szekvencia
		EECON2=0xAA;
		EECON1bits.WR=1;			// Írást vezérlõ bit
									// 1 = kezdeményez adat EEPROM törlés / írás ciklust vagy a program memória törlési ciklust vagy írási ciklus
									// 0 = Write ciklus az EEPROM teljes
		while(EECON1bits.WR);		// Várakozunk az írás befejezéséig
		Nop();						// 1 órajel szünet
		EECON1bits.WREN=0;			// Flash Program / adat EEPROM írást engedélyezõ bit
									// 1 = Engedélyezi a Flash program / adat EEPROM írását
									// 0 = Gátolja a Flash program / adat EEPROM írását
		INTCONbits.GIE=1;			// Megszakítás engedélyezése
		_asm TBLRDPOSTINC _endasm;
	}
}	

static unsigned char PICFlashReadByte(unsigned int Addr)
{
    unsigned char result = 0, i = 0;
    INTCONbits.GIE = 0;     // Disable interrupts
    TBLPTR=Addr;
    _asm TBLWTPOSTINC _endasm;
    _asm TBLRDPOSTDEC _endasm;
    result = TABLAT;
    INTCONbits.GIE = 1;     // Enable interrupts
    return result;
}

unsigned char Osztas(unsigned int Temp, unsigned int oszto, unsigned char Select)
{
	unsigned char osztas = 0, maradek = 0;
	osztas = Temp/oszto;	// 64Kbyte-al történõ osztás egész része
	maradek = Temp%oszto;	// Osztás utáni maradék, ha van	
	if(!Select)
		return osztas;
	else
		return maradek;
}	

/*********************************************
+		Eeprom olvasása
**********************************************/
unsigned char EepromRead (void)
{
	unsigned char buffer;
	EEADR = 0;				// Memória címtõl kezdi az olvasást
	EECON1bits.EEPGD = 0;	// Flash Program vagy adat EEPROM memória vélasztó bit
							// 1 = Hozzáférés a flash program memóriához
							// 0 = Hozzáférés az EEPROM memóriához
	EECON1bits.CFGS = 0;	// Flash Program / adat EEPROM vagy konfiguráció választó bit
							// 1 = Hozzáférés a konfigurációs nyilvántartásokhoz
							// 0 = Hozzáférés a Flash program vagy adat EEPROM memóriához	
	EECON1bits.RD = 1;		// Olvasást vezérlõ bit
							// 1 = kezdeményezi az EEPROM olvasását (RD bitet nem lehet beállítani , ha EEPGD = 1 vagy CFGS = 1 )
							// 0 = Nem kezdeményez EEPROM olvasás
	buffer = EEDATA;		// Kiolvassuk az adatot
	return buffer;
}

/*********************************************
+		Eeprom írása
**********************************************/
void EepromWrite (unsigned char adat)
{
	EEADR = 0;				// Memória címtõl kezdjük az írást
	EEDATA = adat;
	EECON1bits.EEPGD = 0;	// Flash Program vagy adat EEPROM memória vélasztó bit
							// 1 = Hozzáférés a flash program memóriához
							// 0 = Hozzáférés az EEPROM memóriához
	EECON1bits.CFGS = 0;	// Flash Program / adat EEPROM vagy konfiguráció választó bit
							// 1 = Hozzáférés a konfigurációs nyilvántartásokhoz
							// 0 = Hozzáférés a Flash program vagy adat EEPROM memóriához
	EECON1bits.WREN = 1;	// Flash Program / adat EEPROM írást engedélyezõ bit
							// 1 = Engedélyezi a Flash program / adat EEPROM írását
							// 0 = Gátolja a Flash program / adat EEPROM írását
	INTCONbits.GIE = 0;		//Tiltjuk a interrupt-ot (megszakítást)
	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;		// Írást vezérlõ bit
							// 1 = kezdeményez adat EEPROM törlés / írás ciklust vagy a program memória törlési ciklust vagy írási ciklus
							// 0 = Write ciklus az EEPROM teljes
	while (EECON1bits.WR == 1);	//Várakozunk az írás befejezéséig
	INTCONbits.GIE = 1;		// engedélyezzük az interrupt-ot (megszakítást)
	EECON1bits.WREN = 0;	// Flash Program / adat EEPROM írást engedélyezõ bit
							// 1 = Engedélyezi a Flash program / adat EEPROM írását
							// 0 = Gátolja a Flash program / adat EEPROM írását
}
