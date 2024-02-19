#include <hardware/gpio.h>
#include "../config/umips_config.h"

void link_gpio_put(int pin, int value)
{
    gpio_put(pin, value);
}

// void psramCSEN() {
//     console_printf("EN\r\n");
//     gpio_put(PSRAM_SPI_PIN_CK, 1);
// }
// void psramCSCLS() {
//     console_printf("CLS\r\n");
//     gpio_put(PSRAM_SPI_PIN_CK, 0);
// }