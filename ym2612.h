void SN76489_SendByte(unsigned char byte);
void OSC_Init(void);
void PSG_Silence(void);
void YM2612_Send(unsigned char addr, unsigned char data, unsigned char CMD);

void SN76489_SendByte(unsigned char byte)
{
	SN_PORT = byte;
	SN_WE = 0;
	Delay_Us(1);	//~12Us, felbontás (~52Us (szkóppal ellenõrizve))
	SN_WE = 1;
}
void PSG_Silence(void)
{
	SN76489_SendByte(0b10011111);	// 0 csatonra Hangerõ néma (0x9F)
	SN76489_SendByte(0b10111111);	// 1 csatonra Hangerõ néma (0xBF)
	SN76489_SendByte(0b11011111);	// 2 csatonra Hangerõ néma (0xDF)
	SN76489_SendByte(0b11111111);	// 3 csatonra Hangerõ néma (0xFF)
	YM_A0 = 0;
	YM_A1 = 0;
	YM_CS = 1;
	YM_WR = 1;
	YM_RD = 1;
	YM_IC = 1;
	Delay_Ms(5);
	YM_IC = 0;
	Delay_Ms(5);
	YM_IC = 1;

}
void YM2612_Send(unsigned char addr, unsigned char data, unsigned char CMD) //0x52 = A1 LOW, 0x53 = A1 HIGH
{
	YM_A1 = 0;
    if(CMD==0x53){
		YM_A1 = 1;					// A1 HIGH
	}
    YM_A0 = 0; 						// A0 LOW
    YM_CS = 0;						// CS LOW
    SN_PORT = addr;
    YM_WR = 0;						// WR LOW
	Delay_Us(0);
    YM_WR = 1;						// WR HIGH
    YM_CS = 1;						// CS HIGH
    YM_A0 = 1;						// A0 HIGH
    YM_CS = 0;						// CS LOW
    SN_PORT = data;
    YM_WR = 0;						// WR LOW
	Delay_Us(0);
    YM_WR = 1;						// WR HIGH
    YM_CS = 1;						// CS HIGH
}

