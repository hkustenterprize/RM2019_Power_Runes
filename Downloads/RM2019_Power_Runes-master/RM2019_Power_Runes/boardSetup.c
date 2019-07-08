/**
 * @file boardSetup.c
 * @author Alex Au (alex_acw@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2018-12-30
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "boardSetup.h"
#include "usbcfg.h"
#include "DR16.h"
#include "CRC.h"

/**
 * @brief Set the up SPI1 driver for the OLED screen
 * 
 */
static inline void setup_SPI1(void)
{
    palSetLineMode(LINE_OLED_5_SCLK, PAL_MODE_ALTERNATE(5));
    palSetLineMode(LINE_OLED_6_MOSI, PAL_MODE_ALTERNATE(5));

    static const SPIConfig SPI1_Config = {
        false,
        NULL,
        0b010 << SPI_CR1_BR_Pos,
        0};
    spiStart(&SPID1, &SPI1_Config);
}

/**
    * @brief Initializes pin interrupt callbacks and events
    * notice that pins with same number (eveb with different port)
    * cannot have their interrupt enabled together
    */
static inline void setup_Pins(void){
    // enable interrupt from the user button (normal high)
    // palEnableLineEvent(LINE_USER_BUTTON, PAL_EVENT_MODE_FALLING_EDGE);
};

void board_setup_all(void)
{
    CRC32_init();
    DR16_initDBus(&UARTD1);
    setup_Pins();
    setup_SPI1();

#ifndef SHELL_SD
    setup_USB();
#endif //SHELL_USE_USB
};
