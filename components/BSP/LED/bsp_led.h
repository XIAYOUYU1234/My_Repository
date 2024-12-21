#ifndef BSP_LED_H
#define BSP_LED_H

#include "driver/gpio.h"

enum GPIO_OUTPUT_STATE
{
    PIN_RESET = 0,
    PIN_SET = 1
};

#define LED_GPIO_PIN  GPIO_NUM_1
#define LED(x) x?gpio_set_level(LED_GPIO_PIN, PIN_RESET):gpio_set_level(LED_GPIO_PIN, PIN_SET)

void LED_Init(void);

#endif //BSP_LED_H