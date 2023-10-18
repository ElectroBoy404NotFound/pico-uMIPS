#ifndef _RV32_CONFIG_H
#define _RV32_CONFIG_H

/******************/
/* Emulator config
/******************/

// RAM size in megabytes
#define EMULATOR_RAM_MB 16

#define TICKS_PER_SECOND 1000

// FPU setting
#define FPU_SUPPORT_NONE

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
#define PSRAM_SPI_SPEED 50

#endif
// Pins for the PSRAM SPI interface
#define PSRAM_SPI_PIN_CK 10
#define PSRAM_SPI_PIN_TX 11
#define PSRAM_SPI_PIN_RX 12

// Select lines for the two PSRAM chips
#define PSRAM_SPI_PIN_S1 21
#define PSRAM_SPI_PIN_S2 22
#define PSRAM_SPI_PIN_S3 14
#define PSRAM_SPI_PIN_S4 15

// PSRAM chip size (in kilobytes)
#define PSRAM_CHIP_SIZE (8192 * 1024)

// Use two PSRAM chips?
#define PSRAM_TWO_CHIPS   1
// Use three PSRAM chips?
#define PSRAM_THREE_CHIPS 0
// Use four PSRAM chips?
#define PSRAM_FOUR_CHIPS  0

/****************/
/* SD card config
/* SD SPI interface
/******************/

// SPI instance used for SD (if used)
#define SD_SPI_INSTANCE spi0

// Pins for the SD SPI interface (if used)
#define SD_SPI_PIN_MISO 16
#define SD_SPI_PIN_MOSI 19
#define SD_SPI_PIN_CLK 18
#define SD_SPI_PIN_CS 20

/*******************/
/* Config Checks
/* DO NOT MODIFY!
/******************/
#if PSRAM_FOUR_CHIPS
    #undef PSRAM_THREE_CHIPS
    #undef PSRAM_TWO_CHIPS

    #define PSRAM_THREE_CHIPS 0
    #define PSRAM_TWO_CHIPS 0
#else
    #if PSRAM_THREE_CHIPS
        #undef PSRAM_FOUR_CHIPS
        #undef PSRAM_TWO_CHIPS

        #define PSRAM_FOUR_CHIPS 0
        #define PSRAM_TWO_CHIPS 0
    #endif
#endif

#if !PSRAM_TWO_CHIPS && !PSRAM_THREE_CHIPS && !PSRAM_FOUR_CHIPS && PSRAM_CHIP_SIZE < (EMULATOR_RAM_MB * 1024 * 1024)
    #error "RAM Size too Big! 8MB < RAM"
#endif
#if PSRAM_TWO_CHIPS && PSRAM_CHIP_SIZE * 2 < (EMULATOR_RAM_MB * 1024 * 1024)
    #error "RAM Size too Big! 16MB < RAM"
#endif
#if PSRAM_THREE_CHIPS && PSRAM_CHIP_SIZE * 3 < (EMULATOR_RAM_MB * 1024 * 1024)
    #error "RAM Size too Big! 24MB < RAM"
#endif
#if PSRAM_FOUR_CHIPS && PSRAM_CHIP_SIZE * 4 < (EMULATOR_RAM_MB * 1024 * 1024)
    #error "RAM Size too Big! 32MB < RAM"
#endif

#endif
