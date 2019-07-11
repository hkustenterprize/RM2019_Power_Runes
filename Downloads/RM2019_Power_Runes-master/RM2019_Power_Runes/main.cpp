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

    static const I2CConfig i2cfg1 = {
        OPMODE_I2C,
        400000,
        FAST_DUTY_CYCLE_16_9};

    i2cStart(&I2CD2, &i2cfg1);

    myi2cstart();

    palSetLineMode(LINE_VL53L0X_I2C_SDA, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);
    palSetLineMode(LINE_VL53L0X_I2C_SCL, PAL_MODE_ALTERNATE(4) | PAL_STM32_OTYPE_OPENDRAIN);

    while (true)
    {
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
