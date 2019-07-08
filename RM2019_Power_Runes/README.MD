### DJI RoboMaster 2018 Development Board Type.A Embedded System Application Layer Code by HKUST Team "ENTERPRIZE"

### This repository is used for demoing robot application code that can be implemented based on the "Core" repository

## Getting Started
### Building & Uploading
**Robot specific codes are placed under src/[robot name]**
* Run `make` to compile
* If OpenOCD and drivers are well configured, run `make uploadSTLink` to upload with st-link or `make uploadJLink` to upload with j-link (still haven't figure out how to upload via j-link with make command in windows).
* Compile configuration are in `Makefile` for compiling common application code. By default every file with `.c .cpp` as suffix under the root folder of this repository will be compiled.

### Programming Style and Structure

* For developing on specific robot, it is recommended to  **put source and header files in the root directory, and group them with folders if necessary**.  

* **All pin configurations can be pre-configured in `board/board.h`, this file should only be generated from `board/cfg/board.chcfg`, to do that, install and run `fmpp` inside `board/cfg`**

* Setup of all peripherals (including our own implementation of peripheral interface), interrupts or optional pin configurations should be included in `boardSetup.c` and `boardSetup.h`, unless the pin is dedicated for specific purpose by hardware (e.g. LEDs, CAN lines, Buzzer ...)

* When using pins, use their name defined in `board/board.h`

* Developers should check whether the peripheral is already used in an incompatible way.

## Pending Tasks are and should be listed on ClickUp
