#include "states.h"

#include "common.h"
#include "gpio.h"
#include "motorControl.h"
#include "timer.h"

#include <msp430.h>

static const uint32_t inactiveTimeShutdown = 3UL * 60 * 1000;
static const uint16_t buttonTimePowerOn = 3UL * 1000 / 2; // 1.5seconds
static const uint16_t buttonTimeShutdown = 3UL * 1000;
static const uint16_t runWithoutPawTime = 3000U;
static const uint16_t infraSensorActivationTime = 500U;

static void handleBoot();
static void handleIdle();
static void handleActive();

typedef void (*StateHandler)();
static const StateHandler stateHandlers[] = {&handleBoot, &handleIdle, &handleActive};

enum MainState
{
    Boot,
    Idle,
    Active,
} state;

static bool irEnabled;
static bool powerOn;
static uint32_t irAssertedTime;
static uint32_t irDeAssertedTime;
static uint32_t swOnPressedTime;
static uint32_t swFuncPressedTime;

static void clear()
{
    state = Boot;
    swOnPressedTime = 0;
    irEnabled = false;
}

static void shutdown()
{
    Context *const c = context();

    __disable_interrupt();
    context_disableAdc();
    gpio_setInterruptEnabled(&c->irIn, false);
    gpio_clear(&c->irEn);
    powerOn = false;
    irDeAssertedTime = 0;
    clear();
    state = Boot;
    // timer_stop();
    __enable_interrupt();
}

/**
 * Runs once after turning on. Battery voltage gets measured then proceed to Idle state...
 */
static void handleBoot()
{
    if (!powerOn)
        return;

    Context *const c = context();

    gpio_set(&c->adcEn);
    context_enableAdc();
    __nop();
    __nop();
    __nop();
    __nop();
    const uint16_t adcVal = context_readAdc(0);
    if (lowVoltageThreshold > adcVal)
    {
        // blink red led
        blinker_setRate(&c->ledErrBlink, 4);
        blinker_setCount(&c->ledErrBlink, CountInfinite);
    }
    else
    {
        context_blinkSpeedLed();
        blinker_setCount(&c->ledBlink, ledBlinkCountSpeedChange + 1); // leave LED on after finish
    }

    state = Idle;
    irEnabled = false;
    gpio_set(&c->irEn);
}

static void handleIdle()
{
    Context *const c = context();
    if (timer_elapsed(c->lastActivityTime, inactiveTimeShutdown))
    {
        // automatic turn-off after ... minutes
        shutdown();
        return;
    }

    if (!irEnabled && timer_elapsed(c->lastActivityTime, infraSensorActivationTime))
    { // turn on infra gate
        irEnabled = true;
        gpio_setInterruptEnabled(&c->irIn, true);
    }

    if (irAssertedTime)
    {
        state = Active;
        motor_start();
    }
}

static void handleActive()
{
    Context *const c = context();
    if (!gpio_state(&c->irIn) && !irDeAssertedTime)
    { // object moves out of IR sensor range
        irDeAssertedTime = timer_now();
        irAssertedTime = 0;
    }
    if (irDeAssertedTime && timer_elapsed(irDeAssertedTime, runWithoutPawTime))
    {
        irDeAssertedTime = 0;
        state = Idle;
        motor_stop();
    }
}

void state_tick()
{
    // do common things
    Context *const c = context();

    if (swOnPressedTime)
    { // still pressed
        if (gpio_state(&c->swOn))
        {
            if (state == Boot)
            {
                if (timer_elapsed(swOnPressedTime, buttonTimePowerOn))
                { // button pressed long => turn on
                    powerOn = true;
                    c->lastActivityTime = timer_now();
                }
            }
            else if (timer_elapsed(swOnPressedTime, buttonTimeShutdown))
            { // button pressed long => turn off
                shutdown();
            }
        }
        else
        { // switch was released
            swOnPressedTime = 0;
            if (state == Boot)
            { // no power-up yet, it was too short -> go back to off
              // timer_stop();
            }
            else
            { // speed change
                context_incrementSpeed();
                context_blinkSpeedLed();
                c->lastActivityTime = timer_now();
            }
        }
    }

    if (state != Boot && swFuncPressedTime)
    { // still pressed
        if (gpio_state(&c->swFunc))
        { // change rotation mode
            c->rotationMode = (RotationMode)(((uint8_t)c->rotationMode + 1) % numberOfModes);
            swFuncPressedTime = 0;
        }
    }

    // call specific state handler
    stateHandlers[(uint8_t)state]();
}

void state_gpio()
{
    const uint32_t t = timer_now();
    Context *const c = context();
    c->lastActivityTime = t;

    if (gpio_interrupt(&c->swOn))
    {
        // timer_start();
        if (gpio_state(&c->swOn))
            swOnPressedTime = t;
        gpio_clearInterruptFlag(&c->swOn);
    }

    if (gpio_interrupt(&c->swFunc))
    {
        if (gpio_state(&c->swFunc))
            swFuncPressedTime = t;
        gpio_clearInterruptFlag(&c->swFunc);
    }

    if (irEnabled && gpio_interrupt(&c->irIn))
    {
        gpio_clearInterruptFlag(&c->irIn);
        if (gpio_state(&c->irIn))
        {
            irAssertedTime = t;
            irDeAssertedTime = 0;
        }
    }

    state_tick();
}
