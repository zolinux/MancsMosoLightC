#pragma once

#include <stdbool.h>
#include <stdint.h>

struct port_t;
typedef struct gpio_t
{
    volatile struct port_t *port;
    uint8_t bit;
    uint8_t mask;
} Gpio;

void gpio_init(Gpio *ctx, const uint8_t port, const uint8_t bit);
void gpio_initDir(Gpio *ctx, const uint8_t port, const uint8_t bit, bool in);
void gpio_initVal(Gpio *ctx, const uint8_t port, const uint8_t bit, bool in, bool value);
void gpio_set(Gpio *ctx);
void gpio_clear(Gpio *ctx);
void gpio_toggle(Gpio *ctx);
void gpio_setDirection(Gpio *ctx, bool in);
void gpio_setSelection(Gpio *ctx, bool alt);
void gpio_setInterruptEnabled(Gpio *ctx, bool enabled);
void gpio_setInterruptEnabledEdge(Gpio *ctx, bool rising);
bool gpio_interrupt(const Gpio *ctx);
void gpio_clearInterruptFlag(Gpio *ctx);
uint8_t gpio_mask(const Gpio *ctx);
bool gpio_state(const Gpio *ctx);
