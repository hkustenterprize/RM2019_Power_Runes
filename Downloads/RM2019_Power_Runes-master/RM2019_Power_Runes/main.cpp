/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.hpp"
#include "hal.h"
#include "arm_math.h"

#include <stdio.h>
#include <stdlib.h>

#include "chprintf.h"
#include "shell.h"

#include "boardSetup.h"

#include "flash.h"
#include "usbcfg.h"
#include "ShellManager.h"
#include "DR16.h"
#include "CAN_Manager.h"
#include "SH1106.h"
#include "usbcfg.h"
#include "shellApp.hpp"

#include "spiconfig.h"
#include "i2c.hpp"

//void myi2cstart();
//volatile uint8_t mydata;
//static volatile uint64_t position;

// //motor feedbacks
// volatile int16_t g_torque_1 = 0;
// volatile int16_t g_rpm_1 = 0;
// volatile int16_t g_orientation_1 = 0; // [0,8191] -> [0,360 degree)

// extern "C"
// {
//     msg_t i2c_write_bit(I2CDriver *i2cp, unsigned bit);
// }

// void motor1Rx(const CANRxFrame *rxFrame)
// {
//     //received from motor 1
//     g_orientation_1 = rxFrame->data8[0] << 8 | rxFrame->data8[1];
//     g_rpm_1 = rxFrame->data8[2] << 8 | rxFrame->data8[3];
//     g_torque_1 = rxFrame->data8[4] << 8 | rxFrame->data8[5];
// };

// /**
//  * @brief CAN1_SID_Receivers: example input to CAN_Manager to receive for some CAN IDs
//  *
//  */
// /*CAN_receiver_entry CAN1_SID_Receivers[] =
//     {{0x201, motor1Rx},
//      {0, NULL}};
// */
// volatile uint16_t raw_gyro[3] = {0};

// volatile uint8_t DelX = 3;

// //static uint16_t readword(uint8_t address)
// // static uint16_t readword(uint8_t address) //SPI
// // {
// //     static uint16_t data = 0;
// //     data = ((uint16_t)address) << 8;
// //     //spiAcquireBus(ADIS16470_SPID);            //request from user specified address
// //     //spiSelect(ADIS16470_SPID);
// //     //spiStartExchange(ADIS16470_SPID, sizeof(data), &address, &data);
// //     g_rpm_1 = 123;
// //     //spiSend(&SPID4, 1, &address);
// //     //g_rpm_1 = 123;
// //     //spiUnselect(ADIS16470_SPID);
// //     //spiStop(ADIS16470_SPID);

// //     //chThdSleep(20); //stall time specified by data sheet

// //     //spiSelect(ADIS16470_SPID);                //read word
// //     spiReceive(&SPID4, 1, &data);
// //     //spiUnselect(ADIS16470_SPID);
// //     //spiReleaseBus(ADIS16470_SPID);

// //     //chThdSleep(TIME_US2I(16));                    //stall time specified by data sheet

// //     return data;
// // }

// // static const SPIConfig SPI1_Config = {
// //     false,
// //     NULL,
// //     SPI_CR1_BIDIMODE | (0b111 << SPI_CR1_BR_Pos),
// //     0};
// // static const SPIConfig SPI1_Config_1 = {
// //     false,
// //     NULL,
// //     SPI_CR1_MSTR | SPI_CR1_DFF | SPI_CR1_BR_2 | SPI_CR1_BR_0 | SPI_CR1_CPHA | SPI_CR1_CPOL | SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE,
// //     0};

// // static const I2CConfig i2cfg1 = {
// //     0,
// //     LINE_I1,
// //     LINE_I2,
// //     2};

