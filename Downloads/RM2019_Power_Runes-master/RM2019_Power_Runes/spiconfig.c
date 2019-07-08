#include <spiconfig.h>
#include "ch.h"
#include "hal.h"

#define ADIS16470_SPID                     &SPID4

static adis16470_data_t burst_data;          
static adis16470Struct adis16470;
static lpfilterStruct lp_accel[3], lp_gyro[3];     //low pass filter
//****************************************
static void adis16470_update(adis16470Struct* imu);
static uint16_t readword(uint8_t);
static void writeword(uint8_t, uint16_t);
//****************************************

//volatile uint16_t raw_gyro[3] = {0};

//volatile uint8_t DelX = 0;

typedef struct {
  int16_t ID1;
  int16_t ID2;
  int16_t Status;
  int16_t DeltaX;
  int16_t DeltaY;
}ReadfromPAT;
/*
static THD_WORKING_AREA(PAT9125ELThd_wa, 1024);
static THD_FUNCTION(PAT9125ELThd, p) {

  (void)p;

  while(!chThdShouldTerminateX())
  {
      DelX = 123;
      DelX = readword(Delta_X_Lo);
      //adis16470_update(&adis16470);
      //adis16470_get_gyro_raw(raw_gyro);
      chThdSleep(TIME_US2I(16)); 
      //adis16470.counter = readword(0x22);

      chThdSleep(ADIS16470_UPDATE_PERIOD);
      
  }
  
}

void spiinit(){

  palSetLineMode(LINE_ADIS_SPI_MISO, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_KADIS_SPI_MOSI, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  palSetLineMode(LINE_K2, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
  
  
    static const SPIConfig adis16470SpiCfg = {
        false,
        NULL,
        LINE_ADIS_SPI_NSS,
        SPI_CR1_MSTR | SPI_CR1_DFF | SPI_CR1_BR_2 | SPI_CR1_BR_0 | SPI_CR1_CPHA | SPI_CR1_CPOL,
        0
    };

     
    (*ADIS16470_SPID).rxdmamode |= STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_HWORD;
    (*ADIS16470_SPID).txdmamode |= STM32_DMA_CR_PSIZE_HWORD | STM32_DMA_CR_MSIZE_HWORD;
    
    spiStart(ADIS16470_SPID, &adis16470SpiCfg);

    lpfilter_init(&lp_accel[0], ADIS16470_SAMPLE_FREQ, ADIS16470_ACCEL_CUTOFF_FREQ);
    lpfilter_init(&lp_accel[1], ADIS16470_SAMPLE_FREQ, ADIS16470_ACCEL_CUTOFF_FREQ);
    lpfilter_init(&lp_accel[2], ADIS16470_SAMPLE_FREQ, ADIS16470_ACCEL_CUTOFF_FREQ);
    
    lpfilter_init(&lp_gyro[0], ADIS16470_SAMPLE_FREQ, ADIS16470_GYRO_CUTOFF_FREQ);
    lpfilter_init(&lp_gyro[1], ADIS16470_SAMPLE_FREQ, ADIS16470_GYRO_CUTOFF_FREQ);
    lpfilter_init(&lp_gyro[2], ADIS16470_SAMPLE_FREQ, ADIS16470_GYRO_CUTOFF_FREQ);

    chThdCreateStatic(PAT9125ELThd_wa, sizeof(PAT9125ELThd_wa),
                    NORMALPRIO + 7, PAT9125ELThd, NULL);

}

static uint16_t readword(uint8_t address)
{
  uint16_t data = 0;
  data = ((uint16_t)address) << 8;

  //spiAcquireBus(ADIS16470_SPID);            //request from user specified address
  spiSelect(ADIS16470_SPID);
  spiSend(ADIS16470_SPID, 1, &data);
  spiUnselect(ADIS16470_SPID);

  chThdSleep(TIME_US2I(16));                    //stall time specified by data sheet

  spiSelect(ADIS16470_SPID);                //read word
  spiReceive(ADIS16470_SPID, 1, &data);
  spiUnselect(ADIS16470_SPID);
  //spiReleaseBus(ADIS16470_SPID);

  chThdSleep(TIME_US2I(16));                    //stall time specified by data sheet

  return data;
}*/

static void writeword(uint8_t lowerAddress, uint16_t data)
{
  uint16_t txbuffer[2];
  //concatenate write bit, address and data to be written
  txbuffer[0] = 0x8000 | lowerAddress << 8 | (uint8_t)data;     //pack lower word
  txbuffer[1] = 0x8000 | (lowerAddress + 1) << 8 | data >> 8;   //pack higher word

  //spiAcquireBus(ADIS16470_SPID);
  spiSelect(ADIS16470_SPID);
  spiSend(ADIS16470_SPID, 1, &txbuffer);        //send lower word
  spiUnselect(ADIS16470_SPID);

  chThdSleep(TIME_US2I(16));                        //stall time specified by data sheet

  spiSelect(ADIS16470_SPID);
  spiSend(ADIS16470_SPID, 1, &txbuffer[1]);     //send higher word
  spiUnselect(ADIS16470_SPID);
  //spiReleaseBus(ADIS16470_SPID);

  chThdSleep(TIME_US2I(16));                    //stall time specified by data sheet
}