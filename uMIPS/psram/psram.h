#ifndef __PSRAM_H
#define __PSRAM_H

#include "pico/stdlib.h"
#include "../config/umips_config.h"

void writePSRAM(uint32_t addr, size_t size, void *bufP);
void readPSRAM(uint32_t addr, size_t size, void *bufP);
void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP);
int initPSRAM();

#endif