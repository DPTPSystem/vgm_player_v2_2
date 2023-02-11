                    
void main(void);
void MCUInit(void);
void LowInterruptIniTimer2(void);
unsigned char SPI(unsigned char adat);
volatile unsigned int WaveSample(void);
volatile unsigned int WaveSampleIf(void);
void PCMJumpSave(void);

// 25LC640 Mem�ria
void WaitForMem(void);									//V�rakoz�s a SPI mem�ri�ra
void MemWrite(union16 MemCim, unsigned char pbuf);
unsigned char MemRead(union16 MemCim); 
unsigned char MemReadFast(union16 MemCim);	
void MemErase(void);
void MemBlockErase64Kb(union16 MemAddr);				
void FastMemWrite(union16 MemCim, unsigned char *pbuf);	//Mem�ria �r�sa
void FastMemRead(union16 MemCim, unsigned char *pbuf);	//Meria olvas�sa

// PIC18F452 program mem�ri�j�nak t�rl�se 64byte / block
void PICFlashBlockErase(unsigned int Addr);
// PIC18F452 program mem�ria �r�sa 64byte / block �s 8 byte / �r�s
void PICFlashBlockWrite(unsigned int Addr, unsigned char *Data);
// PIC18F452 program mem�ria olvas�sa, byte-os
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

	INTCON = 1;					// A interrupt tilt�sa

	RCONbits.IPEN = 1;			// K�tszint� megszak�t�si m�d be�ll�t�sa
	INTCONbits.GIEH = 1;		// A magas priorit�s� interrupt
    INTCONbits.GIEL = 1;		// Az alacsony priorit�s� interrupt
	INTCONbits.GIE = 1;			// Megszak�t�sok enged�lyez�se
	
	PIE1bits.RCIE = 1;			// USART Receive Interrupt Enable bit
	IPR1bits.RCIP = 1;			// USART Receive Interrupt Priority bit
	
	PIE1bits.TXIE = 0;			// USART Transmit Interrupt disabled bit
	IPR1bits.TXIP = 0;			// USART Transmit Interrupt Priority bit

	ADCON1 = 0x07;				// Minden digit�lis
	ADCON0 = 0;

	CCP1CON = 0;				// Coper�torok kikapcsolva
	
	SN_OE = 0;					// Kimenet SN enged�lyez�se
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
	SPI_SO_TRIS = 0;		//B0 l�b SDI bemenet
	SPI_SCK_TRIS = 0;		//B1 l�b SCK kimenet
	SPI_SI = 1;

	SSPSTATbits.SMP = 0;	//Mintav�telez�s id�z�t�se Master m�d eset�n 1: mintav�telez�s a kimeneti id� v�g�n, 0: mintav�telez�s a kimeneti id� k�zep�n). Slave m�dban k�telez�en null�ba kell �ll�tani.
	SSPSTATbits.CKE = 1;	//�rajel f�zis�nak be�ll�t�sa (1: az adatkimenet be�ll�t�sa akkor t�rt�nik,amikor az �rajel akt�v �llapotb�l t�tlenre v�lt, 0: az adatkimenet be�ll�t�sa akkor t�rt�nik, amikor az �rajel t�tlen �llapotb�l akt�v szintre v�lt)
	SSPSTATbits.BF = 0;		//A buffer foglalts�g�t jelz� �llapotbit  (1: az adat�tvitel lezajlott, SSPBUF kiolvashat�, 0: az adatbuffer �res)
	
	SSPCON1bits.WCOL = 	0;	//�tk�z�s detekt�l�s �r�sn�l (1: �r�st/adatk�ld�st k�s�relt�nk meg, amikor nem lehetett, 0: nincs �tk�z�s)
	SSPCON1bits.SSPOV = 0;	//V�teli t�lcsordul�s-jelz� (1: �jabb b�jt �rkezett, de SSPBUF meg nincs kiolvasva, 0: nincs t�lfoly�s)
	SSPCON1bits.SSPEN = 1;	//Master Szinkron Soros Port enged�lyez�se (1: enged�lyezve, 0: letiltva)
	SSPCON1bits.CKP = 	0;	//�rajel polarit�sa (1: akt�v �llapotban alacsony szint, 0: akt�v �llapotban magas szint)
							//Master Szinkron Soros Port �zemm�dj�nak be�ll�t�sa:
							//0101 = SPI Slave m�d, az SS l�b haszn�lata n�lk�l
							//0100 = SPI Slave m�d, amelyn�l az SS l�b is haszn�latban van
							//0011 = SPI Master m�d, Timer2/2 �rajel frekvenci�val
							//0010 = SPI Master m�d, Fosc/64 �rajel frekvenci�val
							//0001 = SPI Master m�d, Fosc/16 �rajel frekvenci�val
							//0000 = SPI Master m�d, Fosc/4 �rajel frekvenci�val
	SSPCON1bits.SSPM0 = 0;
	SSPCON1bits.SSPM1 = 0;
	SSPCON1bits.SSPM2 = 0;
	SSPCON1bits.SSPM3 = 0;
	LowInterruptIniTimer2();
}

