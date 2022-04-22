#pragma once

#include <stdint.h>

#define CountInfinite UINT8_MAX

struct gpio_t;
typedef struct blinker_t
{
    struct gpio_t *gpio;
    uint8_t count; // 0xFF->infinite
    uint8_t rate;  // how many ticks to keep the level
    uint8_t ctr;
} Blinker;

void blinker_init(Blinker *ctx, struct gpio_t *gpio);
void blinker_setRate(Blinker *ctx, uint8_t ticksPerLevel);
void blinker_setCount(Blinker *ctx, uint8_t count);
void blinker_tick(Blinker *ctx);
