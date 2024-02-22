#pragma GCC optimize ("O2")

#include <stdlib.h>

#include "../config/umips_config.h"
#include "psram.h"
#include "cache.h"

// #define PSRAM_CMD_RES_EN 0x66
// #define PSRAM_CMD_RESET 0x99
// #define PSRAM_CMD_READ_ID 0x9F
// #define PSRAM_CMD_READ 0x03
// #define PSRAM_CMD_READ_FAST 0x0B
// #define PSRAM_CMD_WRITE 0x02
#define PSRAM_KGD 0x5D

// #define selectPsramChip(c) gpio_put(c, false)
// #define deSelectPsramChip(c) gpio_put(c, true)

void spi_tx_array(const uint16_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            gpio_put(PSRAM_SPI_PIN_TX_S1, (data[i] >> j) & 0x01);
            gpio_put(PSRAM_SPI_PIN_TX_S2, (data[i] >> (j + 8)) & 0x01);
            gpio_put(PSRAM_SPI_PIN_CK, 1); 
            asm("nop");                    
            gpio_put(PSRAM_SPI_PIN_CK, 0);
        }

        // console_printf_uart("TX: 0x%4x\r\n", data[i]);
    }
}

void spi_rx_array(uint16_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        for (int j = 7; j >= 0; j--)
        {
            gpio_put(PSRAM_SPI_PIN_CK, 1); 
            asm("nop");                    
            gpio_put(PSRAM_SPI_PIN_CK, 0);
            byte1 |= (gpio_get(PSRAM_SPI_PIN_RX_S1) << j);
            byte2 |= (gpio_get(PSRAM_SPI_PIN_RX_S2) << j);
            // console_printf_uart("RX S2: %d", gpio_get(PSRAM_SPI_PIN_RX_S2));
        }
        // data[i] = byte1;
        // data[i+1] = byte2;
        data[i] = byte1 | (byte2 << 8);
        // console_printf_uart("RX: 0x%4x\r\n", data[i]);
        // console_printf_uart("RX: 0x%2x\t", byte1);
        // console_printf_uart("0x%2x\r\n", byte2);
        // data[i] = byte1;
        // data[i+1] = byte2;
    }
}

// void sendPsramCommand(uint16_t cmd) {
//     spi_tx_array(&cmd, 1);
// }

// void psramReset()
// {
//     selectPsramChip(PSRAM_SPI_PIN_SS);
//     sendPsramCommand(PSRAM_CMD_RES_EN);
//     sendPsramCommand(PSRAM_CMD_RESET);
//     deSelectPsramChip(PSRAM_SPI_PIN_SS);
//     sleep_ms(10);
// }

// void psramReadID(uint16_t *dst)
// {
//     selectPsramChip(PSRAM_SPI_PIN_SS);
//     sendPsramCommand(PSRAM_CMD_READ_ID);
//     spi_tx_array(dst, 3);
//     spi_rx_array(dst, 6);
//     deSelectPsramChip(PSRAM_SPI_PIN_SS);
// }

// int initPSRAM()
// {
//     // spi_pulse_sck_asm();
//     gpio_init(PSRAM_SPI_PIN_SS);
//     gpio_init(PSRAM_SPI_PIN_CK);

//     gpio_init(PSRAM_SPI_PIN_TX_S1);
//     gpio_init(PSRAM_SPI_PIN_RX_S1);

//     // gpio_init(PSRAM_SPI_PIN_TX_S2);
//     // gpio_init(PSRAM_SPI_PIN_RX_S2);

//     gpio_set_dir(PSRAM_SPI_PIN_SS, GPIO_OUT);
//     deSelectPsramChip(PSRAM_SPI_PIN_SS);

//     gpio_set_dir(PSRAM_SPI_PIN_TX_S1, GPIO_OUT);
//     gpio_set_dir(PSRAM_SPI_PIN_RX_S1, GPIO_IN);
//     // gpio_set_dir(PSRAM_SPI_PIN_TX_S2, GPIO_OUT);
//     // gpio_set_dir(PSRAM_SPI_PIN_RX_S2, GPIO_IN);
//     gpio_set_dir(PSRAM_SPI_PIN_CK, GPIO_OUT);

//     gpio_set_slew_rate(PSRAM_SPI_PIN_SS, GPIO_SLEW_RATE_FAST);

