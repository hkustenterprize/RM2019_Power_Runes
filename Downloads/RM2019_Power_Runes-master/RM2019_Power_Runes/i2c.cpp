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
    palSetLineMode(LINE_B,PAL_MODE_OUTPUT_OPENDRAIN);//PF0 I2 SDA ---->LINE_B
    palSetLineMode(LINE_A,PAL_MODE_OUTPUT_OPENDRAIN);//PF1 I1 SCL ---->LINE_A
  palSetLine(LINE_B);
  palSetLine(LINE_A);
  uint8_t i; for (i = 0; i < 100; i++)
    __delay();
}

void swi2c_start(void)
{
  palClearLine(LINE_B);
  __delay();
  palClearLine(LINE_A);
  __delay();
}

void swi2c_stop(void)
{
  palSetLine(LINE_A);
  __delay();
  palSetLine(LINE_B);
  __delay();
}

void swi2c_ack(void)
{
  palClearLine(LINE_B);
  __delay();
  palSetLine(LINE_A);
  __delay();
  palClearLine(LINE_A);
  __delay();
}

void swi2c_nack(void)
{
  palSetLine(LINE_B);
  __delay();
  palSetLine(LINE_A);
  __delay();
  __delay();
  palClearLine(LINE_A);
  __delay();
  palClearLine(LINE_B);
  __delay();
}

uint8_t swi2c_wait_ack()
{
  palSetLineMode(LINE_B,PAL_MODE_INPUT);
  __delay();
//  palSetLine(LINE_B);
  __delay();
  palSetLine(LINE_A);
//  __delay();
  uint8_t ack = 0;
  uint16_t ackto = SWI2C_TMO;
  while (!(ack = ((palReadLine(LINE_B)==PAL_HIGH)?0:1)) && ackto--) __delay();
  palClearLine(LINE_A);
  __delay();
  palSetLineMode(LINE_B,PAL_MODE_OUTPUT_OPENDRAIN);
  __delay();
  palClearLine(LINE_B);
  __delay();
  return ack;
}

uint8_t swi2c_read(void)
{
  palSetLine(LINE_B);
  //__delay();
  palSetLineMode(LINE_B,PAL_MODE_INPUT);
  uint8_t data = 0;
  int8_t bit; for (bit = 7; bit >= 0; bit--)
  {
    palClearLine(LINE_A);
    __delay();
    palSetLine(LINE_A);
    __delay();
    data |= ((palReadLine(LINE_B)==PAL_HIGH)?1:0) << bit;
    __delay();
    palClearLine(LINE_A);
    __delay();
  }
  palSetLineMode(LINE_B,PAL_MODE_OUTPUT_OPENDRAIN);
  return data;
}

