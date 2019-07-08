
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
#define PAT9125_I2C_ADDR  0x79  //ID=NC
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


