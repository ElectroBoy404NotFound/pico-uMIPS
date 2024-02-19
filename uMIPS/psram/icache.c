#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "icache.h"
#include "../emulator/memory/mem.h"
#include "../emulator/cpu/cpu.h"

#define ICACHE_LINE_SZ	32	//in bytes
#define ICACHE_NUM_SETS	32
#define ICACHE_NUM_WAYS	2

struct IcacheLine {
	uint32_t addr;	//kept as LSRed by ICACHE_LINE_SIZE, so 0xfffffffe is a valid "empty "sentinel
	uint8_t icache[ICACHE_LINE_SZ];
} mIcache[ICACHE_NUM_SETS][ICACHE_NUM_WAYS];


void cpuIcacheFlushEntire(void)
{
	memset(mIcache, 0xff, sizeof(mIcache));
}

void cpuIcacheFlushPage(uint32_t va)
{
	struct IcacheLine *line;
	uint_fast16_t i;
		
	line = mIcache[(va / ICACHE_LINE_SZ) % ICACHE_NUM_SETS];
	
	for (i = 0; i < ICACHE_NUM_WAYS; i++, line++) {
		
		if (line->addr == va / ICACHE_LINE_SZ)
			line->addr = 0xffffffff;
	}
}

bool cpuInstrFetchCached(uint32_t pc, int32_t *instrP)	//if false, do nothing, all has been handled
{
	uint32_t va = pc, pa;
	struct IcacheLine *line;
	uint_fast16_t i, set;
	static unsigned rng = 1;

//pretty hard to do this, so let's not check
//	if (va & 3) {
//		
//		cpuPrvTakeAddressError(va, false);
//		return false;
//	}
	
	set = (va / ICACHE_LINE_SZ) % ICACHE_NUM_SETS;
	line = mIcache[set];
	
	for (i = 0; i < ICACHE_NUM_WAYS; i++, line++) {
		
		if (line->addr == va / ICACHE_LINE_SZ) {

			goto hit;
		}
	}
	
	//miss
	line = mIcache[set];
	
	rng *= 214013;
	rng += 2531011;
	line += rng % ICACHE_NUM_WAYS;
		
	if (!cpuPrvMemTranslate(&pa, va, false))
		return false;
	
	pa /= ICACHE_LINE_SZ;
	pa *= ICACHE_LINE_SZ;
	
	if (!memAccess(pa, ICACHE_LINE_SZ, false, line->icache)) {
        cpuPrvTakeException(6);
		return false;
	}
	line->addr = va / ICACHE_LINE_SZ;
	
hit:
	*instrP = *(uint32_t*)(&line->icache[(va % ICACHE_LINE_SZ)]);	//god, i hope gcc optimizes this wel...
	return true;
}