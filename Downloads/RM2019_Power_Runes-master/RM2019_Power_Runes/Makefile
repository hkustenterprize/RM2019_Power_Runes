PROJECT = RM18A
USE_HAL_I2C_FALLBACK = yes
# Imported source files and paths
CONFDIR = ./config
COREDIR = ./RM18A-ChibiOS18-Core

include ./board/board.mk

# Compile all .c and .cpp files in the root directory
ALLCSRC += $(wildcard ./*.c)
ALLCPPSRC += $(wildcard ./*.cpp)

include $(COREDIR)/core.mk
