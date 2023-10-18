#ifndef __PSRAM_H
#define __PSRAM_H

#include "pico/stdlib.h"
#include "../config/umips_config.h"

void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP);
void __accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP);
int initPSRAM();
void RAMGetStat(uint64_t* reads, uint64_t* writes);

#endif