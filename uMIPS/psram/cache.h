// /*
// 	(c) 2021 Dmitry Grinberg   https://dmitry.gr
// 	Non-commercial use only OR licensing@dmitry.gr
// */

// #ifndef _SPI_RAM_H_
// #define _SPI_RAM_H_


// #include <stdbool.h>
// #include <stdint.h>


// void cacheInit(void);

// //crossing chip boundary is not permitted AND not checked for. Crossing 1K coundary is not permitted and not checked for. Enjoy...
// void cacheRead(uint32_t addr, void *data, uint_fast16_t sz);
// void cacheWrite(uint32_t addr, const void *data, uint_fast16_t sz);



// #endif

/*
 * Copyright (c) 2023, Jisheng Zhang <jszhang@kernel.org>. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

void cache_write(uint32_t ofs, void *buf, uint32_t size);
void cache_read(uint32_t ofs, void *buf, uint32_t size);
void cache_get_stat(uint64_t *phit, uint64_t *paccessed);

#endif /* CACHE_H */