#define CACHE_TYPE 0
#pragma GCC optimize ("Ofast")

#if CACHE_TYPE == 0

/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#include <string.h>
#include <stddef.h>
#include "cache.h"
#include "psram.h"

//no matter how i play it, 64-byte lines make things slower, not faster
#define CACHE_LINE_SIZE_ORDER	5		//must be at least the size of icache line, or else...
#define CACHE_NUM_WAYS			2		//number of lines a given PA can be in
#define CACHE_NUM_SETS			20		//number of buckets of PAs


#define CACHE_LINE_SIZE			(1 << CACHE_LINE_SIZE_ORDER)
#if OPTIMAL_RAM_WR_SZ > CACHE_LINE_SIZE || OPTIMAL_RAM_RD_SZ > CACHE_LINE_SIZE
	#error "you're in for a bad time"
#endif
///cache

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

static struct Cache mCache;

void cacheInit(void)
{
	uint_fast16_t way, set;
	
	for (set = 0; set < CACHE_NUM_SETS; set++) {
		for (way = 0; way < CACHE_NUM_WAYS; way++) {
			
			mCache.set[set].line[way].addr = 0x7fffffff;	//definitely invalid
		}
	}
}

static uint_fast16_t spiRamCachePrvHash(uint32_t addr)
{
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

static uint_fast16_t spiRamCachePrvPickVictim(struct Cache *cache)
{
	return getTimeMillis() % CACHE_NUM_WAYS;
}

static struct CacheLine* spiRamCachePrvFillLine(struct Cache *cache, struct CacheSet *set, uint32_t addr, bool loadFromRam)
{
	uint_fast16_t idx = spiRamCachePrvPickVictim(cache);
	struct CacheLine *line = &set->line[idx];
	
//	pr("picked victim way %u currently holding addr 0x%08x, %s\n", idx, line->addr * CACHE_LINE_SIZE, line->dirty ? "DIRTY" : "CLEAN");
	if (line->dirty) {	//clean line
		

//		pr(" flushing -> %08x\n", line->addr * CACHE_LINE_SIZE);
		spiRamPrvCachelineWriteQuadChannel(line->addr * CACHE_LINE_SIZE, line->dataW);
	}
	
	if (loadFromRam) {
//		uint32_t *dst = line->dataW;

		spiRamPrvCachelineReadQuadChannel(addr / CACHE_LINE_SIZE * CACHE_LINE_SIZE, line->dataW);
		
//		pr(" filled %08x -> %08x %08x %08x %08x %08x %08x %08x %08x\n", addr, 
//				dst[0], dst[1], dst[2], dst[3], dst[4], dst[5], dst[6], dst[7]);
	}
	line->dirty = 0;
	line->addr = addr / CACHE_LINE_SIZE;
	
	return line;
}

#if CACHE_NUM_SETS == 24
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0xaaab				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #20					\n\t"	\
        "	movs  r3, #24					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 22
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0xba2f				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #20					\n\t"	\
        "	movs  r3, #22					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 20
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0xcccd				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #20					\n\t"	\
        "	movs  r3, #20					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 18
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0xe38f				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #20					\n\t"	\
        "	movs  r3, #18					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 16
    #define HASH_N_STUFF							\
        "   lsls  r4, #28					\n\t"	\
        "   lsrs  r4, #28					\n\t"
#elif CACHE_NUM_SETS == 15
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0x8889				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #19					\n\t"	\
        "	movs  r3, #15					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 13
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0x9d8a				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #19					\n\t"	\
        "	movs  r3, #13					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 12
    #define HASH_N_STUFF							\
        "	uxth  r4, r4					\n\t"	\
        "	ldr   r5, =0xaaab				\n\t"	\
        "	muls  r5, r4					\n\t"	\
        "	lsrs  r5, #19					\n\t"	\
        "	movs  r3, #12					\n\t"	\
        "	muls  r3, r5					\n\t"	\
        "	subs  r4, r3					\n\t"
#elif CACHE_NUM_SETS == 8
    #define HASH_N_STUFF							\
        "   lsls  r4, #29					\n\t"	\
        "   lsrs  r4, #28					\n\t"
#else

    #error "bad setting for fast path"
#endif


void spiRamPrvCachelineWriteQuadChannel(uint32_t addr, void* src) {
    accessPSRAM(addr, CACHE_LINE_SIZE, true, src);
}
void spiRamPrvCachelineReadQuadChannel(uint32_t addr, void* dst) {
    accessPSRAM(addr, CACHE_LINE_SIZE, false, dst);
}


void __attribute__((naked, noinline)) cacheRead(uint32_t addr, void *dataP, uint_fast16_t sz)
{
    asm volatile(
        ".syntax unified					\n\t"
        "	push  {r4, r5}					\n\t"
        "	lsrs  r4, r0, %2				\n\t"	//r4 = addr / CACHE_LINE_SIZE
        
        HASH_N_STUFF
        
        "	ldr   r3, =%0					\n\t"
        "	ldr   r5, =%3 * 2 + 4 * 2		\n\t"	//size of each set
        "	muls  r4, r5					\n\t"
        "	adds  r4, r3					\n\t"	//points to proper set
        "	ldmia r4!, {r5}					\n\t"	//get info[0], increment past it
        "	lsrs  r3, r0, %2				\n\t"	//addr to compare to
        "	lsrs  r5, #1					\n\t"	//hide dirty bit
        "	cmp   r3, r5					\n\t"
        "	beq   1f						\n\t"
        "	ldr   r5, [r4, %3]				\n\t"	//get info [1]
        "	lsrs  r5, #1					\n\t"	//hide dirty bit
        "	cmp   r3, r5					\n\t"
        "	bne   2f						\n\t"	//not found
        //found in set 2
        "	adds  r4, %3 + 4				\n\t"
        "1:									\n\t"	//common path for "found"
        "	lsls  r0, %4					\n\t"
        "	lsrs  r0, %4					\n\t"
        "	cmp   r2, #4					\n\t"
        "	bne   5f						\n\t"
        "	ldr   r5, [r4, r0]				\n\t"
        "	str   r5, [r1]					\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #1					\n\t"
        "	bne   5f						\n\t"
        "	ldrb  r5, [r4, r0]				\n\t"
        "	strb  r5, [r1]					\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #2					\n\t"
        "	bne   5f						\n\t"
        "	ldrh  r5, [r4, r0]				\n\t"
        "	strh  r5, [r1]					\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #32					\n\t"
        "	bne   5f						\n\t"
        "	adds  r4, r0					\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #64					\n\t"
        "	bne   5f						\n\t"
        "	adds  r4, r0					\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r4!, {r0, r2, r3, r5}		\n\t"
        "	stmia r1!, {r0, r2, r3, r5}		\n\t"
        "	b     4f						\n\t"

        "5: 								\n\t"		//not supported - use slow path
        "	pop   {r4, r5}					\n\t"
        "	b     3f						\n\t"
        
        "4:									\n\t"
        "	pop   {r4, r5}					\n\t"
        "	bx    lr						\n\t"
        "2:									\n\t"	//not found
        "	pop   {r4, r5}					\n\t"
        "	cmp   r2, %3					\n\t"
        "	bne   3f						\n\t"	//slow path
        //direct read path					\n\t"
        "	ldr   r3, =%1					\n\t"
        "	bx    r3						\n\t"
        "3:									\n\t"	//slow
        "	ldr   r3, =spiRamRead_slowpath	\n\t"
        "	bx    r3						\n\t"
        :
        :"i"(mCache.set), "i"(&spiRamPrvCachelineReadQuadChannel), "i"(CACHE_LINE_SIZE_ORDER), "i"(CACHE_LINE_SIZE), "i"(32 - CACHE_LINE_SIZE_ORDER)
    );
}




void __attribute__((used)) spiRamRead_slowpath(uint32_t addr, void *dataP, uint_fast16_t sz)
{
    struct Cache *cache = &mCache;
    struct CacheSet *set = &cache->set[spiRamCachePrvHash(addr)];
    uint32_t *dptr, *dst = (uint32_t*)dataP, dummy1, dummy2;
    struct CacheLine *line; 

//	pr("slow read %u @ 0x%08x -> set %u\n", sz, addr, set - &cache->set[0]);
    
//	pr("fill\n", sz, addr);
    line = spiRamCachePrvFillLine(cache, set, addr, true);

    
    switch (sz) {
        case 1:
            *(uint8_t*)dataP = line->dataB[(addr % CACHE_LINE_SIZE) / sizeof(uint8_t)];
//			pr("read %u @ 0x%08x idx %u -> %02x\n", sz, addr, line - &set->line[0], *(uint8_t*)dataP);
            break;
        
        case 2:
            *(uint16_t*)dataP = line->dataH[(addr % CACHE_LINE_SIZE) / sizeof(uint16_t)];
//			pr("read %u @ 0x%08x idx %u -> %04x\n", sz, addr, line - &set->line[0], *(uint16_t*)dataP);
            break;
        
        case 4:
            *(uint32_t*)dataP = line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
//			pr("read %u @ 0x%08x idx %u -> %08x\n", sz, addr, line - &set->line[0], *(uint32_t*)dataP);
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
//			pr("read %u @ 0x%08x idx %u -> %08x %08x\n", sz, addr, line - &set->line[0], 
//				dst[0], dst[1]);
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
//			pr("read %u @ 0x%08x idx %u -> %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				dst[0], dst[1], dst[2], dst[3]);
            break;

#if CACHE_LINE_SIZE >= 32
        case 32:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(dptr), "1"(dst)
                :"memory", "r0", "r1", "r2", "r3"
            );
//			pr("read %u @ 0x%08x idx %u -> %08x %08x %08x %08x %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				dst[0], dst[1], dst[2], dst[3], dst[4], dst[5], dst[6], dst[7]);
            break;
#endif
#if CACHE_LINE_SIZE >= 64
        case 64:
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(dptr), "1"(dst)
                :"memory", "r0", "r1", "r2", "r3"
            );
//			pr("read %u @ 0x%08x idx %u -> %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				dst[0], dst[1], dst[2], dst[3], dst[4], dst[5], dst[6], dst[7],
//				dst[8], dst[9], dst[10], dst[11], dst[12], dst[13], dst[14], dst[15]);
            break;
#endif

        default:
            console_printf("unknown size %u\n", sz);
            while(1);
    }
}

