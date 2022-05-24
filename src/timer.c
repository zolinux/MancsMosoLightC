#include "timer.h"

#include <msp430.h>

static const uint8_t timeShiftMantissa = 6;
static const uint16_t elapsedTimePerTick[4] = {
    // [ms]*1<<timeShiftMantissa (for clock freq 32768Hz)
    64000U,
    16000U,
    1000U,
    125U,
};

static volatile uint32_t currentTime; // time (ms) * 2^timeShiftMantissa

static bool isRunning()
{
    return !(WDTCTL & WDTHOLD);
}

static void startInterval(WdtInterval interval)
{
    WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | (uint8_t)interval;
    IE1 = 1;
}

void timer_tick()
{
    currentTime += elapsedTimePerTick[WDTCTL & 3];
}

void timer_init()
{
    currentTime = 0;
}

void timer_reset()
{
    timer_init();
}

uint32_t timer_now()
{
    return currentTime >> timeShiftMantissa;
}

bool timer_elapsed(const uint32_t since, const uint32_t period)
{
    const uint32_t t = timer_now();
    return (t > since) && ((t - since) > period);
}

bool timer_setInterval(WdtInterval interval)
{
    if (isRunning())
        return false;

    startInterval(interval);
    return true;
}

void timer_stop()
{
    WDTCTL = WDTPW | WDTHOLD;
}

void timer_start()
{
    WDTCTL = WDTPW | ((0x00FF & WDTCTL) & ~WDTHOLD);
}
