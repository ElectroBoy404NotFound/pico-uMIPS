/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#ifndef _SD_H_
#define _SD_H_

#include <stdbool.h>
#include <stdint.h>

#define SD_BLOCK_SIZE		512

bool sdCardInit();
uint64_t sdNoSectors();

bool sdSecRead(uint32_t sec, void *dst);
bool sdSecWrite(uint32_t sec, void *src);

#endif
