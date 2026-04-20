#include "timer.h"
#include "stddef.h"

#define TIMER_MAX_COUNT   8U
#define TIMER_TICK_US     100U
#define TICKS_PER_MS      (1000U / TIMER_TICK_US)

typedef struct
{
    uint32_t         counter_ticks;
    Timer_Callback_t callback;
    uint8_t          enabled;
    uint8_t          one_shot;
    uint32_t         period_ticks;
} Timer_Data_t;

static Timer_Data_t s_timer_data[TIMER_MAX_COUNT];
static uint8_t s_timer_used[TIMER_MAX_COUNT];

static Timer_Data_t *Timer_GetData(Timer_t *tim)
{
    if ((tim == NULL) || (tim->allocated == 0U) || (tim->slot >= TIMER_MAX_COUNT))
    {
        return NULL;
    }

    return &s_timer_data[tim->slot];
}

static const Timer_Data_t *Timer_GetDataConst(const Timer_t *tim)
{
    if ((tim == NULL) || (tim->allocated == 0U) || (tim->slot >= TIMER_MAX_COUNT))
    {
        return NULL;
    }

    return &s_timer_data[tim->slot];
}

static Timer_Data_t *Timer_Allocate(Timer_t *tim)
{
    uint32_t i;

    if (tim == NULL)
    {
        return NULL;
    }

    if ((tim->allocated != 0U) && (tim->slot < TIMER_MAX_COUNT))
    {
        return &s_timer_data[tim->slot];
    }

    for (i = 0U; i < TIMER_MAX_COUNT; i++)
    {
        if (s_timer_used[i] == 0U)
        {
            s_timer_used[i] = 1U;
            tim->slot = (uint8_t)i;
            tim->allocated = 1U;
            return &s_timer_data[i];
        }
    }

    return NULL;
}

void Timer_Init(Timer_t *tim, uint32_t period_ms, Timer_Callback_t cb)
{
    Timer_Data_t *data;

    if (period_ms == 0U)
    {
        return;
    }

    data = Timer_Allocate(tim);
    if (data == NULL)
    {
        return;
    }

    data->period_ticks  = period_ms * TICKS_PER_MS;
    data->counter_ticks = data->period_ticks;
    data->callback      = cb;
    data->one_shot      = 0U;
    data->enabled       = 1U;
}

void Timer_InitTicks(Timer_t *tim, uint32_t period_ticks, Timer_Callback_t cb)
{
    Timer_Data_t *data;

    if (period_ticks == 0U)
    {
        return;
    }

    data = Timer_Allocate(tim);
    if (data == NULL)
    {
        return;
    }

    data->period_ticks  = period_ticks;
    data->counter_ticks = period_ticks;
    data->callback      = cb;
    data->one_shot      = 0U;
    data->enabled       = 1U;
}

void Timer_Tick(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);

    if ((data == NULL) || (data->enabled == 0U))
    {
        return;
    }

    if (data->counter_ticks > 0U)
    {
        data->counter_ticks--;
    }

    if (data->counter_ticks == 0U)
    {
        if (data->callback != NULL)
        {
            data->callback();
        }

        if (data->one_shot != 0U)
        {
            data->enabled = 0U;
        }
        else
        {
            data->counter_ticks = data->period_ticks;
        }
    }
}

void Timer_Start(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);

    if (data == NULL)
    {
        return;
    }

    data->counter_ticks = data->period_ticks;
    data->enabled = 1U;
}

void Timer_Stop(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);

    if (data == NULL)
    {
        return;
    }

    data->enabled = 0U;
}

uint32_t Timer_GetPeriod(const Timer_t *tim)
{
    const Timer_Data_t *data = Timer_GetDataConst(tim);

    if (data == NULL)
    {
        return 0U;
    }

    return (data->period_ticks / TICKS_PER_MS);
}

void Timer_SetPeriod(Timer_t *tim, uint32_t period_ms)
{
    Timer_Data_t *data = Timer_GetData(tim);

    if ((data == NULL) || (period_ms == 0U))
    {
        return;
    }

    data->period_ticks  = period_ms * TICKS_PER_MS;
    data->counter_ticks = data->period_ticks;
}

void Timer_OnceMs(Timer_t *tim, uint32_t delay_ms, Timer_Callback_t cb)
{
    Timer_Data_t *data;

    if (delay_ms == 0U)
    {
        return;
    }

    Timer_Init(tim, delay_ms, cb);

    data = Timer_GetData(tim);
    if (data != NULL)
    {
        data->one_shot = 1U;
    }
}

void Timer_OnceTicks(Timer_t *tim, uint32_t delay_ticks, Timer_Callback_t cb)
{
    Timer_Data_t *data;

    if (delay_ticks == 0U)
    {
        return;
    }

    Timer_InitTicks(tim, delay_ticks, cb);

    data = Timer_GetData(tim);
    if (data != NULL)
    {
        data->one_shot = 1U;
    }
}
