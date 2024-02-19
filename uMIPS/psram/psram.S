#include "../config/umips_config.h"
#pragma GCC optimize ("Os")

.thumb_func
.global spi_pulse_sck_asm

.align 4

spi_pulse_sck_asm:
push {lr} 
mov R0, #PSRAM_SPI_PIN_CK
@movs r3, #1
@lsl r3, r0
@ldr r2, =0xd0000014
@ldr r2, [r2]
@str r3, [r2]

mov r1, #1
BL link_gpio_put

@bl psramCSEN

nop

mov R0, #PSRAM_SPI_PIN_CK
@movs r3, #1
@lsl r3, r0 @ shift over to pin position
@ldr r2, =0xd0000018 @ address we want
@ldr r2, [r2]
@str r3, [r2]

mov r1, #0
BL link_gpio_put

@bl psramCSCLS

nop

pop {pc}

bx lr
