#pragma once

#include "blinker.h"
#include "gpio.h"

typedef enum _Speed
{
    Slow,
    Normal,
    Fast,
} Speed;

typedef enum _RotationMode
{
    OneWay,
    TwoWays,
} RotationMode;

typedef enum _Direction
{
    Forward,
    Backward,
} Direction;

extern const uint8_t numberOfModes;
extern const uint8_t numberOfDirections;
extern const uint8_t pwmDutyVsSpeed[];
extern const uint8_t ledBlinkRatesVsSpeed[];
extern const uint8_t ledBlinkCountSpeedChange;
extern const uint16_t lowVoltageThreshold;

typedef struct _Context
{
    Gpio swOn;
    Gpio swFunc;
    Gpio irIn;
    Gpio ledErr;
    Gpio led;
    Gpio adcEn;
    Gpio irEn;
    Gpio motorHN;
    Gpio motorHP;
    Gpio motorLN;
    Gpio motorLP;

    Blinker ledErrBlink;
    Blinker ledBlink;

    uint32_t lastActivityTime;
    Speed speed;
    RotationMode rotationMode;
    Direction dir;
} Context;

Context *context();
void context_init();

// bool enableAdc();
// bool disableAdc();
// uint16_t readAdc(const uint8_t channel);
// void blinkSpeedLed();
// void incrementSpeed();
