#pragma GCC optimize ("O2")

#include <stdlib.h>

#include "../config/umips_config.h"
#include "psram.h"

#if PSRAM_HARDWARE_SPI
#include "hardware/spi.h"
#endif

#define PSRAM_CMD_RES_EN 0x66
#define PSRAM_CMD_RESET 0x99
#define PSRAM_CMD_READ_ID 0x9F
#define PSRAM_CMD_READ 0x03
#define PSRAM_CMD_READ_FAST 0x0B
#define PSRAM_CMD_WRITE 0x02
#define PSRAM_KGD 0x5D

#pragma GCC optimize ("Ofast")
#define selectPsramChip(c) gpio_put(c, false)
#define deSelectPsramChip(c) gpio_put(c, true)

#if !PSRAM_HARDWARE_SPI
#define spi_set_mosi(value) gpio_put(PSRAM_SPI_PIN_TX, value)
#define spi_read_miso() gpio_get(PSRAM_SPI_PIN_RX)

#pragma GCC optimize ("O2")

void spi_tx_array(const uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            spi_set_mosi((data[i] >> j) & 0x01);
            gpio_put(PSRAM_SPI_PIN_CK, 1); 
            asm("nop");                    
            gpio_put(PSRAM_SPI_PIN_CK, 0);
        }
    }
}

void spi_rx_array(uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        uint8_t byte = 0;
        for (int j = 7; j >= 0; j--)
        {
            gpio_put(PSRAM_SPI_PIN_CK, 1); 
            asm("nop");                    
            gpio_put(PSRAM_SPI_PIN_CK, 0);
            byte |= (spi_read_miso() << j);
        }
        data[i] = byte;
    }

#define PSRAM_SPI_WRITE(buf, sz) spi_tx_array(buf, sz)
#define PSRAM_SPI_READ(buf, sz) spi_rx_array(buf, sz)
}

#else
#define PSRAM_SPI_WRITE(buf, sz) spi_write_blocking(PSRAM_SPI_INST, buf, sz)
#define PSRAM_SPI_READ(buf, sz) spi_read_blocking(PSRAM_SPI_INST, 0, buf, sz)
#endif

#pragma GCC optimize ("Ofast")


void sendPsramCommand(uint8_t cmd, uint chip)
{
    if (chip)
        selectPsramChip(chip);
    PSRAM_SPI_WRITE(&cmd, 1);

    if (chip)
        deSelectPsramChip(chip);
}

void psramReset(uint chip)
{
    sendPsramCommand(PSRAM_CMD_RES_EN, chip);
    sendPsramCommand(PSRAM_CMD_RESET, chip);
    sleep_ms(10);
}

void psramReadID(uint chip, uint8_t *dst)
{
    selectPsramChip(chip);
    sendPsramCommand(PSRAM_CMD_READ_ID, 0);
    PSRAM_SPI_WRITE(dst, 3);
    PSRAM_SPI_READ(dst, 6);
    deSelectPsramChip(chip);
}

int initPSRAM()
{
    // spi_pulse_sck_asm();
    gpio_init(PSRAM_SPI_PIN_S1);
    gpio_init(PSRAM_SPI_PIN_S2);

    gpio_init(PSRAM_SPI_PIN_TX);
    gpio_init(PSRAM_SPI_PIN_RX);
    gpio_init(PSRAM_SPI_PIN_CK);

    gpio_set_dir(PSRAM_SPI_PIN_S1, GPIO_OUT);
    deSelectPsramChip(PSRAM_SPI_PIN_S1);
    gpio_set_dir(PSRAM_SPI_PIN_S2, GPIO_OUT);
    deSelectPsramChip(PSRAM_SPI_PIN_S2);

#if PSRAM_HARDWARE_SPI
    uint baud = spi_init(PSRAM_SPI_INST, 1000 * 1000 * PSRAM_SPI_SPEED);
    gpio_set_function(PSRAM_SPI_PIN_TX, GPIO_FUNC_SPI);
    gpio_set_function(PSRAM_SPI_PIN_RX, GPIO_FUNC_SPI);
    gpio_set_function(PSRAM_SPI_PIN_CK, GPIO_FUNC_SPI);

#else
    gpio_set_dir(PSRAM_SPI_PIN_TX, GPIO_OUT);
    gpio_set_dir(PSRAM_SPI_PIN_RX, GPIO_IN);
    gpio_set_dir(PSRAM_SPI_PIN_CK, GPIO_OUT);
#endif

    gpio_set_slew_rate(PSRAM_SPI_PIN_S1, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(PSRAM_SPI_PIN_S2, GPIO_SLEW_RATE_FAST);

    gpio_set_slew_rate(PSRAM_SPI_PIN_TX, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(PSRAM_SPI_PIN_RX, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(PSRAM_SPI_PIN_CK, GPIO_SLEW_RATE_FAST);

    sleep_ms(10);

    psramReset(PSRAM_SPI_PIN_S1);
    uint8_t chipId[6];

    psramReadID(PSRAM_SPI_PIN_S1, chipId);
    if (chipId[1] != PSRAM_KGD)
        return -1;

    psramReadID(PSRAM_SPI_PIN_S2, chipId);
    if (chipId[1] != PSRAM_KGD)
        return -2;

    // cacheInit();

    writePSRAM(0, 4, chipId);
    readPSRAM(0, 4, chipId);
    
    if (chipId[1] != PSRAM_KGD)
        return -7;

#if PSRAM_HARDWARE_SPI
    baud = spi_set_baudrate(PSRAM_SPI_INST, 1000 * 1000 * PSRAM_SPI_SPEED);
    return baud;
#else
    return 1;
#endif
}

void readPSRAM(uint32_t addr, size_t size, void *bufP) {
    accessPSRAM(addr, size, false, bufP);
    // if(size != 64)
    //     cacheRead(addr, bufP, size);
    // else {
    //     cacheRead(addr, bufP, 32);
    //     cacheRead(addr + 32, bufP + 32, 32);
    // }

    // cache_read(addr, bufP, size);
}
void writePSRAM(uint32_t addr, size_t size, void *bufP) {
    accessPSRAM(addr, size, true, bufP);
    // if(size != 64)
    //     cacheWrite(addr, bufP, size);
    // else {
    //     cacheWrite(addr, bufP, 32);
    //     cacheWrite(addr + 32, bufP + 32, 32);
    // }

    // cache_write(addr, bufP, size);
}

uint8_t cmdAddr[5];

void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP)
{
    uint8_t *b = (uint8_t *)bufP;
    uint cmdSize = 4;
    uint ramchip = PSRAM_SPI_PIN_S1;

    if (write)
        cmdAddr[0] = PSRAM_CMD_WRITE;
    else
    {
        cmdAddr[0] = PSRAM_CMD_READ_FAST;
        cmdSize++;
    }

    if (addr >= PSRAM_CHIP_SIZE) {
        ramchip = PSRAM_SPI_PIN_S2;
        addr -= PSRAM_CHIP_SIZE;
    }

    cmdAddr[1] = (addr >> 16) & 0xff;
    cmdAddr[2] = (addr >> 8) & 0xff;
    cmdAddr[3] = addr & 0xff;

    selectPsramChip(ramchip);
    PSRAM_SPI_WRITE(cmdAddr, cmdSize);

    if (write)
        PSRAM_SPI_WRITE(b, size);
    else
        PSRAM_SPI_READ(b, size);

    deSelectPsramChip(ramchip);
}
