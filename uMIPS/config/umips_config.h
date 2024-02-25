#ifndef _RV32_CONFIG_H
#define _RV32_CONFIG_H

/******************/
/* Emulator config
/******************/

// RAM size in megabytes
#define EMULATOR_RAM_MB 16

#define TICKS_PER_SECOND 90000000

#define FPU_SUPPORT_FULL

// Enable UART console
#define CONSOLE_UART 1

// Enable USB CDC console
#define CONSOLE_CDC 1


/******************/
/* UART config
/******************/

// UART instance
#define UART_INSTANCE uart0

// UART Baudrate (if enabled)
#define UART_BAUD_RATE 115200

// Pins for the UART (if enabled)
#define UART_TX_PIN 0
#define UART_RX_PIN 1

/******************/
/* PSRAM config
/******************/

// Use hardware SPI for PSRSAM (bitbang otherwise)
#define PSRAM_HARDWARE_SPI 0

#if PSRAM_HARDWARE_SPI

// Hardware SPI instance to use for PSRAM
#define PSRAM_SPI_INST spi1
// PSRAM SPI speed (in MHz)
#define PSRAM_SPI_SPEED 45

#endif
// Pins for the PSRAM SPI interface
// Select lines for the two PSRAM chips
#define PSRAM_SPI_PIN_SS 21
#define PSRAM_SPI_PIN_CK 10
#define PSRAM_SPI_PIN_TX_S1 15
#define PSRAM_SPI_PIN_RX_S1 13

#define PSRAM_SPI_PIN_TX_S2 11
#define PSRAM_SPI_PIN_RX_S2 12

// PSRAM chip size (in kilobytes)
#define PSRAM_CHIP_SIZE (8192 * 1024)

/****************/
/* SD card config
/* SD SPI interface
/******************/

// SPI instance used for SD
#define SD_SPI_INSTANCE spi0

// Pins for the SD SPI interface
#define SD_SPI_PIN_MISO 16
#define SD_SPI_PIN_MOSI 19
#define SD_SPI_PIN_CLK 18
#define SD_SPI_PIN_CS 20

#endif
