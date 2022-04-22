#include "blinker.h"
#include "context.h"
#include "gpio.h"
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
    BCSCTL2 = SELM_3 | SELS;
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

    ctx->led.clear();
    ctx->ledErr.set();
    timer_init();
    timer_setInterval(MS1000);

    for (;;)
    {
        ctr++;
        ctx->ledErr << static_cast<bool>(ctr & 1);
        ctx->led.toggle();
        while (!timeElapsed)
        {
            if (ctx->swOn)
                ctx->led.toggle();
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
    ctx->led.clear();
    ctx->ledErr.clear();

    static const uint32_t runDuration = 5000U;
    volatile uint32_t motorTime = 0;
    volatile bool running = false;
    ctx->rotationMode = RotationMode::OneWay;

    while (true)
    {
        if (timer_elapsed(motorTime, runDuration))
        {
            auto &m = motor();

            if (running)
            {
                // m.stopMotor();
                ctx->led.clear();
            }
            else
            {
                ctx->led.set();
                // m.startMotor();
            }
            running ^= true;

            motorTime = timer_now();
            (void)m;
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
    Context *ctx = context();
    timeElapsed = true;
#if TEST == 3 || TEST == 4
    blinker_tick(&ctx->ledBlink);
    blinker_tick(&ctx->ledErrBlink);
#endif
#if TEST == 4
    //        motor_tick(motor());
    gpio_toggle(&ctx->ledErr);
#endif
}
