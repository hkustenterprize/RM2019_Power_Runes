#ifndef _ADIS16470_H_
#define _ADIS16470_H_

#include "ch.h"
#include "hal.h"

//#include "math_misc.h"

#define GRAV                               9.80665f
#define ADIS16470_ACCEL_DATA_PSC_16        1.25e-3f * GRAV
#define ADIS16470_GYRO_DATA_PSC_16         1.745329252e-3f
#define ADIS16470_ACCEL_DATA_PSC_16        1.25e-3f * GRAV
#define ADIS16470_GYRO_DATA_PSC_32         2.66316109e-8f
#define ADIS16470_ACCEL_DATA_PSC_32        1.907348633e-8f * GRAV
#define ADIS16470_TEMP_PSC                 0.1f
//----configure for hardware of IMU   include sample period and frequency -> gor low pass filter--
#define ADIS16470_SPID                     &SPID4
#define ADIS16470_UPDATE_PERIOD            TIME_US2I(500)
#define ADIS16470_SAMPLE_FREQ               2000U
//-------------------------------------------------------------------------
#define ADIS16470_BURST_CMD                0x6800
#define ADIS16470_BURST_SIZE               10

#define ADIS16470_X_GYRO_LOW                   0x04
#define ADIS16470_X_GYRO_OUT                   0x06
#define ADIS16470_Y_GYRO_LOW                   0x08
#define ADIS16470_Y_GYRO_OUT                   0x0A
#define ADIS16470_Z_GYRO_LOW                   0x0C
#define ADIS16470_Z_GYRO_OUT                   0x0E
#define ADIS16470_X_ACCL_LOW                   0x10
#define ADIS16470_X_ACCL_OUT                   0x12
#define ADIS16470_Y_ACCL_LOW                   0x14
#define ADIS16470_Y_ACCL_OUT                   0x16
#define ADIS16470_Z_ACCL_LOW                   0x18
#define ADIS16470_Z_ACCL_OUT                   0x1A

#define ADIS16470_MSC_CTRL                 0x60

#define ADIS16470_INTERNAL_CLK             0x0000

#define ADIS16470_ACCEL_CUTOFF_FREQ 10.0f
#define ADIS16470_GYRO_CUTOFF_FREQ  60.0f

#define IMU_USE_EULER_ANGLE

typedef struct
{
  uint16_t diag_stat;                   //For diagnostic
  int16_t gyroData[3];                  //1LSB = 0.1deg/s
  int16_t accelData[3];                 //1LSB = 1.25mg
  int16_t  temperature;                 //1LSB = 0.1deg C
  uint16_t data_cnt;                    //1LSB = 500us
  uint16_t  checksum : 8;               //16-bit checksum
  uint16_t  res      : 8;               //reserved
}__attribute__((packed)) adis16470_data_t;

typedef enum
{
  ADIS16470_UNINIT               = 0,
  ADIS16470_READY,
  ADIS16470_CALIBRATING,
  ADIS16470_NOT_READY
} adis16470_state_t;

typedef enum
{
  ADIS16470_ERROR                = 0xFF00,
  ADIS16470_SENSOR_ERROR         = 0x1000,
  ADIS16470_AXIS_CONF_ERROR      = 0x2000,
  ADIS16470_READING_ERROR        = 0x4000,
  ADIS16470_UNCONNECTED          = 0x8000,
  ADIS16470_WARNING              = 0x00FF,
  ADIS16470_ACCEL_NOT_CALIBRATED = 0x0010,
  ADIS16470_GYRO_NOT_CALIBRATED  = 0x0020,
  ADIS16470_DATA_INVALID         = 0x0040,
  ADIS16470_LOSE_FRAME           = 0x0080,
  ADIS16470_CORRUPT_Q_DATA       = 0x0001,
} adis16470_error_t;


typedef enum{
  SET_X = 0,
  SET_Y = 1,
  SET_Z = 2,

  IMU_X = 0,
  IMU_Y = 1,
  IMU_Z = 2
}axis_index_t;

typedef enum{
  REVERSE = -1,
  NORMAL = 1
}axis_reverse_t;



typedef struct 
{
  axis_index_t index;
  axis_reverse_t reverse;
}axes_status_t;
typedef struct 
{
  axes_status_t axes[3];
}axes_congif_t;

