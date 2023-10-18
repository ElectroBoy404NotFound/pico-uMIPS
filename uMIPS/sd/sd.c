#include "sd.h"

#include "ff.h"
#include "f_util.h"
#include "diskio.h"
#include "sd_card.h"
#include "../console/console.h"

FIL file;

bool sdCardInit() {
    // bool rc = sd_init_driver();
    // if (!rc) return RES_NOTRDY;

    // sd_card_t *p_sd = sd_get_by_num(0);
    // if (!p_sd) return RES_PARERR;

    // return rc;

    sd_card_t *pSD0 = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD0->fatfs, pSD0->pcName, 1);
    if (FR_OK != fr)
        console_panic("SD mount error: %s (%d)\n\r", FRESULT_str(fr), fr);

    fr = f_open(&file, "0:linux.busybox", FA_READ | FA_WRITE);

    return FR_OK == fr;
}

bool sdSecRead(uint32_t sec, void *dst) {
    f_lseek(&file, sec * SD_BLOCK_SIZE);
    f_read(&file, dst, SD_BLOCK_SIZE, NULL);
    return true;
}

bool sdSecWrite(uint32_t sec, void *src) {
    f_lseek(&file, sec * SD_BLOCK_SIZE);
    f_write(&file, src, SD_BLOCK_SIZE, NULL);
    return true;
}