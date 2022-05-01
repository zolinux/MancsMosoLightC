#include "common.h"
#include "blinker.h"
#include "gpio.h"

#include <msp430.h>

static Context _context;

const uint8_t numberOfModes = 2;
const uint8_t numberOfDirections = 2;
const uint8_t pwmDutyVsSpeed[] = {45, 75, 100};
const uint8_t ledBlinkRatesVsSpeed[] = {3, 2, 1};
const uint8_t ledBlinkCountSpeedChange = 8U;
const uint16_t lowVoltageThreshold = 444U;

Context *context()
{
    return &_context;
}

void context_init()
{
    gpio_initDir(&_context.swOn, 2, 5, true);
    gpio_initDir(&_context.swFunc, 1, 4, true);
    gpio_initDir(&_context.irIn, 1, 0, true);

    gpio_initVal(&_context.ledErr, 2, 3, false, false);
    gpio_initVal(&_context.led, 2, 4, false, false);
    gpio_initVal(&_context.adcEn, 2, 1, false, false);
    gpio_initVal(&_context.irEn, 1, 3, false, false);
    gpio_initVal(&_context.motorHN, 1, 5, false, true);
    gpio_initVal(&_context.motorHP, 1, 2, false, false);
    gpio_initVal(&_context.motorLN, 1, 7, false, true);
    gpio_initVal(&_context.motorLP, 1, 6, false, false);

    blinker_init(&_context.ledErrBlink, &_context.ledErr);
    blinker_init(&_context.ledBlink, &_context.led);

    _context.lastActivityTime = 0;
    _context.speed = Fast;
    _context.rotationMode = TwoWays;
    _context.dir = Forward;
}

uint16_t context_readAdc(const uint8_t channel)
{
    const uint16_t ch = (uint16_t)channel << 12;
    ADC10CTL1 = ch;
    __nop();
    __nop();
    ADC10CTL0 |= ENC | ADC10SC;
    __nop();

    while (ADC10CTL1 & ADC10BUSY)
    {
    }

    return ADC10MEM;
}

void context_blinkSpeedLed()
{
    blinker_setRate(&_context.ledBlink, ledBlinkRatesVsSpeed[(uint8_t)_context.speed]);
    blinker_setCount(&_context.ledBlink, ledBlinkCountSpeedChange);
}

void context_incrementSpeed()
{
    _context.speed = (Speed)(((uint8_t)_context.speed + 1) % sizeof(ledBlinkRatesVsSpeed));
}

bool context_enableAdc()
{
    ADC10CTL0 = ADC10ON | REFON | SREF_1;
    ADC10AE = ADC10AE0;
    return true;
}

bool context_disableAdc()
{
    ADC10CTL0 = 0;
    return true;
}
