void lo_isr(void);        //el�re deklar�ljuk a kiszolg�l�
void hi_isr(void);        //elj�r�sokat

/** Magas �s alacsony priorit�s� interrupt vektor
    Magas = 0x0008, Alacsony = 0x0018.
*/
#pragma code high_vector_addr=0x08
void high_vector(void) {
  _asm GOTO hi_isr _endasm
}
#pragma code low_vector_addr=0x18
void low_vector(void) {
  _asm GOTO lo_isr _endasm
}
#pragma code             //visszat�r�nk az alap�rtelmezett k�dszekci�ba
#pragma interrupt hi_isr
void hi_isr (void) {
	/*if(PIR1bits.RCIF){
		if(RCSTA & 6){	// Hib�s adat �rkezik
			Error = 1;
			RCSTAbits.SREN = 0;
			temp = RCREG;   // Eldobjuk az adatot
			RCSTAbits.SREN = 1;
			DataSize++;
		}else{ // Ha nincs hiba
			Sample = RCREG;    // Kiolvassuk az adatot �s elt�roljuk
			ResultOK = 1;    // Van adat �gy ezt jelezz�k
			DataSize++;     // Adat forgalom sz�mol�sa
		}
	}*/
	if(PIR1bits.RCIF){
		if(RCSTA & 6){	// Hib�s adat �rkezik
			unsigned char temp;    // T�rol�
			Error++;
			RCSTAbits.SREN = 0;
			temp = RCREG;   // Eldobjuk az adatot
			RCSTAbits.SREN = 1;
			BufferSize++;
		}else{ // Ha nincs hiba
			if(bUffEr){ Buffer[BufferI] = RCREG;    // Kiolvassuk az adatot �s elt�roljuk
			}else{ Buffer2[BufferI] = RCREG; }
			BufferI++;
			DataSize++;     // Adat forgalom sz�mol�sa
			BufferSize++;
		}
		PIR1bits.RCIF = 0;
	}
	
	//Timer0 Interrupt Handler
	// Id�m�r�s tesztel�s�re haszn�lom, 
	// nincs k�ze a f�programhoz
	if (INTCONbits.TMR0IF)	// 107.4uS
	{
		TMR0L = 0xFF;	// 107.4uS / megszak�t�s
		Timer0Count++;
		INTCONbits.TMR0IF = 0;   // clear the flag
	}
}
#pragma interruptlow lo_isr
void lo_isr (void) {
	/* Itt t�rt�nik az alacsony priorit�s� megszak�t�s t�nyleges kiszolg�l�sa */
	if(PIR1bits.TMR2IF)
	{
		unsigned float wait = 0;
		unsigned char Sample = 0;
		unsigned char YM_address = 0;
		unsigned char YM_data = 0;
		unsigned int i = 0, x = 0;
		unsigned char osztas = 0, maradek = 0;
		
		unsigned char temp = 0;
		unsigned char tmp;
	
		if(waitSamples>0)
			waitSamples--;
		
		if(waitSamples==0 && SampleNext == 1)
		{	
			SampleNext=0;
			MemCim.value++;
			//waitSamples = WaveSample();	// 147.8uS/2 = 73.9uS	
			//waitSamples = WaveSampleIf();	// 136.8uS/2 = 68.4uS
			
			////////////////////////////////////////////////
			
			/*MEM_E = 0;
			
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
			*/
			////////////////////////////////////////////////
			
			/******************* if felt�teles verzi� 96.59uS/2 = 48.3uS ******************/
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
				//SN76489_SendByte(MemReadFast(MemCim));
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
				temp = SSPBUF;
				SN76489_SendByte(temp);
				waitSamples = 0;
			}
			else if(Sample==0x50)
			{
				MemCim.value++;
				//SN76489_SendByte(MemReadFast(MemCim));
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
				temp = SSPBUF;
				SN76489_SendByte(temp);
				waitSamples = 0;
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
				waitSamples = 0;
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
				waitSamples = wait/Oszto;
			}	
			else if(Sample==0x62)
			{
				waitSamples = 735.0/Oszto;
			}		
			else if(Sample==0x63)
			{
				waitSamples = 882.0/Oszto;
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
				waitSamples = 0;
			}
			else if(Sample>=0x70 && Sample<=0x7F)
			{	
				wait = (Sample & 0x0F)+1;
				waitSamples = wait/Oszto;
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
				waitSamples = wait/Oszto;
			}	
			else if(Sample==0xE0)
			{
				/*MemCim.value+=4;
				if(PCMLoad)
				{
					BuffIndex = JumpTableE0[PCMJumpIndex++];
				}
				else
				{
					PCMMemAdr.value = pcmBufferPositionStart;
					PCMMemAdr.value += JumpTableE0[PCMJumpIndex++];
				}*/
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
				waitSamples =  0;
			}	
			else if(Sample==0x66)
			{
				MemCim.value=63;
				waitSamples =  0;
			}		
			else
			{
				waitSamples =  0;
			}
			
			/***********************************************************/
			
			SampleNext=1;
		}
		LED2 = !LED2;
		PIR1bits.TMR2IF = 0;
	}
}
