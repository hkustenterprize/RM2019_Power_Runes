#include "i2c.hpp"


#define SWI2C_RMSK   0x01 //read mask (bit0 = 1)
#define SWI2C_WMSK   0x00 //write mask (bit0 = 0)
#define SWI2C_ASHF   0x01 //address shift (<< 1)
#define SWI2C_DMSK   0x7f //device address mask

uint8_t mydata = 2;
uint8_t pat9125_PID1 = 0;
uint8_t pat9125_PID2 = 0;
int16_t pat9125_x = 0;
int16_t pat9125_y = 0;
uint8_t pat9125_b = 0;
uint8_t pat9125_s = 0;
//static volatile uint64_t position = 1;
	char errormsg = 'i';
int64_t position = 0;


uint8_t pat9125_rd_reg(uint8_t addr);
void pat9125_wr_reg(uint8_t addr, uint8_t data);
uint8_t pat9125_wr_reg_verify(uint8_t addr, uint8_t data);


void __delay(void)
{
	chThdSleepMicroseconds(1.5);
}

void swi2c_init(void)
{
    palSetLineMode(LINE_I2,PAL_MODE_OUTPUT_OPENDRAIN);
    palSetLineMode(LINE_I1,PAL_MODE_OUTPUT_OPENDRAIN);
	palSetLine(LINE_I2);
	palSetLine(LINE_I1);
	uint8_t i; for (i = 0; i < 100; i++)
		__delay();
}

void swi2c_start(void)
{
	palClearLine(LINE_I2);
	__delay();
	palClearLine(LINE_I1);
	__delay();
}

void swi2c_stop(void)
{
	palSetLine(LINE_I1);
	__delay();
	palSetLine(LINE_I2);
	__delay();
}

void swi2c_ack(void)
{
	palClearLine(LINE_I2);
	__delay();
	palSetLine(LINE_I1);
	__delay();
	palClearLine(LINE_I1);
	__delay();
}

void swi2c_nack(void)
{
	palSetLine(LINE_I2);
	__delay();
	palSetLine(LINE_I1);
	__delay();
	__delay();
	palClearLine(LINE_I1);
	__delay();
	palClearLine(LINE_I2);
	__delay();
}

uint8_t swi2c_wait_ack()
{
	palSetLineMode(LINE_I2,PAL_MODE_INPUT);
	__delay();
//	palSetLine(LINE_I2);
	__delay();
	palSetLine(LINE_I1);
//	__delay();
	uint8_t ack = 0;
	uint16_t ackto = SWI2C_TMO;
	while (!(ack = ((palReadLine(LINE_I2)==PAL_HIGH)?0:1)) && ackto--) __delay();
	palClearLine(LINE_I1);
	__delay();
	palSetLineMode(LINE_I2,PAL_MODE_OUTPUT_OPENDRAIN);
	__delay();
	palClearLine(LINE_I2);
	__delay();
	return ack;
}

uint8_t swi2c_read(void)
{
	palSetLine(LINE_I2);
	//__delay();
	palSetLineMode(LINE_I2,PAL_MODE_INPUT);
	uint8_t data = 0;
	int8_t bit; for (bit = 7; bit >= 0; bit--)
	{
		palClearLine(LINE_I1);
		__delay();
		palSetLine(LINE_I1);
		__delay();
		data |= ((palReadLine(LINE_I2)==PAL_HIGH)?1:0) << bit;
		__delay();
		palClearLine(LINE_I1);
		__delay();
	}
	palSetLineMode(LINE_I2,PAL_MODE_OUTPUT_OPENDRAIN);
	return data;
}

void swi2c_write(uint8_t data)
{
	int8_t bit; for (bit = 7; bit >= 0; bit--)
	{
		if (data & (1 << bit)) palSetLine(LINE_I2);
		else palClearLine(LINE_I2);
		__delay();
		palSetLine(LINE_I1);
		__delay();
		__delay();
		palClearLine(LINE_I1);
		__delay();
	}
}

uint8_t swi2c_check(uint8_t dev_addr)
{
	swi2c_start();
	swi2c_write((dev_addr & SWI2C_DMSK) << SWI2C_ASHF);
	if (!swi2c_wait_ack()) { swi2c_stop(); return 0; }
	swi2c_stop();
	return 1;
}

#ifdef SWI2C_A8 //8bit address

uint8_t swi2c_readByte_A8(uint8_t dev_addr, uint8_t addr, uint8_t* pbyte)
{
	swi2c_start();
	swi2c_write(SWI2C_WMSK | ((dev_addr & SWI2C_DMSK) << SWI2C_ASHF));
	//swi2c_write(SWI2C_WMSK | (dev_addr & SWI2C_DMSK) );
	swi2c_ack();
	//if (!swi2c_wait_ack()) { swi2c_stop(); return 0; }
	swi2c_write(addr & 0xff);
	swi2c_ack();
	//if (!swi2c_wait_ack()) return 0;
	swi2c_stop();
	swi2c_start();
	swi2c_write(SWI2C_RMSK | ((dev_addr & SWI2C_DMSK) << SWI2C_ASHF) );
	swi2c_ack();
	//if (!swi2c_wait_ack()) return 0;
	uint8_t byte = swi2c_read();
	swi2c_nack();
	swi2c_stop();
	if (pbyte) *pbyte = byte;
	return 1;
}

