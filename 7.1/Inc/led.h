#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LED3 = 0,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED9,
    LED10
} led_t;

void led_init(void);

// 设置目标状态，不会阻塞
void led_set(led_t led, bool state);

// 获取当前“实际显示”的状态
bool led_get(led_t led);

// 切换目标状态
void led_toggle(led_t led);

// 设置最小变化间隔（毫秒）
void led_set_min_interval(uint32_t interval_ms);

// 在主循环里不断调用，让 LED 按 timer 节奏更新
void led_update(void);

#endif // LED_H
