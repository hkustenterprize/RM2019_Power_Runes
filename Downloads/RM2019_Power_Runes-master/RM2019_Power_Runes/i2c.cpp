#include "i2c.hpp"

/*
  Belows are software i2c, developed by Tong Cheng, @ztangaj
*/

#define SWI2C_RMSK 0x01 //read mask (bit0 = 1)
#define SWI2C_WMSK 0x00 //write mask (bit0 = 0)
#define SWI2C_ASHF 0x01 //address shift (<< 1)
#define SWI2C_DMSK 0x7f //device address mask

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
  chThdSleepMicroseconds(50);
}

void swi2c_init(void)
{
  palSetLineMode(LINE_B, PAL_MODE_OUTPUT_OPENDRAIN); //PF0 I2 SDA ---->LINE_B
  palSetLineMode(LINE_A, PAL_MODE_OUTPUT_OPENDRAIN); //PF1 I1 SCL ---->LINE_A
  palSetLine(LINE_B);
  palSetLine(LINE_A);
  uint8_t i;
  for (i = 0; i < 100; i++)
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
  palSetLine(LINE_B);
  __delay();
  palSetLine(LINE_A);
  __delay();
  palClearLine(LINE_A);
  // __delay();
  palSetLine(LINE_B);
  palSetLineMode(LINE_B, PAL_MODE_OUTPUT_OPENDRAIN);
  __delay();
  palSetLineMode(LINE_B, PAL_MODE_OUTPUT_OPENDRAIN);
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
  palSetLineMode(LINE_B, PAL_MODE_INPUT);
  __delay();
  //  palSetLine(LINE_B);
  __delay();
  palSetLine(LINE_A);
  //  __delay();
  uint8_t ack = 0;
  uint16_t ackto = SWI2C_TMO;
  while (!(ack = ((palReadLine(LINE_B) == PAL_HIGH) ? 0 : 1)) && ackto--)
    __delay();
  palClearLine(LINE_A);
  __delay();
  palSetLineMode(LINE_B, PAL_MODE_OUTPUT_OPENDRAIN);
  __delay();
  palClearLine(LINE_B);
  __delay();
  return ack;
}

uint8_t swi2c_read(void)
{
  palSetLine(LINE_B);
  //__delay();
  palSetLineMode(LINE_B, PAL_MODE_INPUT);
  uint8_t data = 0;
  int8_t bit;
  for (bit = 7; bit >= 0; bit--)
  {
    palClearLine(LINE_A);
    __delay();
    palSetLine(LINE_A);
    __delay();
    data |= ((palReadLine(LINE_B) == PAL_HIGH) ? 1 : 0) << bit;
    __delay();
    palClearLine(LINE_A);
    __delay();
  }
  palSetLineMode(LINE_B, PAL_MODE_OUTPUT_OPENDRAIN);
  return data;
}

void swi2c_write(uint8_t data)
{
  int8_t bit;
  for (bit = 7; bit >= 0; bit--)
  {
    if (data & (1 << bit))
      palSetLine(LINE_B);
    else
      palClearLine(LINE_B);
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
  if (!swi2c_wait_ack())
  {
    swi2c_stop();
    return 0;
  }
  swi2c_stop();
  return 1;
}

#ifdef SWI2C_A8 //8bit address

uint8_t swi2c_readByte_A8(uint8_t dev_addr, uint8_t addr, uint8_t *pbyte)
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
  swi2c_write(SWI2C_RMSK | ((dev_addr & SWI2C_DMSK) << SWI2C_ASHF));
  swi2c_ack();
  //if (!swi2c_wait_ack()) return 0;
  uint8_t byte = swi2c_read();
  swi2c_nack();
  swi2c_stop();
  if (pbyte)
    *pbyte = byte;
  return 1;
}

