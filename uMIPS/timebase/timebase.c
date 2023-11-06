/*
	(c) 2021 Dmitry Grinberg   https://dmitry.gr
	Non-commercial use only OR licensing@dmitry.gr
*/

#include "pico/stdlib.h"
#include "hardware/structs/systick.h"

static uint64_t mTicks = 0;

#ifndef TICKS_PER_IRQ
#define TICKS_PER_IRQ 0x01000000
#endif

void timebaseInit(void)
{
    // systick_hw->csr = 0;  // Disable SysTick
    // systick_hw->rvr = TICKS_PER_IRQ - 1;  // Set the reload value
    // systick_hw->cvr = 0;  // Clear the current value
    // systick_hw->csr = (1 << 2) | (1 << 1) | (1 << 0);  // Enable SysTick, interrupt, and use the core clock

    // // Set the interrupt priority
    // irq_set_priority(-1, 1);
}

void SysTick_Handler(void)
{
    // mTicks += TICKS_PER_IRQ;
}

uint64_t getTime(void)
{
    // uint64_t hi1, hi2;
    // uint32_t lo;

    // do {
    //     hi1 = mTicks;
    //     asm volatile("" ::: "memory");
    //     hi2 = mTicks;
    //     asm volatile("" ::: "memory");
    //     lo = systick_hw->cvr;
    //     asm volatile("" ::: "memory");
    // } while (hi1 != hi2 || hi1 != mTicks);

    // return hi1 + (TICKS_PER_IRQ - lo);
    return -1;
}