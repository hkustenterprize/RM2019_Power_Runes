/**
 * @file shellApp.cpp
 * @author Alex Au (alex_acw@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-01-04
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <cstring>
#include "ShellManager.h"
#include "memstreams.h"
#include "DR16.h"
#include "flash.h"
#include "CRC.h"

// TODO: migrate logging function to ShellManager
THD_WORKING_AREA(shellLogger_wa, 32);
THD_FUNCTION(shellLogger, arg)
{
    (void)arg;
    chRegSetThreadName("usb_shell_logger");

    while (!chThdShouldTerminateX())
    {

        /**
         * chprintf might be blocking in some streams (e.g. USB),
         * printing should be done in separate thread, like here 
         */

        if (g_DR16_isConnected)
            chprintf((BaseSequentialStream *)&SHELL_CHN,
                     "\nDR16:\nch0 %d\nch1 %d\nch2 %d\nch3 %d\ns1 %d\ns2 %d\n\n",
                     g_DR16_RC_Value.rc.ch0,
                     g_DR16_RC_Value.rc.ch1,
                     g_DR16_RC_Value.rc.ch2,
                     g_DR16_RC_Value.rc.ch3,
                     g_DR16_RC_Value.rc.s1,
                     g_DR16_RC_Value.rc.s2);
        else
            chprintf((BaseSequentialStream *)&SHELL_CHN,
                     "\nDR16: remote not connected\n\n");

        chThdSleepMilliseconds(2000);
    }
}

/**
 * @brief start the logger thread if it is not started yet
 * 
 */
void ShellApp_Logger_Start()
{
    static thread_t *p_shellLoggerThread = NULL;
    if (!p_shellLoggerThread)
        p_shellLoggerThread =
            chThdCreateStatic(&shellLogger_wa,
                              sizeof(shellLogger_wa),
                              NORMALPRIO,
                              shellLogger,
                              NULL);
}

void testFlashOpen(BaseSequentialStream *chp, int argc, char *argv[])
{
    Flash_Open(23);
};

void testFlashAppend(BaseSequentialStream *chp, int argc, char *argv[])
{
    if (argc > 0)
        Flash_Append(argv[0], strlen(argv[0]));
    else
        chprintf(chp, "write something to Flash_Append\n");
};

void testFlashClose(BaseSequentialStream *chp, int argc, char *argv[])
{
    Flash_Close();
};

void testCRC_Reset(BaseSequentialStream *chp, int argc, char *argv[])
{
    CRC32_reset();
};

void testCRC_Accumulate(BaseSequentialStream *chp, int argc, char *argv[])
{
    if (argc > 0)
    {
        CRC32_accumulate((uint8_t *)argv[0], strlen(argv[0]));
        chprintf(chp, "CRC: %X\n", CRC32_getValue());
    }
    else
        chprintf(chp, "write something to CRC32_accumulate\n");
};

ShellCommand g_shellCommands[] =
    {{"fo", testFlashOpen},
     {"fa", testFlashAppend},
     {"fc", testFlashClose},
     {"cr", testCRC_Reset},
     {"ca", testCRC_Accumulate},
     {NULL, NULL}};