void swi2c_write(uint8_t data)
{
  int8_t bit; for (bit = 7; bit >= 0; bit--)
  {
    if (data & (1 << bit)) palSetLine(LINE_B);
    else palClearLine(LINE_B);
    __delay();
    palSetLine(LINE_A);
    __delay();
    __delay();
    palClearLine(LINE_A);
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
//  PAT9125_ORIENTATION, 0x04 | (xinv?0x08:0) | (yinv?0x10:0), //!? direction switching does not work
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



/*!
 *  @brief  Instantiates a new PCA9685 PWM driver chip with the I2C address on a
 * TwoWire interface
 *  @param  i2c  A pointer to a 'Wire' compatible object that we'll use to
 * communicate with
 *  @param  addr The 7-bit I2C address to locate this chip, default is 0x40
 */
// Adafruit_PWMServoDriver::Adafruit_PWMServoDriver(TwoWire *i2c, uint8_t addr) {
//   _i2c = i2c;
//   _i2caddr = addr;
// }

void delay(uint8_t second)
{
  chThdSleepMicroseconds(second);
}

/*!
 *  @brief  Setups the I2C interface and hardware
 *  @param  prescale
 *          Sets External Clock (Optional)
 *
 */
void Adafruit_PWMServoDriver::begin(uint8_t prescale) {
//   _i2c->begin();
  reset();
  if (prescale) {
    setExtClk(prescale);
  } else {
    // set a default frequency
    setPWMFreq(1000);
  }
}

/*!
 *  @brief  Sends a reset command to the PCA9685 chip over I2C
 */
void Adafruit_PWMServoDriver::reset() {
  write8(PCA9685_MODE1, 0x80);
  delay(10);
}

/*!
 *  @brief  Puts board into sleep mode
 */
void Adafruit_PWMServoDriver::sleep() {
  uint8_t awake = read8(PCA9685_MODE1);
  uint8_t sleep = awake | 0x10; // set sleep bit high
  write8(PCA9685_MODE1, sleep);
  delay(5); // wait until cycle ends for sleep to be active
}

/*!
 *  @brief  Wakes board from sleep
 */
void Adafruit_PWMServoDriver::wakeup() {
  uint8_t sleep = read8(PCA9685_MODE1);
  uint8_t wakeup = sleep & ~0x10; // set sleep bit low
  write8(PCA9685_MODE1, wakeup);
}

/**************************************************************************/
/*!
    @brief  Sets EXTCLK pin to use the external clock
       @param  prescale Configures the prescale value to be used by the external
   clock
*/
/**************************************************************************/
void Adafruit_PWMServoDriver::setExtClk(uint8_t prescale) {
  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode); // go to sleep, turn off internal oscillator

  // This sets both the SLEEP and EXTCLK bits of the MODE1 register to switch to
  // use the external clock.
  write8(PCA9685_MODE1, (newmode |= 0x40));

  write8(PCA9685_PRESCALE, prescale); // set the prescaler

  delay(5);
  write8(PCA9685_MODE1,
         (newmode & ~(0x10)) | 0xa0); // clear the SLEEP bit to start

#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Mode now 0x");
  Serial.println(read8(PCA9685_MODE1), HEX);
#endif
}

/*!
 *  @brief  Sets the PWM frequency for the entire chip, up to ~1.6 KHz
 *  @param  freq Floating point frequency that we will attempt to match
 */
void Adafruit_PWMServoDriver::setPWMFreq(float freq) {
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Attempting to set freq ");
  Serial.println(freq);
#endif

  freq *=
      0.9; // Correct for overshoot in the frequency setting (see issue #11).
  float prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;

#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Estimated pre-scale: ");
  Serial.println(prescaleval);
#endif

  uint8_t prescale = floor(prescaleval + 0.5);
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Final pre-scale: ");
  Serial.println(prescale);
#endif

  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode);            // go to sleep
  write8(PCA9685_PRESCALE, prescale);        // set the prescaler
  write8(PCA9685_MODE1, oldmode);
  delay(5);
  write8(PCA9685_MODE1,
         oldmode |
             0xa0); //  This sets the MODE1 register to turn on auto increment.

#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Mode now 0x");
  Serial.println(read8(PCA9685_MODE1), HEX);
#endif
}

/*!
 *  @brief  Gets the PWM output of one of the PCA9685 pins
 *  @param  num One of the PWM output pins, from 0 to 15
 *  @return requested PWM output value
 */
uint8_t Adafruit_PWMServoDriver::getPWM(uint8_t num) {
  _i2c->requestFrom((int)_i2caddr, LED0_ON_L + 4 * num, (int)4);
  return _i2c->read();
}

/*!
 *  @brief  Sets the PWM output of one of the PCA9685 pins
 *  @param  num One of the PWM output pins, from 0 to 15
 *  @param  on At what point in the 4096-part cycle to turn the PWM output ON
 *  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
 */
void Adafruit_PWMServoDriver::setPWM(uint8_t num, uint16_t on, uint16_t off) {
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Setting PWM ");
  Serial.print(num);
  Serial.print(": ");
  Serial.print(on);
  Serial.print("->");
  Serial.println(off);
#endif

  _i2c->beginTransmission(_i2caddr);
  _i2c->write(LED0_ON_L + 4 * num);
  _i2c->write(on);
  _i2c->write(on >> 8);
  _i2c->write(off);
  _i2c->write(off >> 8);
  _i2c->endTransmission();
}

