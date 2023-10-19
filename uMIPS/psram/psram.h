#ifndef __PSRAM_H
#define __PSRAM_H

#include "pico/stdlib.h"
#include "../config/umips_config.h"

#define OPTIMAL_RAM_WR_SZ 16
#define OPTIMAL_RAM_RD_SZ 16

void accessPSRAM(uint32_t addr, size_t size, bool write, void *bufP);
int initPSRAM();
void RAMGetStat(uint64_t* reads, uint64_t* writes);

#endif