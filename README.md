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
1. Over volt the RP2040 core slightly
2. Over clock the RP2040 to 400MHz
3. Initlise the UART interface
4. Initlise the console (UART and emulator glue) API
5. Initlise and reset PSRAM
6. Switch threads and Hand over control to RP2040 SoC uMIPS implementation
7. Initlise MMIO
8. Initlise and mount SD card
9. Open disk image
10. Start uMIPS CPU

The boot process of Linux goes as follows:
1. uMIPS CPU starts
2. Hardcoded ROM Jumps into Disk Image's BROM (BootROM)
3. BROM copies Kernel into RAM
4. BROM clears BSS
5. BROM jumps into kernel location
6. Linux Kernel starts executing.

## Usage
### Preparing the Hardware
##### Components needed
S.no | Part                                      | No. needed
-----| ------------------------------------------| -----------------------
1    | Raspberry Pi Pico                         | 1
2    | Micro SD card (>=8GB)                     | 1
3    | Adafruit Micro SD card Reader(Breadboard) | 1
4    | PSRAM64H                                  | 2
5    | Wires                                     | As many as Required :\)
6    | Breadboard/PCB                            | 1
7    | SOP8 to DIP8 PCB (Breadboard)             | 2
8    | Micro SD Card Slot (PCB)                  | 1

##### Assembly
> [!WARNING]
> Due to the PSRAM64H chips being too small to directly use on a breadboard, it is recomended to use a SOP8 to DIP8. IF you are using the custom PCB made for this project, then the SOP8 to DIP8 is not required.

Breadboard:
1. Solder the PSRAM chips to the SOP8 to DIP8 PCBs
2. Solder male headers to the SOP8 to DIP8 PCBs
3. Place the Raspberry Pi Pico, PSRAM chips and Micro SD card Reader Mobule on the breadboard
4. Connect The modules according the below table:

Pico GPIO | Module  | Pin
--------- | ------- | ----------------
GPIO 18   | SD Card | CLK/SCK
GPIO 16   | SD Card | MISO
GPIO 19   | SD Card | MOSI
GPIO 20   | SD Card | CS/SS
VSYS      | SD Card | VCC
GND       | SD Card | GND
          |         |
GPIO10    | PSRAM 1 | CLK/SCK/SCLK
GPIO12    | PSRAM 1 | MISO/SO/SIO[1]
GPIO11    | PSRAM 1 | MOSI/SI/SIO[0]
GPIO21    | PSRAM 1 | CS#/CS/SS
3v3 (OUT) | PSRAM 1 | VCC
GND       | PSRAM 1 | VSS
          |         |
GPIO10    | PSRAM 2 | CLK/SCK/SCLK
GPIO12    | PSRAM 2 | MISO/SO/SIO[1]
GPIO11    | PSRAM 2 | MOSI/SI/SIO[0]
GPIO22    | PSRAM 2 | CS#/CS/SS
3v3 (OUT) | PSRAM 2 | VCC
GND       | PSRAM 2 | VSS

5. Now the Circuit is ready to be flashed and run Linux!

PCB:
1. Solder the Raspberry Pi Pico to the PCB according to the footprint
2. Solder the Micro SD Card Slot according to the footprint
3. Solder the PSRAM Chips according to the footprint
4. Solder Male headers as shown on silkscreen if required (For debugging)
5. Now the Circuit is ready to be flashed and run linux!

### Preparing the Software
##### Building the Emulator Code (Linux)
1. Setup the Pico SDK
2. Clone the repo.
3. cd into the folder in a terminal
4. Run `mkdir build`
5. Then `cmake ..` for a release build or `cmake -DCMAKE_BUILD_TYPE=Debug ..` for a debug build
6. Run `make`
7. The output will be in build/uMIPS/ folder

##### Flashing the Emulator Code (BOOTSEL Mode)
1. Disconnect the Raspberry Pi Pico from the computer
2. While pressing the `BOOTSEL` button on the pico, connect the pico the the computer
3. Copy the file `build/uMIPS/pico-uMIPS.uf2` into the newly appeared flash drive `RPI-RP2`

##### Preparing the SD Card
TODO
