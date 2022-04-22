#include "timer.h"

#include <msp430.h>

static const uint8_t timeShiftMantissa = 4;
static const uint16_t clockFrequency = 32768; // Hz
static const uint16_t intervalDivider[4] = {
    32768U,
    8192U,
    512U,
    64U,
};

static uint32_t currentTime; // time (ms) * 2^timeShiftMantissa
static uint32_t timeToTrigger;

static bool isRunning()
{
    return !(WDTCTL & WDTHOLD);
}

static uint32_t getIntervalMs(const WdtInterval interval)
{
    return ((uint32_t)clockFrequency << timeShiftMantissa) / intervalDivider[(uint8_t)interval];
}

static void startInterval(WdtInterval interval)
{
    (void)interval;
    WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | (uint8_t)interval;
    IE1 = 1;
}

static WdtInterval getInterval(uint32_t ms)
{
    ms <<= timeShiftMantissa;
    int i = 0;
    while (i < 4)
    {
        const WdtInterval interval = (WdtInterval)i;
        if (ms >= getIntervalMs(interval))
            return interval;
        i++;
    }
    return MS2;
}

void timer_tick()
{
    const uint32_t delta =
        ((uint32_t)clockFrequency << timeShiftMantissa) / intervalDivider[(WDTCTL >> WDTIS0) & 3];

    if (timeToTrigger)
    {
        if (timeToTrigger > delta)
            timeToTrigger -= delta;
        else
            timeToTrigger = 0;

        if (timeToTrigger)
        { // set next trigger timeout
            timer_setInterval(getInterval(timeToTrigger));
        }
    }
    currentTime += delta;
}

void timer_init()
{
    currentTime = 0;
    timeToTrigger = 0;
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
    return t > since && t - since > period;
}

bool timer_setInterval(WdtInterval interval)
{
    if (isRunning())
        return false;

    timeToTrigger = 0;
    startInterval(interval);
    return true;
}

bool timer_setIntervalMs(uint32_t ms)
{
    if (isRunning())
        return false;

    return timer_setInterval(getInterval(ms));
}

void timer_stop()
{
    WDTCTL = WDTPW | WDTHOLD;
}

void timer_start()
{
}