uint8_t swi2c_writeByte_A8(uint8_t dev_addr, uint8_t addr, uint8_t* pbyte)
{
	swi2c_start();
	swi2c_write(SWI2C_WMSK | ((dev_addr & SWI2C_DMSK) << SWI2C_ASHF));
	swi2c_ack();
	//if (!swi2c_wait_ack()) { swi2c_stop(); return 0; }
	swi2c_write(addr & 0xff);
	swi2c_ack();
	//if (!swi2c_wait_ack()) return 0;
	swi2c_write(*pbyte);
	swi2c_ack();
	//if (!swi2c_wait_ack()) return 0;
	swi2c_stop();
	return 1;
}

#endif //SWI2C_A8
// Init sequence, address & value.
uint8_t pat9125_init_seq1[] = {
	// Disable write protect.
	PAT9125_WP, 0x5a,
	// Set the X resolution to zero to let the sensor know that it could safely ignore movement in the X axis.
    // PAT9125_RES_X, 100,
    // Set the Y resolution to a maximum (or nearly a maximum).
    PAT9125_RES_Y, PAT9125_YRES,
    // Set 12-bit X/Y data format.
    PAT9125_ORIENTATION, 0x04,
//	PAT9125_ORIENTATION, 0x04 | (xinv?0x08:0) | (yinv?0x10:0), //!? direction switching does not work
    // Now continues the magic sequence from the PAT912EL Application Note: Firmware Guides for Tracking Optimization.
    0x5e, 0x08,
    0x20, 0x64,
    0x2b, 0x6d,
    0x32, 0x2f,
    // stopper
    0x0ff
};

// Init sequence, address & value.
uint8_t pat9125_init_seq2[] = {
	// Magic sequence to enforce full frame rate of the sensor.
	0x06, 0x028,
	0x33, 0x0d0,
	0x36, 0x0c2,
	0x3e, 0x001,
	0x3f, 0x015,
	0x41, 0x032,
	0x42, 0x03b,
	0x43, 0x0f2,
	0x44, 0x03b,
	0x45, 0x0f2,
	0x46, 0x022,
	0x47, 0x03b,
	0x48, 0x0f2,
	0x49, 0x03b,
	0x4a, 0x0f0,
	0x58, 0x098,
	0x59, 0x00c,
	0x5a, 0x008,
	0x5b, 0x00c,
	0x5c, 0x008,
	0x61, 0x010,
	0x67, 0x09b,
	0x6e, 0x022,
	0x71, 0x007,
	0x72, 0x008,
	// stopper
    0x0ff
};
static void Param_DebugHalt(const char *msg)
{
	//while (1)
	//for(int i = 0;i<10;i++)
	//{
#ifdef SHELL_CHN
		chprintf((BaseSequentialStream *)&SHELL_CHN, msg);
		chprintf((BaseSequentialStream *)&SHELL_CHN, SHELL_NEWLINE_STR);
#endif
		//blink all LEDs
		palTogglePort(GPIOG, 0xFF << 1);
		palToggleLine(LINE_LED_GREEN);
		palToggleLine(LINE_LED_RED);
		chibios_rt::BaseThread::sleep(TIME_MS2I(20));
	//}
}

uint8_t pat9125_rd_reg(uint8_t addr)
{
	uint8_t data = 0;
	swi2c_readByte_A8(PAT9125_I2C_ADDR, addr, &data);
	return data;
}
void pat9125_wr_reg(uint8_t addr, uint8_t data)
{
	swi2c_writeByte_A8(PAT9125_I2C_ADDR, addr, &data);
}
uint8_t PAT9125init(){

	// Switch to bank0, not allowed to perform OTS_RegWriteRead.
	pat9125_wr_reg(PAT9125_BANK_SELECTION, 0);
	// Software reset (i.e. set bit7 to 1). It will reset to 0 automatically.
	// After the reset, OTS_RegWriteRead is not allowed.
	pat9125_wr_reg(PAT9125_CONFIG, 0x97);
	// Wait until the sensor reboots.
	// Delay 10ms.
	chThdSleepMilliseconds(1);
	{
		int i = 0;
		//const uint8_t ptr = pat9125_init_seq1[i];
		for (;;) {
			const uint8_t addr = pat9125_init_seq1[i++];
			if (addr == 0x0ff)
				break;
			if (! pat9125_wr_reg_verify(addr, pat9125_init_seq1[i++]))
				// Verification of the register write failed.
				return 0;
		}
	}
	chThdSleepMilliseconds(10);
	// Switch to bank1, not allowed to perform OTS_RegWrite.
	pat9125_wr_reg(PAT9125_BANK_SELECTION, 0x01);
	{
		int i = 0;
		//const uint8_t addr = pat9125_init_seq2;
		for (;;) {
			const uint8_t addr = pat9125_init_seq2[i++];
			if (addr == 0x0ff)
				break;
			if (! pat9125_wr_reg_verify(addr, pat9125_init_seq2[i++]))
				// Verification of the register write failed.
				return 0;
		}
	}
	// Switch to bank0, not allowed to perform OTS_RegWriteRead.
	pat9125_wr_reg(PAT9125_BANK_SELECTION, 0x00);
	// Enable write protect.
	pat9125_wr_reg(PAT9125_WP, 0x00);

	pat9125_wr_reg(PAT9125_RES_X, PAT9125_XRES);
	pat9125_wr_reg(PAT9125_RES_Y, PAT9125_YRES);
	return 1;
}