void __attribute__((naked, noinline)) cacheWrite(uint32_t addr, const void *dataP, uint_fast16_t sz)
{
    asm volatile(
        ".syntax unified					\n\t"
        "	push  {r4, r5}					\n\t"
        "	lsrs  r4, r0, %2				\n\t"	//r4 = addr / CACHE_LINE_SIZE
        
        HASH_N_STUFF
        
        "	ldr   r3, =%0					\n\t"
        "	ldr   r5, =%3 * 2 + 4 * 2		\n\t"	//size of each set
        "	muls  r4, r5					\n\t"
        "	adds  r4, r3					\n\t"	//points to proper set
        "	ldmia r4!, {r5}					\n\t"	//get info[0], increment past it
        "	lsrs  r3, r0, %2				\n\t"	//addr to compare to
        "	lsrs  r5, #1					\n\t"	//hide dirty bit
        "	cmp   r3, r5					\n\t"
        "	beq   1f						\n\t"
        "	ldr   r5, [r4, %3]				\n\t"	//get info [1]
        "	lsrs  r5, #1					\n\t"	//hide dirty bit
        "	cmp   r3, r5					\n\t"
        "	bne   2f						\n\t"	//not found
        //found in set 2
        "	adds  r4, %3 + 4				\n\t"
        "1:									\n\t"	//common path for "found"
        "	lsls  r0, %4					\n\t"
        "	lsrs  r0, %4					\n\t"
        "	cmp   r2, #4					\n\t"
        "	bne   5f						\n\t"
        "	ldr   r5, [r1]					\n\t"
        "	str   r5, [r4, r0]				\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #1					\n\t"
        "	bne   5f						\n\t"
        "	ldrb  r5, [r1]					\n\t"
        "	strb  r5, [r4, r0]				\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #2					\n\t"
        "	bne   5f						\n\t"
        "	ldrh  r5, [r1]					\n\t"
        "	strh  r5, [r4, r0]				\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #32					\n\t"
        "	bne   5f						\n\t"
        "	mov   r12, r4					\n\t"
        "	adds  r4, r0					\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	mov   r4, r12					\n\t"
        "	b     4f						\n\t"
        "5:									\n\t"
        "	cmp   r2, #64					\n\t"
        "	bne   5f						\n\t"
        "	mov   r12, r4					\n\t"
        "	adds  r4, r0					\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	ldmia r1!, {r0, r2, r3, r5}		\n\t"
        "	stmia r4!, {r0, r2, r3, r5}		\n\t"
        "	mov   r4, r12					\n\t"
        "	b     4f						\n\t"

        "5: 								\n\t"		//not supported - use slow path
        "	pop   {r4, r5}					\n\t"
        "	b     3f						\n\t"
        
        "4:									\n\t"		//mark as dirty
        "	subs  r4, #4					\n\t"
        "	ldr   r0, [r4]					\n\t"
        "	movs  r1, #1					\n\t"
        "	orrs  r0, r1					\n\t"
        "	str   r0, [r4]					\n\t"
        
        "	pop   {r4, r5}					\n\t"
        "	bx    lr						\n\t"
        "2:									\n\t"	//not found
        "	pop   {r4, r5}					\n\t"
        "	cmp   r2, %3					\n\t"
        "	bne   3f						\n\t"	//slow path
        //direct read path					\n\t"
        "	ldr   r3, =%1					\n\t"
        "	bx    r3						\n\t"
        "3:									\n\t"	//slow
        "	ldr   r3, =spiRamWrite_slowpath	\n\t"
        "	bx    r3						\n\t"
        :
        :"i"(mCache.set), "i"(&spiRamPrvCachelineWriteQuadChannel), "i"(CACHE_LINE_SIZE_ORDER), "i"(CACHE_LINE_SIZE), "i"(32 - CACHE_LINE_SIZE_ORDER)
        :"memory", "cc"
    );
}


