/**
 * @file boardSetup.h
 * @author Alex Au (alex_acw@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2018-12-30
 * 
 * @copyright Copyright (c) 2018
 * 
 */
#ifndef boardSetup_H_
#define boardSetup_H_

#include "ch.h"
#include "hal.h"
#include "usbcfg.h"

#define USE_DR16 TRUE

#define DR16_UART_DRIVER UARTD1
#define DR16_UART_RX LINE_USART1_RX

#define OLED_SPI_DRIVER SPID1
#define OLED_SPI_MOSI LINE_OLED_6_MOSI
#define OLED_SPI_CLK LINE_OLED_5_SCLK
#define OLED_RST_LINE LINE_OLED_3_RST
#define OLED_DC_LINE LINE_OLED_4_DC

//SHELL_SD: if this is not defined, means shell will run on SDU1 from usbcfg.c
// #define SHELL_SD UARTD2

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    * @brief the only exported function here, used to initiallize all on-chip hardwares
    */
    void board_setup_all(void);

#ifdef __cplusplus
}
#endif

#endif //define boardSetup_H_