//     gpio_set_slew_rate(PSRAM_SPI_PIN_TX_S1, GPIO_SLEW_RATE_FAST);
//     gpio_set_slew_rate(PSRAM_SPI_PIN_RX_S1, GPIO_SLEW_RATE_FAST);
//     // gpio_set_slew_rate(PSRAM_SPI_PIN_TX_S2, GPIO_SLEW_RATE_FAST);
//     // gpio_set_slew_rate(PSRAM_SPI_PIN_RX_S2, GPIO_SLEW_RATE_FAST);
//     gpio_set_slew_rate(PSRAM_SPI_PIN_CK, GPIO_SLEW_RATE_FAST);

//     sleep_ms(10);

//     psramReset();

//     uint16_t chipId[6];
//     psramReadID(chipId);
//     if (chipId[1] >> 8 != PSRAM_KGD)
//         return -13;
//     if (chipId[1] & 0xff != PSRAM_KGD)
//         return -23;

//     // cacheInit();

//     writePSRAM(0, 4, chipId);
//     readPSRAM(0, 4, chipId);
    
//     if (chipId[1] != PSRAM_KGD)
//         return -73;

//     return 13;
// }

void readPSRAM(uint32_t addr, size_t size, void *bufP) {
    // accessPSRAM(addr, size, false, bufP);
    // if(size != 64)
    //     cacheRead(addr, bufP, size);
    // else {
    //     cacheRead(addr, bufP, 32);
    //     cacheRead(addr + 32, bufP + 32, 32);
    // }

    // cacheRead(addr, bufP, size);

    // cache_read(addr, bufP, size);

    cache_read_l2(addr, bufP, size);
}
void writePSRAM(uint32_t addr, size_t size, void *bufP) {
    // accessPSRAM(addr, size, true, bufP);
    // if(size != 64)
    //     cacheWrite(addr, bufP, size);
    // else {
    //     cacheWrite(addr, bufP, 32);
    //     cacheWrite(addr + 32, bufP + 32, 32);
    // }

    // cacheWrite(addr, bufP, size);

    // cache_write(addr, bufP, size);

    cache_write_l2(addr, bufP, size);
}

// uint8_t cmdAddr[5];

// void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP)
// {
//     // uint8_t *b = (uint8_t *)bufP;
//     // uint cmdSize = 4;

//     // if (write)
//     //     cmdAddr[0] = PSRAM_CMD_WRITE;
//     // else
//     // {
//     //     cmdAddr[0] = PSRAM_CMD_READ_FAST;
//     //     cmdSize++;
//     // }

//     // cmdAddr[1] = (addr >> 16) & 0xff;
//     // cmdAddr[2] = (addr >> 8) & 0xff;
//     // cmdAddr[3] = addr & 0xff;

//     // selectPsramChip(ramchip);
//     // spi_tx_array(cmdAddr, cmdSize);

//     // if (write)
//     //     spi_tx_array(b, size);
//     // else
//     //     spi_rx_array(b, size);

//     // deSelectPsramChip(ramchip);
// }

#include <stdlib.h>

#include "../config/umips_config.h"
#include "psram.h"

#define PSRAM_CMD_RES_EN 0x66
#define PSRAM_CMD_RESET 0x99
#define PSRAM_CMD_READ_ID 0x9F
#define PSRAM_CMD_READ 0x03
#define PSRAM_CMD_READ_FAST 0x0B
#define PSRAM_CMD_WRITE 0x02
#define PSRAM_KGD 0x5D

#define selectPsramChip(c) gpio_put(c, false)
#define deSelectPsramChip(c) gpio_put(c, true)

#define spi_set_mosi(value) gpio_put(PSRAM_SPI_PIN_TX_S1, value)
#define spi_read_miso() gpio_get(PSRAM_SPI_PIN_RX_S1)

#define spi_pulse_sck()                \
    {                                  \
        asm("nop");                    \
        gpio_put(PSRAM_SPI_PIN_CK, 1); \
        asm("nop");                    \
        gpio_put(PSRAM_SPI_PIN_CK, 0); \
    }

// void spi_tx_array(const uint8_t *data, size_t size)
// {
//     for (size_t i = 0; i < size; i++)
//     {
//         uint8_t byte = data[i];
//         for (int j = 7; j >= 0; j--)
//         {
//             spi_set_mosi((byte >> j) & 0x01);
//             spi_pulse_sck();
//         }
//     }
// }