// static uint16_t temp = 0x02;
// // uint16_t readRegister(uint16_t addr)
// // {
// //     spiStart(&SPID4, &SPI1_Config_1);
// //     //cs = 0;                                 //Set chip select low/active
// //     addr = addr & 0x7F; //Set MSB to 0 to indicate read operation
// //     // spiSend(&SPID4, 1, &addr); //Write the given address
// //     temp = 0x02;
// //     // chThdSleep(10);
// //     // spiReceive(&SPID4, 2, &temp);    //Throw dummy byte after sending address to receieve data
// //     //cs = 1;                                 //Set chip select back to high/inactive
// //     // spiStop(&SPID4);
// //     //spiStart(&SPID4, &SPI1_Config_1);
// //     spiExchange(&SPID4, 1, &addr, &temp);
// //     return temp; //Returns 8-bit data from register
// // }

// /**
//  * @brief   Sends data via the I2C bus.
//  * @details Function designed to realize "read-through-write" transfer
//  *          paradigm. If you want transmit data without any further read,
//  *          than set @b rxbytes field to 0.
//  *
//  * @param[in] i2cp      pointer to the @p I2CDriver object
//  * @param[in] addr      slave device address (7 bits) without R/W bit
//  * @param[in] txbuf     pointer to transmit buffer
//  * @param[in] txbytes   number of bytes to be transmitted
//  * @param[out] rxbuf    pointer to receive buffer
//  * @param[in] rxbytes   number of bytes to be received, set it to 0 if
//  *                      you want transmit only
//  * @param[in] timeout   the number of ticks before the operation timeouts,
//  *                      the following special values are allowed:
//  *                      - @a TIME_INFINITE no timeout.
//  *                      .
//  *
//  * @return              The operation status.
//  * @retval MSG_OK       if the function succeeded.
//  * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
//  *                      be retrieved using @p i2cGetErrors().
//  * @retval MSG_TIMEOUT  if a timeout occurred before operation end.
//  *
//  * @api
//  */
// msg_t i2cMasterTransmitTimeout2(I2CDriver *i2cp,
//                                 i2caddr_t addr,
//                                 const uint8_t *txbuf,
//                                 size_t txbytes,
//                                 uint8_t *rxbuf,
//                                 size_t rxbytes,
//                                 sysinterval_t timeout)
// {
//     msg_t rdymsg;

//     //osalDbgCheck((i2cp != NULL) &&
//     //             (txbytes > 0U) && (txbuf != NULL) &&
//     //             ((rxbytes == 0U) || ((rxbytes > 0U) && (rxbuf != NULL))) &&
//     //             (timeout != TIME_IMMEDIATE));

//     //osalDbgAssert(i2cp->state == I2C_READY, "not ready");

//     osalSysLock();
//     i2cp->errors = I2C_NO_ERROR;
//     i2cp->state = I2C_ACTIVE_TX;
//     rdymsg = i2c_lld_master_transmit_timeout(i2cp, addr, txbuf, txbytes,
//                                              rxbuf, rxbytes, timeout);
//     rdymsg = MSG_OK;
//     if (rdymsg == MSG_TIMEOUT)
//     {
//         i2cp->state = I2C_LOCKED;
//     }
//     else
//     {
//         i2cp->state = I2C_READY;
//     }
//     osalSysUnlock();
//     return rdymsg;
// }

// static uint16_t temp2 = 0x02;
// static volatile msg_t msg;
// uint8_t readi2c(uint16_t addr)
// {
//     temp2 = 0x02;
//     // size_t addrsize = sizeof(addr);
//     // size_t temp2size = sizeof(temp2);
//     static volatile uint16_t reg;
//     static volatile uint16_t read;
//     // reg = 0x14;
//     // read = 0;
//     // msg = i2cMasterTransmitTimeout2(&I2CD2,
//     //                                0x0079U,
//     //                                (uint8_t *)&reg,
//     //                                1,
//     //                                (uint8_t *)&read,
//     //                                1,
//     //                                TIME_MS2I(200));
//     // i2c_write_bit(&I2CD2,1);

//     return read;
// }

// static THD_WORKING_AREA(PAT9125ELThd_wa, 1024);
// static THD_FUNCTION(PAT9125ELThd, p)
// {

//     (void)p;