void __attribute__((used)) spiRamWrite_slowpath(uint32_t addr, const void *dataP, uint_fast16_t sz)
{
    struct Cache *cache = &mCache;
    struct CacheSet *set = &cache->set[spiRamCachePrvHash(addr)];
    const uint32_t *src = (const uint32_t*)dataP;
    uint32_t dummy1, dummy2;
    struct CacheLine *line; 
    uint32_t *dptr;
    
    
//	pr("slow write %u @ 0x%08x -> set %u\n", sz, addr, set - cache->set);
    
    //not found
//	pr("fill\n", sz, addr);
    line = spiRamCachePrvFillLine(cache, set, addr, sz != CACHE_LINE_SIZE);
    
    switch (sz) {
        case 1:
//			pr("write %u @ 0x%08x idx %u <- %02x\n", sz, addr, line - &set->line[0], *(uint8_t*)dataP);
            line->dataB[(addr % CACHE_LINE_SIZE) / sizeof(uint8_t)] = *(uint8_t*)dataP;
            break;
        
        case 2:
//			pr("write %u @ 0x%08x idx %u <- %04x\n", sz, addr, line - &set->line[0], *(uint16_t*)dataP);
            line->dataH[(addr % CACHE_LINE_SIZE) / sizeof(uint16_t)] = *(uint16_t*)dataP;
            break;
        
        case 4:
//			pr("write %u @ 0x%08x idx %u <- %08x\n", sz, addr, line - &set->line[0], *(uint32_t*)dataP);
            line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)] = *(uint32_t*)dataP;
            break;
        
        case 8:
