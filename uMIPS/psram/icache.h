#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void cpuIcacheFlushEntire(void);
void cpuIcacheFlushPage(uint32_t va);
bool cpuInstrFetchCached(uint32_t pc, int32_t *instrP);