typedef struct data_16470_t{

//==============sensor direct data output===================//
  uint16_t diag_stat;                   //For diagnostic
  volatile float gyroData[3];          //1LSB = 0.1deg/s
  volatile float accelData[3];         //1LSB = 1.25mg
  float temperature;                   //1LSB = 0.1deg C
  volatile uint32_t stamp;
//==============end of sensor data output===================//

const axes_congif_t           *config;

volatile float gyroDataFiltered[3];          //1LSB = 0.1deg/s

  adis16470_state_t state;
  adis16470_error_t error;
  uint8_t calibration_id;

  float qIMU[4];          /* Attitude quaternion of the IMU. */

  #ifdef  IMU_USE_EULER_ANGLE
    float euler_angle[3];      /* Euler angle of the IMU. */
    float d_euler_angle[3];    /* Euler angle changing rate of the IMU. */

    int rev;                 /* used to detect zero-crossing */
    float prev_yaw; //TODO change this inside a static global variable
  #else
    float dqIMU[4];         /* Attitude quaternion changing rate of the IMU. */
  #endif

  uint16_t counter;
  thread_reference_t imu_Thd;

} adis16470Struct;


#ifdef __cplusplus
extern "C"
{
#endif

void IMUStart(const axes_congif_t* config);
void adis16470_get_gyro_raw(int32_t gyroRawData[3]);
void adis16470_get_accel_raw(int32_t accelRawData[3]);
adis16470Struct* getImuData(void);

/**
 * @source pixhawk/src/lib/mathlib/math/filter/LowPassFilter2p.cpp
 *
 * Data structure for a IIR second-order sections form filter
 * b_0 + b_1 * z^-1 + b_2 * z^-2
 * -------------------------------
 *   1 + a_1 * z^-1 + a_2 * z^-2
 */
typedef struct {
  float a1;
  float a2;
  float b0;
  float b1;
  float b2;

  float data[2];
} lpfilterStruct;

#ifdef __cplusplus
}
#endif

#define Product_ID1       0x00    //Product Identifier [11:4]
#define Product_ID2       0x01    //Upper 4 bits for Product Identifier, PID [3:0]
                                  //Lower 4 bits for Product Version, VID [3:0]
#define Motion_Status     0x02    //Motion Status information
#define Delta_X_Lo        0x03    //8-bit 2’s complement number for X-movement data in 8-bit movement data format
                                  //X-movement = Delta_X_Lo[7:0]
#define Delta_Y_Lo        0x04    //8-bit 2’s complement number for Y-movement data in 8-bit
                                  //movement data format
                                  //Y-movement = Delta_Y_Lo[7:0]
#define Operation_Mode    0x05    //Operation mode selection
#define Configuration     0x06    //Software power down and reset
#define Write_Protect     0x09    //Write Protect to avoid missed-writing registers
#define Sleep1            0x0A    //Sleep1 configuration
#define Sleep2            0x0B    //Sleep2 configuration
#define RES_X             0x0D    //CPI resolution setting for X axis
#define RES_Y             0x0E    //CPI resolution setting for Y axis
#define Delta_XY_Hi       0x12    //High nibble of X-movement and Y-movement for 12-bit 2's complement data format
                                  //X-movement = {Delta_XY_Hi[7:4], Delta_X_Lo[7:0]}
                                  //Y-movement = {Delta_XY_Hi[3:0], Delta_Y_Lo[7:0]}
#define Shutter           0x14
#define Frame_Avg         0x17
#define Orientation       0x19
//Reset***********************
#define re_Product_ID1    0x31
#define re_Product_ID2    0x91
#define re_Operation_Mode 0xA0
#define re_Configuration  0x17
#define re_Write_Protect  0x00
#define re_Sleep1         0x77
#define re_Sleep2         0x10
#define re_RES_X          0x14
#define re_RES_Y          0x14

    static const SPIConfig PACCfg = {
        //false,
        //NULL,
        //LINE_ADIS_SPI_NSS,
        //SPI_CR1_MSTR | SPI_CR1_DFF | SPI_CR1_BR_2 | SPI_CR1_BR_0 | SPI_CR1_CPHA | SPI_CR1_CPOL
        //0
        false,
        NULL,
        0b010 << SPI_CR1_BR_Pos,
        0
    };


#endif