/*!
 *   @brief  Helper to set pin PWM output. Sets pin without having to deal with
 * on/off tick placement and properly handles a zero value as completely off and
 * 4095 as completely on.  Optional invert parameter supports inverting the
 * pulse for sinking to ground.
 *   @param  num One of the PWM output pins, from 0 to 15
 *   @param  val The number of ticks out of 4096 to be active, should be a value
 * from 0 to 4095 inclusive.
 *   @param  invert If true, inverts the output, defaults to 'false'
 */
void Adafruit_PWMServoDriver::setPin(uint8_t num, uint16_t val, bool invert) {
  // Clamp value between 0 and 4095 inclusive.
  val = min(val, (uint16_t)4095);
  if (invert) {
    if (val == 0) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    } else if (val == 4095) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    } else {
      setPWM(num, 0, 4095 - val);
    }
  } else {
    if (val == 4095) {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    } else if (val == 0) {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    } else {
      setPWM(num, 0, val);
    }
  }
}

uint8_t Adafruit_PWMServoDriver::read8(uint8_t addr) {
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(addr);
  _i2c->endTransmission();

  _i2c->requestFrom((uint8_t)_i2caddr, (uint8_t)1);
  return _i2c->read();
}

void Adafruit_PWMServoDriver::write8(uint8_t addr, uint8_t d) {
  _i2c->beginTransmission(_i2caddr);
  _i2c->write(addr);
  _i2c->write(d);
  _i2c->endTransmission();
}


static THD_WORKING_AREA(PAT9125ELThd_wa, 1024);
static THD_FUNCTION(PAT9125ELThd, p)
{

    (void)p;

    while (!chThdShouldTerminateX())
    {
    //PAT9125init();
    //swi2c_readByte_A8(PAT9125_I2C_ADDR,0x03,&mydata);
    
    palSetLine(LINE_C);
    palSetLine(LINE_F);
	pat9125_wr_reg(0x02,0xfb);
	// pat9125_b = pat9125_rd_reg(0);
    // uint8_t ucMotion = pat9125_rd_reg(PAT9125_MOTION);
    // pat9125_b = pat9125_rd_reg(PAT9125_FRAME);
    // pat9125_s = pat9125_rd_reg(PAT9125_SHUTTER);
    // if (ucMotion & 0x80)
    // {

    //   uint8_t ucXL = pat9125_rd_reg(PAT9125_DELTA_XL);
    //   uint8_t ucYL = pat9125_rd_reg(PAT9125_DELTA_YL);
    //   uint8_t ucXYH = pat9125_rd_reg(PAT9125_DELTA_XYH);
    //   int16_t iDX = ucXL | ((ucXYH << 4) & 0xf00);
    //   int16_t iDY = ucYL | ((ucXYH << 8) & 0xf00);
    //   if (iDX & 0x800) iDX -= 4096;
    //   if (iDY & 0x800) iDY -= 4096;
    //   position -= iDY;
    //   pat9125_x += iDX;
    //   pat9125_y -= iDY; //negative number, because direction switching does not work
    // }
    //else  Param_DebugHalt(&errormsg);
    
    chThdSleepMilliseconds(1);
    }
}


void myi2cstart(){
  
    //palSetLineMode(LINE_K1, PAL_MODE_OUTPUT_PUSHPULL);
    //palSetLineMode(LINE_K2, PAL_MODE_ALTERNATE(5));
    palSetLineMode(LINE_C, PAL_MODE_OUTPUT_PUSHPULL);//for 3.3V
    palSetLineMode(LINE_F, PAL_MODE_OUTPUT_PUSHPULL);//for 3.3V
    //palSetLineMode(LINE_A, PAL_MODE_ALTERNATE(4));
    //palSetLineMode(LINE_B, PAL_MODE_ALTERNATE(4));
    palSetLineMode(LINE_A, PAL_MODE_OUTPUT_OPENDRAIN);
    palSetLineMode(LINE_B, PAL_MODE_OUTPUT_PUSHPULL);
    palSetLine(LINE_A);
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
/*
want to use PAT9125CL,
1 (VLD) to LINE_C
3 (VDD) to LINE_F
4 (GND) to GND
5 (SCL) to LINE_A 
6 (SDA) to LINE_B
*/