#include "bsp_i2c.h"

#include "esp_log.h"
#define TAG "I2C"

uint8_t I2C1_Init_Flag = 0;

void I2C_Init(void)
{
    i2c_config_t i2c_config_struct = {0};
    esp_err_t ret = ESP_OK;

    i2c_config_struct.master.clk_speed = 400000;
    i2c_config_struct.mode = I2C_MODE_MASTER;
    i2c_config_struct.scl_io_num = GPIO_NUM_42;
    i2c_config_struct.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config_struct.sda_io_num = GPIO_NUM_41;
    i2c_config_struct.sda_pullup_en = GPIO_PULLUP_ENABLE;

    i2c_param_config(I2C_NUM_0, &i2c_config_struct);
    
    ret = i2c_driver_install(I2C_NUM_0, i2c_config_struct.mode, 0, 0, 0);

    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C 总线初始化失败， ErrorCode = %d\r\n", ret);
    }
    else
    {
        ESP_LOGI(TAG, "I2C 总线初始化成功\r\n");
    }

    SetI2C1InitFlagVal(1);
}

void SetI2C1InitFlagVal(uint8_t Data)
{
    I2C1_Init_Flag = Data;
}

uint8_t GetI2C1InitFlagVal(void)
{
    return I2C1_Init_Flag;
}