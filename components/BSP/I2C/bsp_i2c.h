#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "driver/gpio.h"
#include "driver/i2c.h"
// #include "driver/i2c_master.h"

void I2C_Init(void);
void SetI2C1InitFlagVal(uint8_t Data);
uint8_t GetI2C1InitFlagVal(void);

#endif //BSP_I2C_H