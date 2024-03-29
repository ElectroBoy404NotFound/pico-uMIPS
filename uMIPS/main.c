#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "ff.h"
#include "hw_config.h"

#include "psram/psram.h"

#include "console/console.h"

#include "hardware/pll.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

#include "emulator/soc/soc.h"
#include "emulator/rtc/ds1287.h"

extern bool globalReadyToTickRTC;

void core1_entry();

void gset_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2)
{
    if (!running_on_fpga())
    {
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        48 * MHZ,
                        48 * MHZ);

        pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
        uint32_t freq = vco_freq / (post_div1 * post_div2);

        // Configure clocks
        // CLK_REF = XOSC (12MHz) / 1 = 12MHz
        clock_configure(clk_ref,
                        CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                        0, // No aux mux
                        12 * MHZ,
                        12 * MHZ);

        // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        freq, freq);

        clock_configure(clk_peri,
                        0,
                        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
                        freq,
                        freq);
    }
}

static inline bool gset_sys_clock_khz(uint32_t freq_khz, bool required)
{
    uint vco, postdiv1, postdiv2;
    if (check_sys_clock_khz(freq_khz, &vco, &postdiv1, &postdiv2))
    {
        gset_sys_clock_pll(vco, postdiv1, postdiv2);
        return true;
    }
    else if (required)
    {
        panic("System clock of %u kHz cannot be exactly achieved", freq_khz);
    }
    return false;
}

bool tickRTC(struct repeating_timer *t) {
    if(globalReadyToTickRTC) ds1287step(1);
    return true;
}

int main()
{
    sleep_ms(1000);
    vreg_set_voltage(VREG_VOLTAGE_MAX); // overvolt the core just a bit
    sleep_ms(50);
    gset_sys_clock_khz(400000, true); // overclock to 400 MHz (from 125MHz)
    sleep_ms(50);
    console_init();

    // console_printf("\x1B[J");

    multicore_reset_core1();
    multicore_fifo_drain();
    multicore_launch_core1(core1_entry);

    struct repeating_timer timer;
    add_repeating_timer_us(-122, tickRTC, NULL, &timer);

    while (true) {
        console_task();
    }
}

void core1_entry()
{
    int r = initPSRAM();
    if (r < 1)
        console_panic_uart("Error initalizing PSRAM (%d)!\n\r", r);

    console_printf_uart("\x1b[32mPSRAM init OK!\n\r");
    console_printf_uart("\x1b[32mPSRAM Baud: %d\n\r", r);

    startEmu();
}
