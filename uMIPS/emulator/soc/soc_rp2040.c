/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#include <string.h>
#include "../hypercall/hypercall.h"
#include "../bus/decBus.h"
#include "../rtc/ds1287.h"
#include "../dz11/dz11.h"
#include "soc.h"
#include "../memory/mem.h"
#include "../../sd/sd.h"
#include "../../psram/psram.h"
#include "../../console/console.h"
#include <stdio.h>

static uint32_t mRamTop;

static uint8_t mDiskBuf[SD_BLOCK_SIZE];

static uint8_t gRom[]  = {
  0x23, 0x00, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x10, 0x04, 0x00, 0x03, 0x24, 
  0x26, 0x00, 0x00, 0x10, 0x08, 0x00, 0x03, 0x24, 0x24, 0x00, 0x00, 0x10, 0x0c, 0x00, 0x03, 0x24, 
  0x22, 0x00, 0x00, 0x10, 0x10, 0x00, 0x03, 0x24, 0x20, 0x00, 0x00, 0x10, 0x14, 0x00, 0x03, 0x24, 
  0x1e, 0x00, 0x00, 0x10, 0x18, 0x00, 0x03, 0x24, 0x1c, 0x00, 0x00, 0x10, 0x1c, 0x00, 0x03, 0x24, 
  0x1a, 0x00, 0x00, 0x10, 0x20, 0x00, 0x03, 0x24, 0x18, 0x00, 0x00, 0x10, 0x24, 0x00, 0x03, 0x24, 
  0x16, 0x00, 0x00, 0x10, 0x28, 0x00, 0x03, 0x24, 0x14, 0x00, 0x00, 0x10, 0x2c, 0x00, 0x03, 0x24, 
  0x12, 0x00, 0x00, 0x10, 0x30, 0x00, 0x03, 0x24, 0x10, 0x00, 0x00, 0x10, 0x34, 0x00, 0x03, 0x24, 
  0x0e, 0x00, 0x00, 0x10, 0x38, 0x00, 0x03, 0x24, 0x0c, 0x00, 0x00, 0x10, 0x3c, 0x00, 0x03, 0x24, 
  0x0a, 0x00, 0x00, 0x10, 0x40, 0x00, 0x03, 0x24, 0x08, 0x00, 0x00, 0x10, 0x44, 0x00, 0x03, 0x24, 
  0x25, 0x28, 0x00, 0x00, 0x03, 0x00, 0x01, 0x24, 0x76, 0x67, 0x64, 0x4f, 0xff, 0xff, 0x40, 0x10, 
  0x00, 0x80, 0x10, 0x3c, 0x08, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x3c, 
  0x08, 0x10, 0x21, 0x24, 0x08, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0xe8, 0xff, 0x00, 0x10, 0x84, 0x00, 0x03, 0x24
};

uint64_t getTimeMillis() {
	return to_ms_since_boot(get_absolute_time());
}

void delayMsec(uint32_t msec)
{
	uint64_t till = getTimeMillis() + (uint64_t)msec * (TICKS_PER_SECOND / 1000);
	
	while (getTimeMillis() < till);
}


void prPutchar(char chr)
{	
    console_putc(chr);
}

void dz11charPut(uint8_t line, uint8_t chr)
{		
	if (line == 3) {
        console_putc(chr);
		/*	--	for benchmarking
		static uint8_t state = 0;
		
		switch (state) {
			case 0:
				if (chr == 'S')
					state = 1;
				break;
			case 1:
				state = (chr == 'C') ? 2 : 0;
				break;
			
			case 2:
				state = (chr == 'S') ? 3 : 1;
				break;
		
			case 3:
				state = (chr == 'I') ? 4 : 0;
				break;
			
			case 4:
				;
				uint64_t time = getTime();
				pr("\ntook 0x%08x%08x\n", (uint32_t)(time >> 32), (uint32_t)time);
				state = 5;
				break;
		}
		
		//*/
	//	prPutchar(chr);
	}
}


static bool massStorageAccess(uint8_t op, uint32_t sector, void *buf)
{
    switch (op) {
        case MASS_STORE_OP_GET_SZ:
            *(uint32_t*)buf = 16777216;
			console_printf("\33[33mNumber of sectors requested. Assuming each sector is 512 bytes and file can go to 8GB.\33[m\n");
            return true;
        
        case MASS_STORE_OP_READ:
            sdSecRead(sector, buf);
            return true;
        case MASS_STORE_OP_WRITE:
			sdSecWrite(sector, buf);
			return true;
    }
    return false;
}

static bool accessRom(uint32_t pa, uint8_t size, bool write, void* buf)
{
	const uint8_t *mem = gRom;

	pa -= (EMU_ROM_BASE & 0x1FFFFFFFUL);
	
	if (write)
		return false;
	else if (size == 4)
		*(uint32_t*)buf = *(uint32_t*)(mem + pa);
	else if (size == 1)
		*(uint8_t*)buf = mem[pa];
	else if (size == 2)
		*(uint16_t*)buf = *(uint16_t*)(mem + pa);
	else
		memcpy(buf, mem + pa, size);
	
	return true;
}

