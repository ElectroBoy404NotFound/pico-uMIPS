# Pico-uMIPS
Port of Dmitry Grinberg's Linux Card uMIPS emulator to the Raspberry Pi Pico.

![image](images/consoleBooted.png)

## What is this?
This is a port of [Dmitry Grinberg's uMIPS emulator for his Linux Card](https://dmitry.gr/?r=05.Projects&proj=33.%20LinuxCard) on RP2040 (Raspberry Pi Pico).

This code boots Linux 4.4.292+ Kernel on RP2040.

## How does it work?
The idea is simple: The RP2040 runs Dmitry Grinberg's uMIPS emulator, which uses 2 to 4 PSRAM64H (or any PSRAM chip) as RAM and a file on a SD card's FAT32/exFAT partition as its main storage.

The code is written in C using Pico-SDK.

The code does the following (in order):
    - Over volt the RP2040 core slightly
    - Over clock the RP2040 to 400MHz
    - Initlise the UART interface
    - Initlise the console (UART and emulator glue) API
    - Initlise and reset PSRAM
    - Switch threads and Hand over control to RP2040 SoC uMIPS implementation
    - Initlise MMIO
    - Initlise and mount SD card
    - Open disk image
    - Start uMIPS CPU

The boot process of Linux goes as follows:
    - Emulator starts
    - ROM Jumps into Disk Image's BROM (BootROM)
    - BROM copies Kernel into RAM
    - BROM clears BSS
    - BROM jumps into kernel location
    - Linux Kernel starts executing.

## Usage
### Preparing the Hardware
##### Components needed
S.no | Part                          | No. needed
-----| ------------------------------| -----------------------
1    | Raspberry Pi Pico             | 1
2    | Micro SD card (>=8GB)         | 1
3    | Adafruit Micro SD card Reader | 1
4    | PSRAM64H                      | 2
5    | Wires                         | As many as Required :\)

##### Assembly
> [!WARNING]
> Due to the PSRAM64H chips being too small to directly use on a breadboard, it is recomended to use a