// Id�m�r�shez l�trehozott megszak�t�s
void HighIntTimer0(void)
{
	// Sz�mol�shoz, 8bit-es m�d
	// (FOSC/4) / Prescaler / TMR0H TMR0L
    // (40M/4) / 	2		/ 1
	INTCON2bits.TMR0IP = 1;		// Timer0 megszak�t�s magas priorit�s� legyen
	
	T0CONbits.TMR0ON = 0;		// Timer0 kikapcsolva
	T0CONbits.T08BIT = 1;		// 8 bites m�d kiv�laszt�s
	T0CONbits.T0CS = 0;			// Meghajt�s bels� �rajelr�l
	T0CONbits.T0SE = 0;			// K�ls� �rajel fel- vagy lefut� �l�nek v�laszt�sa
	T0CONbits.PSA = 0;			// El�oszt�s bekapcsol�sa
	//El�oszt�si ar�ny be�ll�t�sa (000 = 1:2, 001 = 1:4, 010 = 1:8, 011 = 0:16,
								// 100 = 1:32, 101 = 1:64, 110 = 1:128, 111 = 1:256)
	T0CONbits.T0PS2 = 0;		// El�oszt�s 1:2 oszt�sra �ll�t�sa
	T0CONbits.T0PS1 = 0;
	T0CONbits.T0PS0 = 0;
	INTCONbits.TMR0IE = 1;			// Timer tilt�sa
	//TMR0H = 0xCD;
	// 0x01 107.4uS/2 = 53.7uS - Szk�ppal m�rve
	// 0xFF 7.8uS/2 = 3.9uS - Szk�ppal m�rve
	TMR0L = 0xFF;
}

//Alacsony pior�t�s� interrupt ind�t�sa
void LowInterruptIniTimer2(void){
	/*********************************************
	+		TIMER2 & INTERRUPT INIT
	+	Timer2 low interrupt
	**********************************************/
    // Timer2
    // (FOSC/4) / Prescaler / PR2 / Postscaler
    // (40M/4) / 	1		/ 112 / 1
    // Actual Interrupt Time : 11,2 uS 
    // |-|_| egy teljes hull�m 22.6 uS, 
	T2CONbits.TOUTPS0 = 0;
	T2CONbits.TOUTPS1 = 0;
	T2CONbits.TOUTPS2 = 0;
	T2CONbits.TOUTPS3 = 0;
	T2CONbits.T2CKPS0 = 0;
	T2CONbits.T2CKPS1 = 0;
	PR2 = 226;				// 44100 Hz (226)
	IPR1bits.TMR2IP = 0;	// Alacsony megszak�t�s enged�lyez�se
	PIR1bits.TMR2IF = 0;
	PIE1bits.TMR2IE = 1;	// TMR2 enged�lyez�, 0 = Enabled
	T2CONbits.TMR2ON = 0;
}

unsigned char SPI(unsigned char adat){
	unsigned char tmp;  
	SSPSTATbits.BF = 0;			// t�rli foglalts�g jelz� �lapot�t a st�tusz regiszterben
	SSPCON1bits.WCOL = 0;		// t�rli az esetleges �r�s �tk�z�s hibajelz�t
	tmp = SSPBUF;				// t�rli a BF jelz�bitet
	SSPBUF = adat;				// kirakja a kimen� adatot az SSPBUF regiszterbe
	while(!SSPSTATbits.BF);		// megv�rjuk a busz ciklus v�g�t 
	return (SSPBUF);			// a vett b�jttal t�r�nk vissza
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
		SPI(CMD_RDSR);                       //St�tuszregiszter olvas�sa parancs
		flag = SPI(0x00);                    //Olvasunk, de valamit akkor is k�ldeni kell!
		MEM_E = 1;                   		 //megsz�ntetj�k a Chip Enable jelet
	} while (flag & 0x01); 
	*/
	unsigned char tmp;
	do {
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
}