// void spi_rx_array(uint8_t *data, size_t size)
// {
//     for (size_t i = 0; i < size; i++)
//     {
//         uint8_t byte = 0;
//         for (int j = 7; j >= 0; j--)
//         {
//             spi_pulse_sck();
//             byte |= (spi_read_miso() << j);
//         }
//         data[i] = byte;
//     }
// }

void sendPsramCommand(uint8_t cmd, uint chip)
{
    if (chip)
        selectPsramChip(chip);
    uint16_t cmdx2 = (cmd << 8) | cmd;
    spi_tx_array(&cmdx2, 1);

    if (chip)
        deSelectPsramChip(chip);
}

void psramReset(uint chip)
{
    sendPsramCommand(PSRAM_CMD_RES_EN, chip);
    sendPsramCommand(PSRAM_CMD_RESET, chip);
    sleep_ms(10);
}

void psramReadID(uint chip, uint16_t *dst)
{
    selectPsramChip(chip);
    sendPsramCommand(PSRAM_CMD_READ_ID, 0);
    spi_tx_array(dst, 3);
    spi_rx_array(dst, 6);
    deSelectPsramChip(chip);
}

int initPSRAM()
{
    gpio_init(PSRAM_SPI_PIN_SS);
    gpio_init(PSRAM_SPI_PIN_RX_S1);
    gpio_init(PSRAM_SPI_PIN_TX_S1);
    gpio_init(PSRAM_SPI_PIN_RX_S2);
    gpio_init(PSRAM_SPI_PIN_TX_S2);
    gpio_init(PSRAM_SPI_PIN_CK);

    gpio_set_dir(PSRAM_SPI_PIN_SS, GPIO_OUT);
    gpio_set_dir(PSRAM_SPI_PIN_RX_S1, GPIO_IN);
    gpio_set_dir(PSRAM_SPI_PIN_TX_S1, GPIO_OUT);
    gpio_set_dir(PSRAM_SPI_PIN_RX_S2, GPIO_IN);
    gpio_set_dir(PSRAM_SPI_PIN_TX_S2, GPIO_OUT);
    gpio_set_dir(PSRAM_SPI_PIN_CK, GPIO_OUT);

    deSelectPsramChip(PSRAM_SPI_PIN_SS);

    gpio_put(PSRAM_SPI_PIN_TX_S1, 0);
    gpio_put(PSRAM_SPI_PIN_TX_S2, 0);
    gpio_put(PSRAM_SPI_PIN_CK, 0);

    sleep_ms(10);

    psramReset(PSRAM_SPI_PIN_SS);

    uint16_t chipId[6];

    psramReadID(PSRAM_SPI_PIN_SS, chipId);
    if ((chipId[1] & 0xff) != PSRAM_KGD)
        return -1;

    if ((chipId[1] >> 8) != PSRAM_KGD)
        return -2;

    return 1;
}

uint16_t cmdAddr[5];

void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP)
{
    uint8_t *b = (uint8_t *)bufP;
    uint cmdSize = 4;
    
    if (write)
        cmdAddr[0] = PSRAM_CMD_WRITE | PSRAM_CMD_WRITE << 8;
    else
    {
        cmdAddr[0] = PSRAM_CMD_READ_FAST | PSRAM_CMD_READ_FAST << 8;
        cmdSize++;
    }

    cmdAddr[1] = (addr >> 16) & 0xff | ((addr >> 16) & 0xff) << 8;
    cmdAddr[2] = (addr >> 8) & 0xff | ((addr >> 8) & 0xff) << 8;
    cmdAddr[3] = addr & 0xff | (addr & 0xff) << 8;

    selectPsramChip(PSRAM_SPI_PIN_SS);
    spi_tx_array(cmdAddr, cmdSize);

    if(size % 2) console_panic_uart("Ayo, PSRAM read/write size is not even!");

    uint16_t tmpb[size/2];

    if (write) {
        for(int i = 0; i < size/2; i++) {
            // b[i] = tmpb[i] & 0xff;
            // b[i+1] = (tmpb[i] << 8) & 0xff;
            tmpb[i] = b[i] | (b[i+1] << 8);
        }
        spi_tx_array(tmpb, size/2);
    } else {
        spi_rx_array(tmpb, size/2);
        for(int i = 0; i < size/2; i++) {
            b[i] = tmpb[i] & 0xff;
            b[i+1] = (tmpb[i] << 8) & 0xff;
        }
    }

    deSelectPsramChip(PSRAM_SPI_PIN_SS);
}