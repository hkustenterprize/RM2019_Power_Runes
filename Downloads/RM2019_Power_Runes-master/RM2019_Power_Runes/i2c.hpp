
#include "ch.hpp"
#include "hal.h"
#include "arm_math.h"

#include <stdio.h>
#include <stdlib.h>

#include "spiconfig.h"

extern void myi2cstart();
extern int64_t position;

//SWI2C configuration
#define SWI2C
//#define SWI2C_SDA         20 //SDA on P3
//#define SWI2C_SCL         21 //SCL on P3
#define SWI2C_A8
#define SWI2C_DEL         20 //2us clock delay
#define SWI2C_TMO         2048 //2048 cycles timeout

//PAT9125 configuration
#define PAT9125_SWI2C
//#define PAT9125_I2C_ADDR  0x75  //ID=LO
//#define PAT9125_I2C_ADDR  0x73  //ID=HI
// #define PAT9125_I2C_ADDR  0x79  //ID=NC
#define PAT9125_I2C_ADDR  0x70  //ID=NC
#define PAT9125_XRES      0
#define PAT9125_YRES      240

//PAT9125 registers
#define PAT9125_PID1			0x00
#define PAT9125_PID2			0x01
#define PAT9125_MOTION			0x02
#define PAT9125_DELTA_XL		0x03
#define PAT9125_DELTA_YL		0x04
#define PAT9125_MODE			0x05
#define PAT9125_CONFIG			0x06
#define PAT9125_WP				0x09
#define PAT9125_SLEEP1			0x0a
#define PAT9125_SLEEP2			0x0b
#define PAT9125_RES_X			0x0d
#define PAT9125_RES_Y			0x0e
#define PAT9125_DELTA_XYH		0x12
#define PAT9125_SHUTTER			0x14
#define PAT9125_FRAME			0x17
#define PAT9125_ORIENTATION		0x19
#define PAT9125_BANK_SELECTION	0x7f


#define PCA9685_SUBADR1 0x2 /**< i2c bus address 1 */
#define PCA9685_SUBADR2 0x3 /**< i2c bus address 2 */
#define PCA9685_SUBADR3 0x4 /**< i2c bus address 3 */

#define PCA9685_MODE1 0x0 /**< Mode Register 1 */
#define PCA9685_PRESCALE 0xFE /**< Prescaler for PWM output frequency */

#define LED0_ON_L 0x6 /**< LED0 output and brightness control byte 0 */
#define LED0_ON_H 0x7 /**< LED0 output and brightness control byte 1 */
#define LED0_OFF_L 0x8 /**< LED0 output and brightness control byte 2 */
#define LED0_OFF_H 0x9 /**< LED0 output and brightness control byte 3 */

#define ALLLED_ON_L 0xFA /**< load all the LEDn_ON registers, byte 0 */
#define ALLLED_ON_H 0xFB /**< load all the LEDn_ON registers, byte 1 */
#define ALLLED_OFF_L 0xFC /**< load all the LEDn_OFF registers, byte 0 */
#define ALLLED_OFF_H 0xFD /**< load all the LEDn_OFF registers, byte 1 */

/*! 
 *  @brief  Class that stores state and functions for interacting with PCA9685 PWM chip
 */
class Adafruit_PWMServoDriver {
 public:
//   Adafruit_PWMServoDriver(TwoWire *I2C = &Wire, uint8_t addr = 0x40);
  void begin(uint8_t prescale = 0);
  void reset();
  void sleep();
  void wakeup();
  void setExtClk(uint8_t prescale);
  void setPWMFreq(float freq);
  uint8_t getPWM(uint8_t num);
  void setPWM(uint8_t num, uint16_t on, uint16_t off);
  void setPin(uint8_t num, uint16_t val, bool invert=false);

 private:
  uint8_t _i2caddr;
  
  //TwoWire *_i2c;

  uint8_t read8(uint8_t addr);
  void write8(uint8_t addr, uint8_t d);
};