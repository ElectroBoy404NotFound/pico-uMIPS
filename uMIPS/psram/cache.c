#include "cache.h"

#pragma GCC optimize ("Ofast")

static struct Cache mCache;

static void cacheReadPSRAM(uint32_t addr, void *dstP) {
	accessPSRAM(addr, CACHE_LINE_SIZE, false, dstP);
}
static void cacheWritePSRAM(uint32_t addr, const void *srcP) {
	accessPSRAM(addr, CACHE_LINE_SIZE, true, srcP);
}

static struct CacheLine* cacheFillLinePSRAM(struct Cache *cache, struct CacheSet *set, uint32_t addr, bool loadFromRam) {
    uint16_t idx = to_us_since_boot(get_absolute_time()) % CACHE_NUM_WAYS;
	struct CacheLine *line = &set->line[idx];
	
	if (line->dirty)
		cacheWritePSRAM(line->addr * CACHE_LINE_SIZE, line->dataW);
	
	if (loadFromRam) 
		cacheReadPSRAM(addr / CACHE_LINE_SIZE * CACHE_LINE_SIZE, line->dataW);
	line->dirty = 0;
	line->addr = addr / CACHE_LINE_SIZE;
	
	return line;
}

void cacheInit(void)
{
	uint16_t way, set;
	
	for (set = 0; set < CACHE_NUM_SETS; set++) 
		for (way = 0; way < CACHE_NUM_WAYS; way++) 			
			mCache.set[set].line[way].addr = 0x7fffffff;	//definitely invalid
}

static uint16_t cacheHash(uint32_t addr) {
    #if (CACHE_NUM_SETS & (CACHE_NUM_SETS - 1))
	
		#if CACHE_NUM_SETS == 3
			#define RECIP 	0xaaab
			#define SHIFT	17
		#elif CACHE_NUM_SETS == 5
			#define RECIP 	0xcccd
			#define SHIFT	18
		#elif CACHE_NUM_SETS == 6
			#define RECIP 	0xaaab
			#define SHIFT	18
		#elif CACHE_NUM_SETS == 9
			#define RECIP 	0xe38f
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 10
			#define RECIP 	0xcccd
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 11
			#define RECIP 	0xba2f
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 12
			#define RECIP 	0xaaab
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 13
			#define RECIP 	0x9d8a
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 15
			#define RECIP 	0x8889
			#define SHIFT	19
		#elif CACHE_NUM_SETS == 18
			#define RECIP 	0xe38f
			#define SHIFT	20
		#elif CACHE_NUM_SETS == 20
			#define RECIP 	0xcccd
			#define SHIFT	20
		#elif CACHE_NUM_SETS == 22
			#define RECIP 	0xba2f
			#define SHIFT	20
		#elif CACHE_NUM_SETS == 24
			#define RECIP 	0xaaab
			#define SHIFT	20
		#elif CACHE_NUM_SETS == 26
			#define RECIP 	0x9d8a
			#define SHIFT	20
		#elif CACHE_NUM_SETS == 30
			#define RECIP 	0x8889
			#define SHIFT	20
		#else
			#error "we lack a reciprocal for this value - it will be slow. refusing!"
		#endif
		
		uint32_t div;
		
		addr /= CACHE_LINE_SIZE;
		addr = (uint16_t)addr;
		
		div = addr * RECIP;
		div >>= SHIFT;
		addr -= div * CACHE_NUM_SETS;
		
	#else
	
		addr /= CACHE_LINE_SIZE;
		addr %= CACHE_NUM_SETS;
	
	#endif
	
	return addr;
}

void cacheRead(uint32_t addr, void *dataP, uint16_t sz)	//assume not called with zero
{
    struct Cache *cache = &mCache;
    struct CacheSet *set = &cache->set[cacheHash(addr)];
    uint32_t *dptr, *dst = (uint32_t*)dataP, dummy1, dummy2;
    struct CacheLine *line; 
    uint16_t i;

    for (i = 0, line = &set->line[0]; i < CACHE_NUM_WAYS; i++, line++)
        if (line->addr == addr / CACHE_LINE_SIZE)
            goto found;
    
    
    if (sz == CACHE_LINE_SIZE) {
        cacheReadPSRAM(addr, dataP);
        return;
    }
    
    line = cacheFillLinePSRAM(cache, set, addr, true);

found:
    switch (sz) {
        case 1:
            *(uint8_t*)dataP = line->dataB[(addr % CACHE_LINE_SIZE) / sizeof(uint8_t)];
            break;
        
        case 2:
            *(uint16_t*)dataP = line->dataH[(addr % CACHE_LINE_SIZE) / sizeof(uint16_t)];
            break;
        
        case 4:
            *(uint32_t*)dataP = line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            break;
        
        case 8:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1}	\n"
                "	stmia %1!, {r0, r1}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(dptr), "1"(dst)
                :"memory", "r0", "r1"
            );
            break;
        
        case 16:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(dptr), "1"(dst)
                :"memory", "r0", "r1", "r2", "r3"
            );
            break;
        default:
            console_panic("unknown size %u\n", sz);
    }
}

void cacheWrite(uint32_t addr, const void *dataP, uint16_t sz) {
    struct Cache *cache = &mCache;
    struct CacheSet *set = &cache->set[cacheHash(addr)];
    const uint32_t *src = (const uint32_t*)dataP;
    uint32_t dummy1, dummy2;
    struct CacheLine *line; 
    uint32_t *dptr;
    uint16_t i;

    for (i = 0, line = &set->line[0]; i < CACHE_NUM_WAYS; i++, line++)
        if (line->addr == addr / CACHE_LINE_SIZE)
            goto found;    
    
    if (sz == CACHE_LINE_SIZE)	//missed writes of cache line size or more
        return cacheWritePSRAM(addr, dataP);

    line = cacheFillLinePSRAM(cache, set, addr, sz != CACHE_LINE_SIZE);

found:
    switch (sz) {
        case 1:
            line->dataB[(addr % CACHE_LINE_SIZE) / sizeof(uint8_t)] = *(uint8_t*)dataP;
            break;
        
        case 2:
            line->dataH[(addr % CACHE_LINE_SIZE) / sizeof(uint16_t)] = *(uint16_t*)dataP;
            break;
        
        case 4:
            line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)] = *(uint32_t*)dataP;
            break;
        
        case 8:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1}	\n"
                "	stmia %1!, {r0, r1}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(src), "1"(dptr)
                :"memory", "r0", "r1"
            );
            break;
        
        case 16:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(src), "1"(dptr)
                :"memory", "r0", "r1", "r2", "r3"
            );
            break;
        default:
            console_panic("unknown size %u\n", sz);
    }
    line->dirty = 1;
}