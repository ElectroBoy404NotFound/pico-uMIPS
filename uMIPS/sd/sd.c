#pragma GCC optimize ("Os")

#include "sd.h"

#include "ff.h"
#include "f_util.h"
#include "diskio.h"
#include "sd_card.h"
#include "../console/console.h"

// FIL file;
sd_card_t *p_sd;

bool sdCardInit() {
    bool rc = sd_init_driver();
    if (!rc) return RES_NOTRDY;

    p_sd = sd_get_by_num(0);
    if (!p_sd) return RES_PARERR;

    p_sd->init(p_sd);

    return true;

    // sd_card_t *pSD0 = sd_get_by_num(0);
    // FRESULT fr = f_mount(&pSD0->fatfs, pSD0->pcName, 1);
    // if (FR_OK != fr)
    //     console_panic("SD mount error: %s (%d)\n\r", FRESULT_str(fr), fr);

    // fr = f_open(&file, "0:LinuxDiskImage.disk", FA_READ | FA_WRITE);

    // return FR_OK == fr;
}

bool sdSecRead(uint32_t sec, void *dst) {
    // f_lseek(&file, sec * SD_BLOCK_SIZE);
    // f_read(&file, dst, SD_BLOCK_SIZE, NULL);
    p_sd->read_blocks(p_sd, dst, sec, 1);
    return true;
}

bool sdSecWrite(uint32_t sec, void *src) {
    // f_lseek(&file, sec * SD_BLOCK_SIZE);
    // f_write(&file, src, SD_BLOCK_SIZE, NULL);
    p_sd->write_blocks(p_sd, src, sec, 1);
    return true;
}

uint64_t sdNoSectors() {
    return p_sd->get_num_sectors(p_sd);
}