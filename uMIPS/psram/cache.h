#pragma once

#include "psram.h"

#define CACHE_LINE_SIZE_ORDER	5		//must be at least the size of icache line, or else...
#define CACHE_NUM_WAYS			2		//number of lines a given PA can be in
#define CACHE_NUM_SETS			20		//number of buckets of PAs

#define CACHE_LINE_SIZE			(1 << CACHE_LINE_SIZE_ORDER)

#if OPTIMAL_RAM_WR_SZ > CACHE_LINE_SIZE || OPTIMAL_RAM_RD_SZ > CACHE_LINE_SIZE
	#error "you're in for a bad time"
#endif

struct CacheLine {
	uint32_t	dirty	:  1;
	uint32_t	addr	: 31;
	union {
		uint32_t dataW[CACHE_LINE_SIZE / sizeof(uint32_t)];
		uint16_t dataH[CACHE_LINE_SIZE / sizeof(uint16_t)];
		uint8_t dataB[CACHE_LINE_SIZE / sizeof(uint8_t)];
	};
};

struct CacheSet {
	struct CacheLine line[CACHE_NUM_WAYS];
};

struct Cache {
	struct CacheSet set[CACHE_NUM_SETS];
};

void cacheInit();

void cacheRead(uint32_t addr, void *dataP, uint16_t sz);
void cacheWrite(uint32_t addr, const void *dataP, uint16_t sz);