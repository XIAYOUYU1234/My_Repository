#include "bsp_led.h"

void LED_Init(void)
{
    gpio_config_t gpio_config_struct;

    gpio_config_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_config_struct.mode = GPIO_MODE_OUTPUT;
    gpio_config_struct.pin_bit_mask = 1ull << LED_GPIO_PIN;
    gpio_config_struct.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config_struct.pull_up_en = GPIO_PULLUP_DISABLE;

    gpio_config(&gpio_config_struct);

    LED(0);
}