// Lefut�s fuggv�nyen bel�l: 8.4uS/2 = 4.2uS + 5*4.2uS = 29.4uS/2 = 14.7uS
// Fuggv�ny h�v�sa + 4.8uS/2 = 2.4uS. �sszesen: 34.2uS/2 = 17.2uS
// F�ggv�nyb�l minden sallangot kivettem ami nem kell
// Csak a legsz�ks�gesebb r�szek maradtak
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
	WaitForMem();                         //V�runk, ha az EEPROM elfoglalt
	MEM_E = 0;
	SPI(CMD_WREN);                       //�r�s �jraenged�lyez�se
	MEM_E = 1;
	Delay_Ms(1);
	MEM_E = 0;
	SPI(CMD_WRITE);                        //Adatblokk �r�sa
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
		WaitForMem();                         //V�runk, ha az EEPROM elfoglalt
		MEM_E = 0;
		SPI(CMD_FREAD);                         //Adatblokk olvas�sa
		SPI(MemCim.low);
		SPI(MemCim.high);
		SPI(MemCim.hh);
		for (i=0; i < 64; i++) {				// Maximum 64byte (W25Q64)
			*pbuf++=SPI(0x00);                   //Olvasunk, de valamit akkor is k�ldeni kell!
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
	//Baud rate kisz�mol�sa (pl:9600 baud rate)
	/*((FOSC/Desired Baud Rate)/64) � 1
	= ((40000000/9600)/64) � 1
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
	// Nyers adat lek�rdez�s			// 29.4uS/2 = 14.7uS alatt olvas 1 byte-ot
	
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
			return wait/Oszto;
		case 0x62:
			return 735.0/Oszto;
		case 0x63:
			return 882.0/Oszto;
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
				// PCM adat kezel�se
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


// PIC18F452 program mem�ri�j�nak t�rl�se 64byte / block
void PICFlashBlockErase(unsigned int Addr)
{
	TBLPTR=Addr;
	//----------------- Blokk t�rl�s ----------------------------------
	EECON1bits.EEPGD=1;			// Flash Program vagy adat EEPROM mem�ria v�laszt� bit
								// 1 = Hozz�f�r�s a flash program mem�ri�hoz
								// 0 = Hozz�f�r�s az EEPROM mem�ri�hoz
	EECON1bits.CFGS=0;			// Flash Program / adat EEPROM vagy konfigur�ci� v�laszt� bit
								// 1 = Hozz�f�r�s a konfigur�ci�s nyilv�ntart�sokhoz
								// 0 = Hozz�f�r�s a Flash program vagy adat EEPROM mem�ri�hoz
	EECON1bits.WREN=1;			// Flash Program / adat EEPROM �r�st enged�lyez� bit
								// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
								// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
	EECON1bits.FREE=1;			// flash t�rl�s�nek enged�lyez�se
								// 1 = T�rl�s enged�lyez�se
								// 0 = Csak olvas�s
	INTCONbits.GIE=0;			// Megszak�t�s letiltva
	EECON2=0x55;				// EEPROM nem fizikai adat regiszter, �r�s �s t�rl�si szekvencia
	EECON2=0xAA;
	EECON1bits.WR=1;			// �r�st vez�rl� bit
								// 1 = kezdem�nyez adat EEPROM t�rl�s / �r�s ciklust vagy a program mem�ria t�rl�si ciklust vagy �r�si ciklus
								// 0 = Write ciklus az EEPROM teljes
	while(EECON1bits.WR);		// V�rakozunk az �r�s befejez�s�ig
	Nop();						// 1 �rajel sz�net
	INTCONbits.GIE=1;			// Megszak�t�s enged�lyezve
	EECON1bits.WREN=0;			// Flash Program / adat EEPROM �r�st enged�lyez� bit
								// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
								// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
}

// PIC18F452 program mem�ria �r�sa 64byte / block �s 8 byte / �r�s
void PICFlashBlockWrite(unsigned int Addr, unsigned char *Data)
{
	unsigned char i = 0;
	unsigned int K = 0;
	TBLPTR=Addr;
	EECON1bits.WREN=0;				// Flash Program / adat EEPROM �r�st enged�lyez� bit
									// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
									// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
	for(K=0;K<BufferArraySize;K=K+PMWbyte)     
	{
		for(i=0;i<PMWbyte;i++)
		{
		   TABLAT = Data[K+i];
		   _asm TBLWTPOSTINC _endasm;
		}
		_asm TBLRDPOSTDEC _endasm;
		EECON1bits.EEPGD=1;			// Flash Program vagy adat EEPROM mem�ria v�laszt� bit
									// 1 = Hozz�f�r�s a flash program mem�ri�hoz
									// 0 = Hozz�f�r�s az EEPROM mem�ri�hoz
		EECON1bits.CFGS=0;			// Flash Program / adat EEPROM vagy konfigur�ci� v�laszt� bit
									// 1 = Hozz�f�r�s a konfigur�ci�s nyilv�ntart�sokhoz
									// 0 = Hozz�f�r�s a Flash program vagy adat EEPROM mem�ri�hoz
		EECON1bits.WREN=1;			// Flash Program / adat EEPROM �r�st enged�lyez� bit
									// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
									// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
		INTCONbits.GIE=0;			// Megszak�t�s letiltva
		EECON2=0x55;				// EEPROM nem fizikai adat regiszter, �r�s �s t�rl�si szekvencia
		EECON2=0xAA;
		EECON1bits.WR=1;			// �r�st vez�rl� bit
									// 1 = kezdem�nyez adat EEPROM t�rl�s / �r�s ciklust vagy a program mem�ria t�rl�si ciklust vagy �r�si ciklus
									// 0 = Write ciklus az EEPROM teljes
		while(EECON1bits.WR);		// V�rakozunk az �r�s befejez�s�ig
		Nop();						// 1 �rajel sz�net
		EECON1bits.WREN=0;			// Flash Program / adat EEPROM �r�st enged�lyez� bit
									// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
									// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
		INTCONbits.GIE=1;			// Megszak�t�s enged�lyez�se
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
	osztas = Temp/oszto;	// 64Kbyte-al t�rt�n� oszt�s eg�sz r�sze
	maradek = Temp%oszto;	// Oszt�s ut�ni marad�k, ha van	
	if(!Select)
		return osztas;
	else
		return maradek;
}	

/*********************************************
+		Eeprom olvas�sa
**********************************************/
unsigned char EepromRead (void)
{
	unsigned char buffer;
	EEADR = 0;				// Mem�ria c�mt�l kezdi az olvas�st
	EECON1bits.EEPGD = 0;	// Flash Program vagy adat EEPROM mem�ria v�laszt� bit
							// 1 = Hozz�f�r�s a flash program mem�ri�hoz
							// 0 = Hozz�f�r�s az EEPROM mem�ri�hoz
	EECON1bits.CFGS = 0;	// Flash Program / adat EEPROM vagy konfigur�ci� v�laszt� bit
							// 1 = Hozz�f�r�s a konfigur�ci�s nyilv�ntart�sokhoz
							// 0 = Hozz�f�r�s a Flash program vagy adat EEPROM mem�ri�hoz	
	EECON1bits.RD = 1;		// Olvas�st vez�rl� bit
							// 1 = kezdem�nyezi az EEPROM olvas�s�t (RD bitet nem lehet be�ll�tani , ha EEPGD = 1 vagy CFGS = 1 )
							// 0 = Nem kezdem�nyez EEPROM olvas�s
	buffer = EEDATA;		// Kiolvassuk az adatot
	return buffer;
}

/*********************************************
+		Eeprom �r�sa
**********************************************/
void EepromWrite (unsigned char adat)
{
	EEADR = 0;				// Mem�ria c�mt�l kezdj�k az �r�st
	EEDATA = adat;
	EECON1bits.EEPGD = 0;	// Flash Program vagy adat EEPROM mem�ria v�laszt� bit
							// 1 = Hozz�f�r�s a flash program mem�ri�hoz
							// 0 = Hozz�f�r�s az EEPROM mem�ri�hoz
	EECON1bits.CFGS = 0;	// Flash Program / adat EEPROM vagy konfigur�ci� v�laszt� bit
							// 1 = Hozz�f�r�s a konfigur�ci�s nyilv�ntart�sokhoz
							// 0 = Hozz�f�r�s a Flash program vagy adat EEPROM mem�ri�hoz
	EECON1bits.WREN = 1;	// Flash Program / adat EEPROM �r�st enged�lyez� bit
							// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
							// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
	INTCONbits.GIE = 0;		//Tiltjuk a interrupt-ot (megszak�t�st)
	EECON2 = 0x55;
	EECON2 = 0xAA;
	EECON1bits.WR = 1;		// �r�st vez�rl� bit
							// 1 = kezdem�nyez adat EEPROM t�rl�s / �r�s ciklust vagy a program mem�ria t�rl�si ciklust vagy �r�si ciklus
							// 0 = Write ciklus az EEPROM teljes
	while (EECON1bits.WR == 1);	//V�rakozunk az �r�s befejez�s�ig
	INTCONbits.GIE = 1;		// enged�lyezz�k az interrupt-ot (megszak�t�st)
	EECON1bits.WREN = 0;	// Flash Program / adat EEPROM �r�st enged�lyez� bit
							// 1 = Enged�lyezi a Flash program / adat EEPROM �r�s�t
							// 0 = G�tolja a Flash program / adat EEPROM �r�s�t
}