uint8_t swi2c_writeByte_A8(uint8_t dev_addr, uint8_t addr, uint8_t *pbyte)
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
    0x0ff};

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
    0x0ff};
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
uint8_t PAT9125init()
{

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
    for (;;)
    {
      const uint8_t addr = pat9125_init_seq1[i++];
      if (addr == 0x0ff)
        break;
      if (!pat9125_wr_reg_verify(addr, pat9125_init_seq1[i++]))
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
    for (;;)
    {
      const uint8_t addr = pat9125_init_seq2[i++];
      if (addr == 0x0ff)
        break;
      if (!pat9125_wr_reg_verify(addr, pat9125_init_seq2[i++]))
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
    if (iDY & 0x800)
      iDY -= 4096;
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

/*
  Belows are hardware i2c, use PCA9685 PWM driver chip
*/

uint8_t rxBuffer[32];
uint8_t rxBufferIndex = 0;
uint8_t rxBufferLength = 0;

uint8_t txAddress = 0;
uint8_t txBuffer[32];
uint8_t txBufferIndex = 0;
uint8_t txBufferLength = 0;

/*!
 *  @brief  Instantiates a new PCA9685 PWM driver chip with the I2C address on a
 * TwoWire interface
 *  @param  i2c  A pointer to a 'Wire' compatible object that we'll use to
 * communicate with
 *  @param  addr The 7-bit I2C address to locate this chip, default is 0x40
 */
Adafruit_PWMServoDriver::Adafruit_PWMServoDriver(uint8_t addr)
{
  _i2caddr = addr;
}

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
void Adafruit_PWMServoDriver::begin(uint8_t prescale)
{
  reset();
  if (prescale)
  {
    setExtClk(prescale);
  }
  else
  {
    // set a default frequency
    setPWMFreq(1000);
  }
}

/*!
 *  @brief  Sends a reset command to the PCA9685 chip over I2C
 */
void Adafruit_PWMServoDriver::reset()
{
  write8(PCA9685_MODE1, 0x80);
  delay(10);
}

/*!
 *  @brief  Puts board into sleep mode
 */
void Adafruit_PWMServoDriver::sleep()
{
  uint8_t awake = read8(PCA9685_MODE1);
  uint8_t sleep = awake | 0x10; // set sleep bit high
  write8(PCA9685_MODE1, sleep);
  delay(5); // wait until cycle ends for sleep to be active
}

/*!
 *  @brief  Wakes board from sleep
 */
void Adafruit_PWMServoDriver::wakeup()
{
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
void Adafruit_PWMServoDriver::setExtClk(uint8_t prescale)
{
  uint8_t oldmode = read8(PCA9685_MODE1);
  uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
  write8(PCA9685_MODE1, newmode);            // go to sleep, turn off internal oscillator

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
void Adafruit_PWMServoDriver::setPWMFreq(float freq)
{
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
uint8_t Adafruit_PWMServoDriver::getPWM(uint8_t num)
{
  // _i2c->requestFrom((int)_i2caddr, LED0_ON_L + 4 * num, (int)4);
  uint8_t data = 0;
  uint8_t txbuf[1] = {LED0_ON_L + 4 * num};
  uint8_t rxbuf[1];
  // swi2c_readByte_A8((int)_i2caddr, 0x00, &data); //not yet done
  i2cMasterTransmit(&I2CD2, 0x40, txbuf, 1, rxbuf, 1);
  // return _i2c->read();
  return rxbuf[0];
}

/*!
 *  @brief  Sets the PWM output of one of the PCA9685 pins
 *  @param  num One of the PWM output pins, from 0 to 15
 *  @param  on At what point in the 4096-part cycle to turn the PWM output ON
 *  @param  off At what point in the 4096-part cycle to turn the PWM output OFF
 */

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
size_t write(uint8_t data)
{
  // if(transmitting){
  // in master transmitter mode
  // don't bother if buffer is full
  // if(txBufferLength >= BUFFER_LENGTH){
  //   setWriteError();
  //   return 0;
  // }
  // put byte in tx buffer
  txBuffer[txBufferIndex] = data;
  ++txBufferIndex;
  // update amount in buffer
  txBufferLength = txBufferIndex;
  // }else{
  // in slave send mode
  // reply to master
  // twi_transmit(&data, 1);
  // }
  return 1;
}

static uint8_t twi_masterBuffer[32];
static volatile uint8_t twi_masterBufferIndex;
static volatile uint8_t twi_masterBufferLength;

static uint8_t twi_txBuffer[32];
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static uint8_t twi_rxBuffer[32];
static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twi_error;

/* 
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
uint8_t twi_writeTo(uint8_t address, uint8_t *data, uint8_t length, uint8_t wait, uint8_t sendStop)
{
  swi2c_start();
  swi2c_write(SWI2C_WMSK | ((address & SWI2C_DMSK) << SWI2C_ASHF));
  swi2c_ack();
  for (int i = 0; i < length; ++i)
  {
    swi2c_write(data[i] & 0xff);
    swi2c_ack();
  }
  swi2c_stop();
}

uint8_t endTransmission()
{
  // transmit buffer (blocking)
  uint8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1, true);
  // reset tx buffer iterator vars
  txBufferIndex = 0;
  txBufferLength = 0;
  // indicate that we are done transmitting
  // transmitting = 0;
  return ret;
}

void Adafruit_PWMServoDriver::setPWM(uint8_t num, uint16_t on, uint16_t off)
{
#ifdef ENABLE_DEBUG_OUTPUT
  Serial.print("Setting PWM ");
  Serial.print(num);
  Serial.print(": ");
  Serial.print(on);
  Serial.print("->");
  Serial.println(off);
#endif
  // swi2c_start();
  // swi2c_write(SWI2C_WMSK | ((0x40 & SWI2C_DMSK) << SWI2C_ASHF));
  // swi2c_ack();
  // swi2c_write(LED0_ON_L + 4 * num);
  // swi2c_ack();
  // swi2c_write(on);
  // swi2c_ack();
  // swi2c_write((on >> 8) );//& 0b111);
  // swi2c_ack();
  // swi2c_write(off);
  // swi2c_ack();
  // swi2c_write((off >> 8) );//& 0b111);
  // swi2c_ack();
  // swi2c_stop();
  uint8_t txbuf[5] = {
      LED0_ON_L + 4 * num,
      on, on >> 8, off, off >> 8};
  i2cMasterTransmit(&I2CD2, 0x40, txbuf, 5, 0, 0);
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
void Adafruit_PWMServoDriver::setPin(uint8_t num, uint16_t val, bool invert)
{
  // Clamp value between 0 and 4095 inclusive.
  // val = min(val, (uint16_t)4095);
  val = (val < (uint16_t)4095) ? val : 4095;
  if (invert)
  {
    if (val == 0)
    {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 4095)
    {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else
    {
      setPWM(num, 0, 4095 - val);
    }
  }
  else
  {
    if (val == 4095)
    {
      // Special value for signal fully on.
      setPWM(num, 4096, 0);
    }
    else if (val == 0)
    {
      // Special value for signal fully off.
      setPWM(num, 0, 4096);
    }
    else
    {
      setPWM(num, 0, val);
    }
  }
}

uint8_t Adafruit_PWMServoDriver::read8(uint8_t addr)
{
  // _i2c->beginTransmission(_i2caddr);
  Adafruit_PWMServoDriver(_i2caddr);
  // _i2c->write(addr);
  write(addr);
  // _i2c->endTransmission();
  endTransmission();
  uint8_t txbuf[1] = {addr};
  uint8_t rxbuf[1];
  i2cMasterTransmit(&I2CD2, 0x40, txbuf, 1, rxbuf, 1);
  return rxbuf[0];
}

void Adafruit_PWMServoDriver::write8(uint8_t addr, uint8_t d)
{
  uint8_t txbuf[2] = {
      addr,
      d};
  i2cMasterTransmit(&I2CD2, 0x40, txbuf, 2, 0, 0);
}

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

static THD_WORKING_AREA(PAT9125ELThd_wa, 1024);
static THD_FUNCTION(PAT9125ELThd, p)
{

  (void)p;

  while (!chThdShouldTerminateX())
  {

    for (uint8_t pwmnum = 0; pwmnum < 16; pwmnum++)
    {
      pwm.setPWM(pwmnum, 0, 4000);
    }

    chThdSleepMilliseconds(10);
  }
}

void myi2cstart()
{
  // palSetLineMode(LINE_C, PAL_MODE_OUTPUT_PUSHPULL); //for 3.3V
  // palSetLineMode(LINE_F, PAL_MODE_OUTPUT_PUSHPULL); //for 3.3V
  // palSetLineMode(LINE_A, PAL_MODE_OUTPUT_OPENDRAIN);
  // palSetLineMode(LINE_B, PAL_MODE_OUTPUT_PUSHPULL);
  // palSetLine(LINE_A);
  // Adafruit_PWMServoDriver(0x40);
  pwm.begin();
  pwm.setPWMFreq(1600); // This is the maximum PWM frequency

  // PAT9125init();
  chThdCreateStatic(PAT9125ELThd_wa, sizeof(PAT9125ELThd_wa),
                    NORMALPRIO + 9, PAT9125ELThd, NULL);
}

uint8_t pat9125_wr_reg_verify(uint8_t addr, uint8_t data)
{
  pat9125_wr_reg(addr, data);
  return pat9125_rd_reg(addr) == data;
}
/*
want to use software i2c,
1 (VLD) to LINE_C
3 (VDD) to LINE_F
4 (GND) to GND
5 (SCL) to LINE_A 
6 (SDA) to LINE_B
*/