//			pr("write %u @ 0x%08x idx %u <- %08x %08x\n", sz, addr, line - &set->line[0], 
//				src[0], src[1]);
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
//			pr("write %u @ 0x%08x idx %u <- %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				src[0], src[1], src[2], src[3]);
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(src), "1"(dptr)
                :"memory", "r0", "r1", "r2", "r3"
            );
            break;
#if CACHE_LINE_SIZE >= 32
        case 32:
//			pr("write %u @ 0x%08x idx %u <- %08x %08x %08x %08x %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7]);
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(src), "1"(dptr)
                :"memory", "r0", "r1", "r2", "r3"
            );
            break;
#endif
#if CACHE_LINE_SIZE >= 64
        case 64:
//			pr("write %u @ 0x%08x idx %u <- %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", sz, addr, line - &set->line[0], 
//				src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7],
//				src[8], src[9], src[10], src[11], src[12], src[13], src[14], src[15]);
            dptr = &line->dataW[(addr % CACHE_LINE_SIZE) / sizeof(uint32_t)];
            asm volatile(
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                "	ldmia %0!, {r0, r1, r2, r3}	\n"
                "	stmia %1!, {r0, r1, r2, r3}	\n"
                :"=l"(dummy1), "=l"(dummy2)
                :"0"(src), "1"(dptr)
                :"memory", "r0", "r1", "r2", "r3"
            );
            break;
#endif
        default:
            console_printf("unknown size %u\n", sz);
            while(1);
    }
    line->dirty = 1;
}

