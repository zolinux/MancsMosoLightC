#include "blinker.h"
#include "common.h"
#include "gpio.h"
#include "motorControl.h"
#include "states.h"
#include "timer.h"

#include <msp430.h>

int main(void)
{
    Context *ctx = context();
    __disable_interrupt();
    // stop WDT
    timer_stop();
    context_init();

    // configure 32khz clock
    BCSCTL2 = SELM_3 | DIVM_0 | SELS | DIVS_0;
    __enable_interrupt();
    timer_init();
    timer_setInterval(MS250);

    gpio_clear(&ctx->led);
    gpio_clear(&ctx->ledErr);
    gpio_setInterruptEnabledEdge(&ctx->swOn, true);
    gpio_setInterruptEnabled(&ctx->swOn, true);
    gpio_setInterruptEnabledEdge(&ctx->swFunc, true);
    gpio_setInterruptEnabled(&ctx->swFunc, true);
    gpio_setInterruptEnabledEdge(&ctx->irIn, false);

    __low_power_mode_3();
    return 0;
}

__interrupt_vec(PORT1_VECTOR) void port1_ISR(void)
{
    state_gpio();
}
__interrupt_vec(PORT2_VECTOR) void port2_ISR(void)
{
    state_gpio();
}
__interrupt_vec(ADC10_VECTOR) void adc_ISR(void)
{
}
__interrupt_vec(NMI_VECTOR) void nmi_ISR(void)
{
}
__interrupt_vec(TIMERA0_VECTOR) void ta0_ISR(void)
{
}
__interrupt_vec(TIMERA1_VECTOR) void ta1_ISR(void)
{
}
__interrupt_vec(WDT_VECTOR) void wdt_ISR(void)
{
    Context *ctx = context();

    timer_tick();
    state_tick();
    blinker_tick(&ctx->ledBlink);
    blinker_tick(&ctx->ledErrBlink);
    motor_tick();
}
