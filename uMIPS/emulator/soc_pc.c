/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "hypercall.h"
#include "decBus.h"
#include "ds1287.h"
#include "printf.h"
#include "dz11.h"
#include "soc.h"
#include "cpu.h"
#include "mem.h"

static MassStorageF gDiskF;
static uint8_t gRam[RAM_AMOUNT];
static uint8_t gRom[256*1024];


static bool accessRamRom(uint32_t pa, uint_fast8_t size, bool write, void* buf, void* isRam)
{
	//XXX: endianness
	uint8_t *mem;

	if (isRam) {
		pa -= RAM_BASE;
		mem = gRam;
	}
	else{
		pa -= (ROM_BASE & 0x1FFFFFFFUL);
		mem = gRom;
	}

	if (write) {
		if (size == 4)
			*(uint32_t*)(mem + pa) = *(uint32_t*)buf;
		else if (size == 1)
			mem[pa] = *(uint8_t*)buf;
		else if (size == 2)
			*(uint16_t*)(mem + pa) = *(uint16_t*)buf;
		else if (size == 8)
			memcpy(mem + pa, buf, size);
	}
	else {
		if (size == 4)
			*(uint32_t*)buf = *(uint32_t*)(mem + pa);
		else if (size == 1)
			*(uint8_t*)buf = mem[pa];
		else if (size == 2)
			*(uint16_t*)buf = *(uint16_t*)(mem + pa);
		else
			memcpy(buf, mem + pa, size);
	}
	
	return true;
}

bool cpuExtHypercall(void)	//call type in $at, params in $a0..$a3, return in $v0, if any
{
	uint32_t hyperNum = cpuGetRegExternal(MIPS_REG_AT), t;
	uint32_t blk, pa;
	uint8_t chr;
	bool ret;

	switch (hyperNum) {
		case H_GET_MEM_MAP:
			//a0 is byte index index if >= 2, [0] is nBits, [1] is eachBitSz
			switch (cpuGetRegExternal(MIPS_REG_A0)) {
				case 0:
					pa = 1;
					break;
				
				case 1:
					pa = RAM_AMOUNT;
					break;
				
				case 2:
					pa = 0x01;	//that one bit :D
					break;
				
				default:
					pa = 0;
					break;
			}
			cpuSetRegExternal(MIPS_REG_V0, pa);
			break;
		
		case H_CONSOLE_WRITE:
			chr = cpuGetRegExternal(MIPS_REG_A0);
			if (chr == '\n')
				fputc('\r', stderr);
			fputc(chr, stderr);
			break;
		
		case H_STOR_GET_SZ:
			if (!gDiskF(MASS_STORE_OP_GET_SZ, 0, &t))
				return false;
			cpuSetRegExternal(MIPS_REG_V0, t);
			break;
		
		case H_STOR_READ:
			blk = cpuGetRegExternal(MIPS_REG_A0);
			pa = cpuGetRegExternal(MIPS_REG_A1);
			ret = pa < RAM_AMOUNT && RAM_AMOUNT - pa >= 512 && gDiskF(MASS_STORE_OP_READ, blk, gRam + pa);
			cpuSetRegExternal(MIPS_REG_V0, ret);
	//		fprintf(stderr, " rd_block(%u, 0x%08x) -> %d\r\n", blk, pa, ret);
		
			break;
		
		case H_STOR_WRITE:
			blk = cpuGetRegExternal(MIPS_REG_A0);
			pa = cpuGetRegExternal(MIPS_REG_A1);
			ret = pa < RAM_AMOUNT && RAM_AMOUNT - pa >= 512 && gDiskF(MASS_STORE_OP_WRITE, blk, gRam + pa);
			cpuSetRegExternal(MIPS_REG_V0, ret);
	//		fprintf(stderr, " wr_block(%u, 0x%08x) -> %d\r\n", blk, pa, ret);
			break;
		
		case H_TERM:
			exit(0);
			break;
		
		default:
			err_str("hypercall %u @ 0x%08x\n", hyperNum, cpuGetRegExternal(MIPS_EXT_REG_PC));
			return false;
	}
	return true;
}

#if CDROM_SUPORTED

	static struct ScsiDisk gCDROM;
	
	static bool cdromStorageAccess(uint8_t op, uint32_t sector, void *buf)
	{
		const uint32_t blockSz = 512;
		
		static FILE *f;
		
		if (!f) {
			const char *cdpath = "../ref/ultrix/ultrix-risc-4.5-mode1.ufs";
			f = fopen(cdpath, "rb");
			if (!f) {
				fprintf(stderr, "cannot open cdrom file '%s'\n", cdpath);
				exit(-6);
			}
		}
		
		switch (op) {
			case MASS_STORE_OP_GET_SZ:
				fseeko64(f, 0, SEEK_END);
				 *(uint32_t*)buf = (off64_t)ftello64(f) / (off64_t)blockSz;
				 return true;
			case MASS_STORE_OP_READ:
				fseeko64(f, (off64_t)sector * (off64_t)blockSz, SEEK_SET);
				return fread(buf, 1, blockSz, f) == blockSz;
			case MASS_STORE_OP_WRITE:
				return false;
		}
		return false;
	}

#endif

static bool accessRam(uint32_t pa, uint_fast8_t size, bool write, void* buf)
{
	return accessRamRom(pa, size, write, buf, (void*)1);
}

static bool accessRom(uint32_t pa, uint_fast8_t size, bool write, void* buf)
{
	return accessRamRom(pa, size, write, buf, (void*)0);
}

bool socInit(MassStorageF diskF)
{
	uint_fast8_t i;
	
	gDiskF = diskF;
	
	if (!memRegionAdd(RAM_BASE, sizeof(gRam), accessRam))
		return false;
	
	if (!memRegionAdd(ROM_BASE & 0x1FFFFFFFUL, sizeof(gRom), accessRom))
		return false;
	
	if (!decBusInit())
		return false;
	
	if (!dz11init())
		return false;
	
	if (!ds1287init())
		return false;
	
	cpuInit(RAM_AMOUNT);
	
	return true;
}

static bool singleStep = false;
	
void socStop(void)
{
	singleStep = true;
}

void socRun(int gdbPort)
{
	uint16_t cy = 0;
	
	(void)gdbPort;
	
	while(true) {
		cy++;
		
		#ifdef GDB_SUPPORT
			gdbCmdWait(gdbPort, &singleStep);
		#endif
		
		cpuCycle(RAM_AMOUNT);
		
		if (!(cy & 0x0fff))
			ds1287step(1);
		
		if (!(cy & 0x1fff))
			socInputCheck();
		
	}
}

