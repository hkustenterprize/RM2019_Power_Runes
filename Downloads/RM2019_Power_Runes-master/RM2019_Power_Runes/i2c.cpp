#include "i2c.hpp"

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

static THD_WORKING_AREA(PCA9685ELThd_wa, 1024);
static THD_FUNCTION(PCA9685ELThd, p)
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

void PCA_i2cstart()
{
  // Adafruit_PWMServoDriver(0x40);
  pwm.begin();
  pwm.setPWMFreq(1600); // This is the maximum PWM frequency

  chThdCreateStatic(PCA9685ELThd_wa, sizeof(PCA9685ELThd_wa),
                    NORMALPRIO + 9, PCA9685ELThd, NULL);
}

