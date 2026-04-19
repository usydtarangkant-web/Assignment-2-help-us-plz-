//timer.c module (part a, b, d)
#include "timer.h"
#include "stm32f303xc.h"
#include "stddef.h"

#define TIMER_MAX_COUNT   8U //can store 8 independent software timers
#define TIMER_TICK_US     100U //one timer tick is 100us or 0.1ms (chosen so that 1ms, 1.5ms, 2ms positions can be represented more accurately)
#define TICKS_PER_MS      (1000U / TIMER_TICK_US) //convert from ms to ticks

typedef struct {
    uint32_t         counter_ticks; //Counter - to count down from period_ticks to 0
    Timer_Callback_t callback; //Function when counter hits 0
    uint8_t          enabled; //1 = timer running, 0 = timer stopped
    uint8_t          one_shot; //1 = stop after first time, 0 = periodic (part d)
    uint32_t         period_ticks; //Load value in internal 100 us ticks
} Timer_Data_t;

//Part b - keep timer data in timer.c
//allows multiple timers at once
static Timer_Data_t s_timer_data[TIMER_MAX_COUNT]; //size of timer
static uint8_t s_timer_used[TIMER_MAX_COUNT]; //to track which timer slots are used

//Return a pointer to the timer data in timer.c
static Timer_Data_t *Timer_GetData(Timer_t *tim)
{
	//safety checks - if true return null
    if ((tim == NULL) || (tim->allocated == 0U) || (tim->slot >= TIMER_MAX_COUNT)) {
        return NULL;
    }

    return &s_timer_data[tim->slot];
}

//Return a pointer to the timer data in timer.c (doesn't allow modification to use in part b)
static const Timer_Data_t *Timer_GetDataConst(const Timer_t *tim)
{
	//safety checks - if true return null
    if ((tim == NULL) || (tim->allocated == 0U) || (tim->slot >= TIMER_MAX_COUNT)) {
        return NULL;
    }

    return &s_timer_data[tim->slot];
}

//find a free timer slot, assign to 'Timer_t' and return a pointer
static Timer_Data_t *Timer_Allocate(Timer_t *tim)
{
    uint32_t i;
    //safety check
    if (tim == NULL) {
        return NULL;
    }
    //if already allocated, don't reallocate and return existing pointer
    if ((tim->allocated != 0U) && (tim->slot < TIMER_MAX_COUNT)) {
        return &s_timer_data[tim->slot];
    }
    //loop through all timer slots till 0 is found (free slot), if found then allocate and mark as 1 (used)
    for (i = 0U; i < TIMER_MAX_COUNT; i++) {
        if (s_timer_used[i] == 0U) {
            s_timer_used[i] = 1U;
            tim->slot = (uint8_t)i;
            tim->allocated = 1U;
            return &s_timer_data[i]; //return data
        }
    }
    //return null if no free slots
    return NULL;
}

//Part a - initialisation
void Timer_Init(Timer_t *tim, uint32_t period_ms, Timer_Callback_t cb)
{
    Timer_Data_t *data;
    //safety check for invalid 0ms period
    if (period_ms == 0U) {
        return;
    }
    //find a free slot and check if allocation works
    data = Timer_Allocate(tim);
    if (data == NULL) {
        return;
    }

    data->period_ticks  = period_ms * TICKS_PER_MS;   //Store the interval (ms) that was passed as a parameter
    data->counter_ticks = data->period_ticks;         //Set counter = period for immediate decrement of counter
    data->callback      = cb;                         //Store the function pointer for callback
    data->one_shot      = 0U;                         //Ensure timer is periodic (for part d) (0 = periodic default, 1 = one-shot)
    data->enabled       = 1U;                         //Start timer right after initialisation (1 = timer running, 0 = timer stop)
}

//Initialisation in 100 us ticks for servo PWM (part a)
void Timer_InitTicks(Timer_t *tim, uint32_t period_ticks, Timer_Callback_t cb)
{
    Timer_Data_t *data;
    //period safety check
    if (period_ticks == 0U) {
        return;
    }
    //check allocation success
    data = Timer_Allocate(tim);
    if (data == NULL) {
        return;
    }

    //definitions
    data->period_ticks  = period_ticks; //store the period of timer in ticks
    data->counter_ticks = period_ticks; //set countdown from period_ticks to 0
    data->callback      = cb; //store function pointer (for callback)
    data->one_shot      = 0U; //0 = periodic timer, 1 = one shot
    data->enabled       = 1U; //1 = timer running, 0 = timer stop
}

//Part a - send callback after every 100 us systick
void Timer_Tick(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);

    //Stop function if timer is not enabled
    if ((data == NULL) || (data->enabled == 0U)) {
        return;
    }

    //Decrement counter by 1 to 0 every 100 us
    if (data->counter_ticks > 0U) {
        data->counter_ticks--;
    }

    //If counter is 0, then call the function pointer during initialisation in Timer_Init
    if (data->counter_ticks == 0U) {
        if (data->callback != NULL) {
            data->callback();
        }

        //Part d: if 1 continue one-shot and don't load periodic intervals, else stop one-shot and load periodic intervals
        if (data->one_shot != 0U) {
            data->enabled = 0U; //Don't reload counter
        } else {
            data->counter_ticks = data->period_ticks; //Load counter for next interval and immediate decrement (repeat forever)
        }
    }
}
//Part a
void Timer_Start(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);

    //safety check
    if (data == NULL) {
        return;
    }

    data->counter_ticks = data->period_ticks;  //reset counter for full period
    data->enabled = 1U;                        //Reload counter
}
//Part a
void Timer_Stop(Timer_t *tim)
{
    Timer_Data_t *data = Timer_GetData(tim);
    //safety check
    if (data == NULL) {
        return;
    }

    data->enabled = 0U; //Don't reload counter (stop counter)
}

//Part b
//get period (ms)
uint32_t Timer_GetPeriod(const Timer_t *tim)
{
    const Timer_Data_t *data = Timer_GetDataConst(tim);
    //safety check
    if (data == NULL) {
        return 0U;
    }
    //return converted ticks to ms
    return (data->period_ticks / TICKS_PER_MS);
}

//Set new period (ms)
void Timer_SetPeriod(Timer_t *tim, uint32_t period_ms)
{
    Timer_Data_t *data = Timer_GetData(tim); //convert to internal timer storage
    //safety check
    if ((data == NULL) || (period_ms == 0U)) {
        return;
    }

    data->period_ticks  = period_ms * TICKS_PER_MS; //convert ms to ticks
    data->counter_ticks = data->period_ticks; //reset counter to start at new period
}

//Part d
void Timer_OnceMs(Timer_t *tim, uint32_t delay_ms, Timer_Callback_t cb)
{
    Timer_Data_t *data;
    //if delay is 0us, return
    if (delay_ms == 0U) {
        return;
    }

    Timer_Init(tim, delay_ms, cb); //set period, load counter and enable
    //convert to 1 shot if timer data is valid
    data = Timer_GetData(tim);
    if (data != NULL) {
        data->one_shot = 1U;
    }
}

//One-shot in 100 us ticks for servo PWM
void Timer_OnceTicks(Timer_t *tim, uint32_t delay_ticks, Timer_Callback_t cb)
{
    Timer_Data_t *data;
    //safety check
    if (delay_ticks == 0U) {
        return;
    }

    Timer_InitTicks(tim, delay_ticks, cb); //set period, load counter and enable
    //1 shot if timer ticks is valid (for part d + c)
    data = Timer_GetData(tim);
    if (data != NULL) {
        data->one_shot = 1U;
    }
}
