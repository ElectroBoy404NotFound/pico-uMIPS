#pragma GCC optimize ("Ofast")

#include "sd.h"

#include "ff.h"
#include "f_util.h"
#include "diskio.h"
#include "sd_card.h"
#include "../console/console.h"
#include "hw_config.h"

sd_card_t *p_sd;

bool sdCardInit() {
    bool rc = sd_init_driver();
    if (!rc) return RES_NOTRDY;

    p_sd = sd_get_by_num(0);
    if (!p_sd) return RES_PARERR;

    return p_sd->init(p_sd) == 0;
}

bool sdSecRead(uint32_t sec, void *dst) {
    p_sd->read_blocks(p_sd, dst, sec, 1);
    return true;
}

bool sdSecWrite(uint32_t sec, void *src) {
    p_sd->write_blocks(p_sd, src, sec, 1);
    return true;
}

uint64_t sdNoSectors() {
    return p_sd->get_num_sectors(p_sd);
}