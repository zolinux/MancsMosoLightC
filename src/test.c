#include "blinker.h"
#include "common.h"
#include "gpio.h"
#include "motorControl.h"
#include "timer.h"

#include <msp430.h>

static volatile bool timeElapsed = false;
static volatile uint32_t swOnPressTime = 0;

int main(void)
{
    Context *ctx = context();
    _disable_interrupts();

    // stop WDT
    timer_stop();

    context_init();

    // configure 32khz clock
    BCSCTL2 = SELM_3 | DIVM_0 | SELS | DIVS_0;
    __enable_interrupt();

#if TEST == 1      // test basic MCU functionality
    P2DIR |= 0x18; // Set LEDs to output direction
    P2OUT |= 0x18;
    P2SEL &= (~0x18);
    for (;;)
    {
        volatile unsigned int i; // volatile to prevent optimization

        P2OUT ^= 0x18; // Toggle P1.0 using exclusive-OR

        i = 10000; // SW Delay
        do
            i--;
        while (i != 0);
    }
#elif TEST == 2 // test GPIO in/out, timer
    uint8_t ctr = 1U;

    gpio_clear(&ctx->led);
    gpio_set(&ctx->ledErr);
    timer_init();
    timer_setInterval(MS1000);

    for (;;)
    {
        ctr++;
        (ctr & 1) ? gpio_set(&ctx->ledErr) : gpio_clear(&ctx->ledErr);
        gpio_toggle(&ctx->led);
        while (!timeElapsed)
        {
            if (gpio_state(&ctx->swOn))
                gpio_toggle(&ctx->led);
        }
        timeElapsed = false;
    }
#elif TEST == 3 // test blinking and GPI interrupt
    timer_init();
    timer_setInterval(MS250);

    gpio_setInterruptEnabledEdge(&ctx->swOn, true);
    gpio_setInterruptEnabled(&ctx->swOn, true);

    gpio_clear(&ctx->led);
    blinker_setRate(&ctx->ledBlink, 1);
    blinker_setCount(&ctx->ledBlink, 10);               // at the end LED will be off
    blinker_setRate(&ctx->ledErrBlink, 3);              // slower
    blinker_setCount(&ctx->ledErrBlink, CountInfinite); // continuous blinking

    while (true)
    {
        if (/*timeElapsed &&*/ swOnPressTime && !gpio_state(&ctx->swOn) &&
            timer_elapsed(swOnPressTime, 250U))
        {
            timeElapsed = false;
            swOnPressTime = 0;
            // gpio_set(&ctx->swOn, true);
            // gpio_toggle(&context()->led);
        }
    }
#elif TEST == 4 // test motor
    timer_init();
    timer_setInterval(MS250);
    gpio_clear(&ctx->led);
    gpio_clear(&ctx->ledErr);

    static const uint32_t offDuration = 3333U;
    static uint32_t runDuration = 1500U;
    volatile uint32_t motorTime = 0;
    volatile bool running = false;
    ctx->rotationMode = TwoWays;
    ctx->dir = Forward;
    bool initial = true;

    while (true)
    {
        if (timer_elapsed(motorTime, running ? runDuration : offDuration) || initial)
        {
            initial = false;
            if (running)
            {
                motor_stop();
                gpio_clear(&ctx->led);
            }
            else
            {
                runDuration += 500; // increment motor-on time by 500ms in each cycle
                gpio_set(&ctx->led);
                motor_start();
            }
            running = !running;
            motorTime = timer_now();
        }
    }

#endif
    return 0;
}

__interrupt_vec(PORT1_VECTOR) void port1_ISR(void)
{
}
__interrupt_vec(PORT2_VECTOR) void port2_ISR(void)
{
#if TEST == 3
    if (gpio_interrupt(&context()->swOn))
    {
        gpio_clearInterruptFlag(&context()->swOn);
        if (!swOnPressTime)
        {
            swOnPressTime = timer_now();
            blinker_setCount(&context()->ledBlink, 4);
        }
    }
#endif
    P2IFG = 0;
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
    timer_tick();
    timeElapsed = true;
#if TEST == 3 || TEST == 4
    Context *const ctx = context();
    blinker_tick(&ctx->ledBlink);
    blinker_tick(&ctx->ledErrBlink);
#endif
#if TEST == 4
    motor_tick();
    gpio_toggle(&ctx->ledErr);
#endif
}
