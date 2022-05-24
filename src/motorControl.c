#include "motorControl.h"

#include "common.h"
#include "timer.h"

#include <stdbool.h>
#include <stdint.h>

void doStartMotor();
void doStartMotorSpeed(uint8_t pwmPercent);
void doStopMotor();

static const uint16_t spinUpTime = 500U;
static const uint16_t spinDownTime = 500U;
static const uint16_t dirChangeTime = 3000U;

static enum MotorState {
    Idle,
    Starting,
    Running,
    Stopping,
} state;

static uint32_t startedAt;
static bool stopRequested;

void motor_start()
{
    if (state != Idle)
        return;

    stopRequested = false;
    state = Starting;
    startedAt = 0;

    motor_tick();
}

void motor_stop()
{
    if (state == Idle)
        return;

    stopRequested = true;
    motor_tick();
}

void motor_tick()
{
    if (state == Idle)
        return;

    if (stopRequested)
    {
        state = Idle;
        doStopMotor();
        return;
    }

    Context *const c = context();

    switch (state)
    {
    case Starting:
        if (!startedAt)
        {
            startedAt = timer_now();
            doStartMotor();
        }
        else if (timer_elapsed(startedAt, spinUpTime))
        { // spin-up done, switch to running
            startedAt = timer_now();
            state = Running;
            if (c->speed != Fast)
            {
                doStartMotorSpeed(pwmDutyVsSpeed[(uint8_t)c->speed]);
            }
        }

        break;
    case Running:
        if (c->rotationMode == TwoWays && timer_elapsed(startedAt, dirChangeTime))
        { // stop and reverse rotation
            startedAt = timer_now();
            doStopMotor();
            state = Stopping;
        }
        break;
    case Stopping:
        if (timer_elapsed(startedAt, spinDownTime))
        {
            startedAt = 0;
            state = Starting;
            if (c->rotationMode == TwoWays)
            {
                c->dir = (Direction)(((uint8_t)(c->dir) + 1) % numberOfDirections);
            }
        }
        break;
    case Idle:
        break;
    }
}

void doStartMotor()
{
    Context *const c = context();
    Gpio *const fetArray[] = {&c->motorHP, &c->motorHN, &c->motorLP, &c->motorLN};
    uint8_t toSetIdx = c->dir == Forward ? 0 : 2;

    // enable motor +Vbat, order is important
    gpio_set(fetArray[toSetIdx + 1]);
    gpio_set(fetArray[toSetIdx]);

    const uint8_t fetCount = sizeof(fetArray) / sizeof(*fetArray);
    toSetIdx += fetCount / 2;
    if (toSetIdx >= fetCount)
        toSetIdx = 0;

    // enable motor -Vbat, order is important
    gpio_clear(fetArray[toSetIdx]);
    gpio_clear(fetArray[toSetIdx + 1]);
}

void doStartMotorSpeed(uint8_t pwmPercent)
{
    // ToDo: implement
    (void)pwmPercent;
}

void doStopMotor()
{
    Context *const c = context();

    gpio_clear(&c->motorHP);
    gpio_set(&c->motorHN);
    gpio_clear(&c->motorLP);
    gpio_set(&c->motorLN);
}