uint8_t pat9125_update_y(void)
{
	
		uint8_t ucMotion = pat9125_rd_reg(PAT9125_MOTION);
		//if (pat9125_PID1 == 0xff) return 0;
		if (ucMotion & 0x80)
		{
			uint8_t ucYL = pat9125_rd_reg(PAT9125_DELTA_YL);
			uint8_t ucXYH = pat9125_rd_reg(PAT9125_DELTA_XYH);
			//if (pat9125_PID1 == 0xff) return 0;
			int16_t iDY = ucYL | ((ucXYH << 8) & 0xf00);
			if (iDY & 0x800) iDY -= 4096;
			pat9125_y -= iDY; //negative number, because direction switching does not work
		}
		return 1;
	
	
}

uint8_t pat9125_update_y2(void)
{
		uint8_t ucMotion = pat9125_rd_reg(PAT9125_MOTION);
		//if (pat9125_PID1 == 0xff) return 0; //NOACK error
		if (ucMotion & 0x80)
		{
			int8_t dy = pat9125_rd_reg(PAT9125_DELTA_YL);
			//if (pat9125_PID1 == 0xff) return 0; //NOACK error
			pat9125_y -= dy; //negative number, because direction switching does not work
		}
		return 1;
	
}

static THD_WORKING_AREA(PAT9125ELThd_wa, 1024);
static THD_FUNCTION(PAT9125ELThd, p)
{

    (void)p;

    while (!chThdShouldTerminateX())
    {
		//PAT9125init();
		//swi2c_readByte_A8(PAT9125_I2C_ADDR,0x03,&mydata);
		
    palSetLine(LINE_J1);
    palSetLine(LINE_J2);
	
		uint8_t ucMotion = pat9125_rd_reg(PAT9125_MOTION);
		pat9125_b = pat9125_rd_reg(PAT9125_FRAME);
		pat9125_s = pat9125_rd_reg(PAT9125_SHUTTER);
		if (ucMotion & 0x80)
		{

			uint8_t ucXL = pat9125_rd_reg(PAT9125_DELTA_XL);
			uint8_t ucYL = pat9125_rd_reg(PAT9125_DELTA_YL);
			uint8_t ucXYH = pat9125_rd_reg(PAT9125_DELTA_XYH);
			int16_t iDX = ucXL | ((ucXYH << 4) & 0xf00);
			int16_t iDY = ucYL | ((ucXYH << 8) & 0xf00);
			if (iDX & 0x800) iDX -= 4096;
			if (iDY & 0x800) iDY -= 4096;
			position -= iDY;
			pat9125_x += iDX;
			pat9125_y -= iDY; //negative number, because direction switching does not work
		}
		//else	Param_DebugHalt(&errormsg);
		
		chThdSleepMilliseconds(1);
    }
}


void myi2cstart(){
	
    //palSetLineMode(LINE_K1, PAL_MODE_OUTPUT_PUSHPULL);
    //palSetLineMode(LINE_K2, PAL_MODE_ALTERNATE(5));
    palSetLineMode(LINE_J1, PAL_MODE_OUTPUT_PUSHPULL);
    palSetLineMode(LINE_J2, PAL_MODE_OUTPUT_PUSHPULL);
    //palSetLineMode(LINE_I1, PAL_MODE_ALTERNATE(4));
    //palSetLineMode(LINE_I2, PAL_MODE_ALTERNATE(4));
    palSetLineMode(LINE_I1, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetLineMode(LINE_I2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetLine(LINE_I1);
	position = 1;

	PAT9125init();
    chThdCreateStatic(PAT9125ELThd_wa, sizeof(PAT9125ELThd_wa),
                      NORMALPRIO + 9, PAT9125ELThd, NULL);
}

uint8_t pat9125_wr_reg_verify(uint8_t addr, uint8_t data)
{
	pat9125_wr_reg(addr, data);
	return pat9125_rd_reg(addr) == data;
}