#include "blinker.h"
#include "gpio.h"

void blinker_init(Blinker *ctx, Gpio *gpio)
{
    ctx->gpio = gpio;
    ctx->count = 0;
    ctx->ctr = 2;
    ctx->rate = 2;
}

void blinker_setRate(Blinker *ctx, uint8_t ticksPerLevel)
{
    ctx->rate = ticksPerLevel;
    ctx->ctr = ctx->rate;
}

void blinker_setCount(Blinker *ctx, uint8_t count)
{
    ctx->ctr = ctx->rate;
    ctx->count = count;
}

void blinker_tick(Blinker *ctx)
{
    if (!ctx->count)
        return;

    if (--ctx->ctr == 0)
    {
        ctx->ctr = ctx->rate;
        gpio_toggle(ctx->gpio);
        if (ctx->count != CountInfinite)
            ctx->count--;
    }
}
