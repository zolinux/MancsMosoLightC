#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum _WdtInterval
{
    MS1000,
    MS250,
    MS16,
    MS2
} WdtInterval;

void timer_tick();

void timer_init();
void timer_reset();

/**
 * @brief return current time
 *
 * @return milliseconds
 */
uint32_t timer_now();

bool timer_elapsed(const uint32_t since, const uint32_t msecs);

/**
 * @brief Set periodic timer interrupt Interval. Running timer must be stopped
 * before setting new timeout. Implicitly starts the timer
 *
 * @param interval in milliseconds (rounded)
 * @return true success
 * @return false timer is already running.
 */
bool timer_setInterval(WdtInterval interval);

/**
 * @brief Set the next timer interrupt to occur in ... msecs. Running timer must be stopped
 * before setting new timeout.
 *
 * @param ms time after interrupt to be triggered
 * @return true success
 * @return false timer is already running.
 */
bool timer_setIntervalMs(uint32_t ms);

/**
 * @brief stop timer
 */
void timer_stop();

/** @brief start interval timer with current settings
 */
void timer_start();
