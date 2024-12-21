#ifndef BSP_KEY_H
#define BSP_KEY_H

#include "driver/gpio.h"

#define KEY_BOOT_GPIO_PIN  GPIO_NUM_0

#define KEY_BOOT_Read gpio_get_level(KEY_BOOT_GPIO_PIN)

typedef enum{
    KEY_RELEASE = 0,    //按键抬起
    KEY_SHORT_DOWN,     //按键短按
    KEY_LONG_DOWN,      //按键长按
    KEY_LONG_LONG_DOWN, //按键超长按
    KEY_COMBINATE0,     //组合按键0
    KEY_COMBINATE1,     //组合按键1
    KEY_TIMEOUT         //按下超时
}EnumDef_KEY_Status;

typedef struct{
    EnumDef_KEY_Status KEY_Status;
}StructDef_SingleKEY;

typedef struct 
{
    StructDef_SingleKEY KEY0;
    StructDef_SingleKEY KEY_POWER;
    StructDef_SingleKEY KEY_SOS;
    StructDef_SingleKEY KEY_PTT;
    StructDef_SingleKEY KEY_OK;
    StructDef_SingleKEY KEY_UP;
    StructDef_SingleKEY KEY_DOWN;
    StructDef_SingleKEY KEY_BOOT;
}StructDef_KEY;


extern uint8_t Flag_KeySwitch;


uint8_t Total_KeyScan(StructDef_KEY* StructDef_KEY_Buf);
void Key_Init(void);

#endif //BSP_KEY_H