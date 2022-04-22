#include "gpio.h"

#include <msp430.h>

struct port_t
{
    volatile const uint8_t in;
    volatile uint8_t out;
    volatile uint8_t dir;
    volatile uint8_t ifg;
    volatile uint8_t ies;
    volatile uint8_t ie;
    volatile uint8_t sel;
} __attribute__((__packed__));

void gpio_init(Gpio *ctx, const uint8_t port, const uint8_t bit)
{
    ctx->port = (volatile struct port_t *)(port == 1 ? &P1IN : &P2IN);
    ctx->bit = bit;
    ctx->mask = 1 << bit;
    gpio_setSelection(ctx, false);
}

void gpio_initDir(Gpio *ctx, const uint8_t port, const uint8_t bit, bool in)
{
    gpio_init(ctx, port, bit);
    gpio_setDirection(ctx, in);
}

void gpio_initVal(Gpio *ctx, const uint8_t port, const uint8_t bit, bool in, bool value)
{
    gpio_initDir(ctx, port, bit, in);
    value ? gpio_set(ctx) : gpio_clear(ctx);
}

void gpio_set(Gpio *ctx)
{
    ctx->port->out |= ctx->mask;
}

void gpio_clear(Gpio *ctx)
{
    ctx->port->out &= ~ctx->mask;
}

void gpio_toggle(Gpio *ctx)
{
    ctx->port->out ^= ctx->mask;
}

void gpio_setDirection(Gpio *ctx, bool in)
{
    const uint8_t cv = ctx->port->dir;
    ctx->port->dir = in ? cv & ~ctx->mask : cv | ctx->mask;
}

void gpio_setSelection(Gpio *ctx, bool alt)
{
    const uint8_t cv = ctx->port->sel;
    ctx->port->sel = alt ? cv | ctx->mask : cv & ~ctx->mask;
}

void gpio_setInterruptEnabled(Gpio *ctx, bool enabled)
{
    const uint8_t cv = ctx->port->ie;
    ctx->port->ie = enabled ? cv | ctx->mask : cv & ~ctx->mask;
}

void gpio_setInterruptEnabledEdge(Gpio *ctx, bool rising)
{
    const uint8_t cv = ctx->port->ies;
    ctx->port->ies = rising ? cv & ~ctx->mask : cv | ctx->mask;
}

bool gpio_interrupt(const Gpio *ctx)
{
    return ctx->port->ifg & ctx->mask;
}

void gpio_clearInterruptFlag(Gpio *ctx)
{
    ctx->port->ifg &= ~ctx->mask;
}

uint8_t gpio_mask(const Gpio *ctx)
{
    return ctx->mask;
}

bool gpio_state(const Gpio *ctx)
{
    return ctx->port->in & ctx->mask;
}