#else

/*
 * Copyright (c) 2023, Jisheng Zhang <jszhang@kernel.org>. All rights reserved.
 *
 * Modified by Vlad Tomoiaga (tvlad1234)
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include "cache.h"
#include "psram.h"

#define psram_write(handle, ofs, p, sz) accessPSRAM(ofs, sz, true, p)
#define psram_read(handle, ofs, p, sz) accessPSRAM(ofs, sz, false, p)

struct cacheline
{
	uint8_t data[64];
};

static uint64_t accessed, hit;
static uint32_t tags[4096 / 64 / 2][2];
static struct cacheline cachelines[4096 / 64 / 2][2];

/*
 * bit[0]: valid
 * bit[1]: dirty
 * bit[2]: for LRU
 * bit[3:10]: reserved
 * bit[11:31]: tag
 */
#define VALID (1 << 0)
#define DIRTY (1 << 1)
#define LRU (1 << 2)
#define LRU_SFT 2
#define TAG_MSK 0xfffff800

/*
 * bit[0: 5]: offset
 * bit[6: 10]: index
 * bit[11: 31]: tag
 */
static inline int get_index(uint32_t addr)
{
	return (addr >> 6) & 0x1f;
}

void cache_write(uint32_t ofs, void *buf, uint32_t size)
{
	// if (((ofs | (64 - 1)) != ((ofs + size - 1) | (64 - 1))))
	//	printf("write cross boundary\n");

	int ti, i, index = get_index(ofs);
	uint32_t *tp;
	uint8_t *p;

	++accessed;

	for (i = 0; i < 2; i++)
	{
		tp = &tags[index][i];
		p = cachelines[index][i].data;
		if (*tp & VALID)
		{
			if ((*tp & TAG_MSK) == (ofs & TAG_MSK))
			{
				++hit;
				ti = i;
				break;
			}
			else
			{
				if (i != 1)
					continue;

				ti = 1 - ((*tp & LRU) >> LRU_SFT);
				tp = &tags[index][ti];
				p = cachelines[index][ti].data;

				if (*tp & DIRTY)
				{
					psram_write(handle, *tp & ~0x3f, p, 64);
				}
				psram_read(handle, ofs & ~0x3f, p, 64);
				*tp = ofs & ~0x3f;
				*tp |= VALID;
			}
		}
		else
		{
			if (i != 1)
				continue;

			ti = i;
			psram_read(handle, ofs & ~0x3f, p, 64);
			*tp = ofs & ~0x3f;
			*tp |= VALID;
		}
	}

	tags[index][1] &= ~(LRU);
	tags[index][1] |= (ti << LRU_SFT);
	memcpy(p + (ofs & 0x3f), buf, size);
	*tp |= DIRTY;
}

void cache_read(uint32_t ofs, void *buf, uint32_t size)
{
	// if (((ofs | (64 - 1)) != ((ofs + size - 1) | (64 - 1))))
	//	printf("read cross boundary\n");

	int ti, i, index = get_index(ofs);
	uint32_t *tp;
	uint8_t *p;

	++accessed;

	for (i = 0; i < 2; i++)
	{
		tp = &tags[index][i];
		p = cachelines[index][i].data;
		if (*tp & VALID)
		{
			if ((*tp & TAG_MSK) == (ofs & TAG_MSK))
			{
				++hit;
				ti = i;
				break;
			}
			else
			{
				if (i != 1)
					continue;

				ti = 1 - ((*tp & LRU) >> LRU_SFT);
				tp = &tags[index][ti];
				p = cachelines[index][ti].data;

				if (*tp & DIRTY)
				{
					psram_write(handle, *tp & ~0x3f, p, 64);
				}
				psram_read(handle, ofs & ~0x3f, p, 64);
				*tp = ofs & ~0x3f;
				*tp |= VALID;
			}
		}
		else
		{
			if (i != 1)
				continue;

			ti = i;
			psram_read(handle, ofs & ~0x3f, p, 64);
			*tp = ofs & ~0x3f;
			*tp |= VALID;
		}
	}

	tags[index][1] &= ~(LRU);
	tags[index][1] |= (ti << LRU_SFT);
	memcpy(buf, p + (ofs & 0x3f), size);
}

void cache_get_stat(uint64_t *phit, uint64_t *paccessed)
{
	*phit = hit;
	*paccessed = accessed;
}

#endif