bool accessRam(uint32_t pa, uint8_t size, bool write, void* buf)
{	
	accessPSRAM(pa, size, write, buf);
	return true;
}

bool cpuExtHypercall(void)	//call type in $at, params in $a0..$a3, return in $v0, if any
{
	uint32_t hyperNum = cpuGetRegExternal(MIPS_REG_AT), t,  ramMapNumBits, ramMapEachBitSz;
	uint16_t ofst;
	uint32_t blk, pa;
	uint8_t chr;
	bool ret;

	switch (hyperNum) {
		case H_GET_MEM_MAP:		//stays for booting older images which expect this
		
			switch (pa = cpuGetRegExternal(MIPS_REG_A0)) {
				case 0:
					pa = 1;
					break;
				
				case 1:
					pa = EMULATOR_RAM_MB * 1024 * 1024;
					break;
				
				case 2:
					pa = 1;
					break;
				
				default:
					pa = 0;
					break;
			}
			cpuSetRegExternal(MIPS_REG_V0, pa);
			break;
		
		case H_CONSOLE_WRITE:
			chr = cpuGetRegExternal(MIPS_REG_A0);
			console_putc(chr);
			break;
		
		case H_STOR_GET_SZ:
			if (!massStorageAccess(MASS_STORE_OP_GET_SZ, 0, &t))
				return false;
			cpuSetRegExternal(MIPS_REG_V0, t);
			break;
		
		case H_STOR_READ:
			blk = cpuGetRegExternal(MIPS_REG_A0);
			pa = cpuGetRegExternal(MIPS_REG_A1);
			ret = massStorageAccess(MASS_STORE_OP_READ, blk, mDiskBuf);
			for (ofst = 0; ofst < SD_BLOCK_SIZE; ofst += 64)
				accessPSRAM(pa + ofst, 64, true, mDiskBuf + ofst);
			cpuSetRegExternal(MIPS_REG_V0, ret);
			if (!ret) {
				console_panic(" rd_block(%u, 0x%08x) -> %d\n", blk, pa, ret);
			}
			break;
		
		case H_STOR_WRITE:
			blk = cpuGetRegExternal(MIPS_REG_A0);
			pa = cpuGetRegExternal(MIPS_REG_A1);
			for (ofst = 0; ofst < SD_BLOCK_SIZE; ofst += 64)
				accessPSRAM(pa + ofst, 64, false, mDiskBuf + ofst);
			ret = massStorageAccess(MASS_STORE_OP_WRITE, blk, mDiskBuf);
			cpuSetRegExternal(MIPS_REG_V0, ret);
			if (!ret) {
				console_panic(" wr_block(%u, 0x%08x) -> %d\n", blk, pa, ret);
			}
			break;
		
		case H_TERM:
			console_panic("termination requested\n");
		    break;

		default:
			console_printf("hypercall %u @ 0x%08x\n", hyperNum, cpuGetRegExternal(MIPS_EXT_REG_PC));
			return false;
	}
	return true;
}

void startEmu(void)
{
	uint32_t ramAmt = EMULATOR_RAM_MB * 1024 * 1024;
	
	console_printf("\33[m");

    if (!memRegionAdd(EMU_RAM_BASE, ramAmt, accessRam))
        console_panic("failed to init %s\n", "RAM");
    if (!memRegionAdd(EMU_ROM_BASE & 0x1FFFFFFFUL, sizeof(gRom), accessRom))
        console_panic("failed to init %s\n", "ROM");
    if (!decBusInit())
        console_panic("failed to init %s\n", "DEC BUS");
    if (!dz11init())
        console_panic("failed to init %s\n", "DZ11");
    if (!ds1287init())
        console_panic("failed to init %s\n", "DS1287");
    if(!sdCardInit())
        console_panic("failed to init SD card\n");

    console_printf("uMIPS v2.2.0 (BL ver ");
    console_printf("%d", *(volatile uint8_t*)8 - 0x10);
    console_printf(")\r\nwill run with ");
	console_printf(PSRAM_TWO_CHIPS ? "2" : PSRAM_THREE_CHIPS ? "3" : "4");
	console_printf(" PSRAM Chips");
    
    cpuInit(ramAmt);
	uint64_t lastMilli = getTimeMillis();
    while(1) {
        cpuCycle(ramAmt);
		
		if ((getTimeMillis() - lastMilli) >= 100) {
			ds1287step((getTimeMillis() - lastMilli) * 10000);
			lastMilli = getTimeMillis();
		}
    }

    console_printf("CPU exited!");

    while(1);
}