//     i2cStart(&I2CD2, &i2cfg1);
//     while (!chThdShouldTerminateX())
//     {
//         DelX = 3;
//         g_rpm_1 = 123;
//         //DelX = readword(Product_ID1);
//         //DelX = readRegister(Delta_X_Lo);
//         // DelX = readi2c(0x0A);
//         //spiSend(&SPID4, 2, &fdsfdsfd);
//         g_rpm_1 = 123;
//         //adis16470_update(&adis16470);
//         //adis16470_get_gyro_raw(raw_gyro);
//         chThdSleep(TIME_US2I(16));
//         chThdSleepMilliseconds(2);
//         //adis16470.counter = readword(0x22);

//         chThdSleep(ADIS16470_UPDATE_PERIOD);
//     }
// }

void setPWM(uint8_t num, uint16_t on, uint16_t off)
{

    uint8_t txbuf[5] = {
        LED0_ON_L + 4 * num,
        on, on >> 8 & 0x0f, off, off >> 8 & 0x0f};
    i2cMasterTransmit(&I2CD2, 0x40, txbuf, 5, 0, 0);
}

int main(void)
{
    /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

    halInit();

    chSysInit();

    board_setup_all();

    //DR16_start();
    //OLED_Init();
    //OLED_Display_On();

    //OLED_BUFFER_ARRAY(buffer_array, 127, 64);
    //OLED_DIRTY_ARRAY(dirty_array, 127, 64);

    //OLED_Buffer buffer = OLED_BufferInit(127, 64,  dirty_array,  buffer_array);
    //OLED_drawString(&buffer, 3, 0, 120, 64, "For developing on specific robot, it is recommended to put source and header files in the root directory, and group them with folders if necessary.", FontSize_6x8, true);
    //OLED_writeToScreen(&buffer, 0, 0, false);

    // ShellManager_start(g_shellCommands);

    // CAN_Manager_Start(CAN1_SID_Receivers, NULL, NULL, NULL);
    // char tempStr[10];

    //spiInit();
    //palSetLineMode(GPIOE_K1, PAL_MODE_ALTERNATE);
    //palSetLineMode(GPIOE_K2, PAL_MODE_ALTERNATE);
    //spiStop(ADIS16470_SPID);

    // myi2cstart();

    //spiStart(&SPID4, &SPI1_Config);

    // chThdCreateStatic(PAT9125ELThd_wa, sizeof(PAT9125ELThd_wa),
    //                  NORMALPRIO + 9, PAT9125ELThd, NULL);

    //palSetLineMode(LINE_OLED_5_SCLK, PAL_MODE_ALTERNATE(5));
    //palSetLineMode(LINE_OLED_6_MOSI, PAL_MODE_ALTERNATE(5));

    static const I2CConfig i2cfg1 = {
        OPMODE_I2C,
        400000,
        // 96000,
        // 46000,
        FAST_DUTY_CYCLE_16_9};

    i2cStart(&I2CD2, &i2cfg1);

    myi2cstart();

    palSetLineMode(LINE_VL53L0X_I2C_SDA, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    palSetLineMode(LINE_VL53L0X_I2C_SCL, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    // palSetLineMode(LINE_VL53L0X_I2C_SDA, PAL_MODE_OUTPUT_OPENDRAIN);
    // palSetLineMode(LINE_VL53L0X_I2C_SCL, PAL_MODE_OUTPUT_OPENDRAIN);

    palSetLineMode(LINE_C, PAL_MODE_OUTPUT_PUSHPULL); //for 3.3V
    palSetLineMode(LINE_F, PAL_MODE_OUTPUT_PUSHPULL); //for 3.3V
    while (true)
    {

        //g_rpm_1 = 1;
        //sprintf(tempStr, "%d", g_rpm_1);
        palToggleLine(LINE_LED_GREEN);
        // for (int i = 0; i < 16; i++)
        //     {setPWM(i, 10, 4000);
        //     chThdSleepMilliseconds(10);}
        uint8_t addr = LED0_ON_L + 4;
        uint8_t rxbuf[20];
        // i2cMasterTransmit(&I2CD2, 0x40, &addr, 1, rxbuf, 20);
        chThdSleepMilliseconds(500);
    }
}
