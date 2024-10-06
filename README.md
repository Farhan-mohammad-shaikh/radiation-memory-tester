# radiation-memory-tester
This repository contains a test system for validating the integrity of the MR25H128A memory under radiation using the nRF52832 board and Zephyr RTOS. The system writes predefined data patterns to the IS25LP128 memory, reads back the data, and compares it to detect radiation-induced bit flips or errors. This project is specifically designed for environments where memory stability is crucial under adverse conditions such as radiation.

## Project Overview

The main objective of this project is to assess the robustness and reliability of the MR25H128A memory when exposed to radiation. The system executes a series of read/write tests using various data patterns to identify any inconsistencies, ensuring the memory performs accurately and without corruption.

### Key Features
- **Memory Used**: IS25LP128 (128 Mbit Serial Flash Memory)
- **Board**: nRF52832 Development Kit
- **RTOS**: Zephyr RTOS
- **Connection Details**: Specified connection to the memory module through the SPI interface
- **Test Patterns**: Checkerboard, Walking Ones/Zeroes, March Test, Random Data
- **Error Detection**: Compares the read data with the expected pattern to identify bit flips or memory corruption

## Hardware and Software Requirements

### Hardware
- nRF52832 Development Kit
- IS25LP128 Memory Module
- Debugger (e.g., J-Link) for programming and debugging

### Software
- Zephyr RTOS (2.6.0 or later recommended)
- ARM GCC Toolchain
- nRF Command Line Tools or Segger Embedded Studio
- CMake (for building the project)
