#include <stdio.h>
#include "bsp_led.h"
#include "bsp_i2c.h"
#include "bsp_XL9555.h"
#include "bsp_key.h"
#include "bsp_usart.h"
#include "bsp_ci1303.h"
#include "bsp_qma6100p.h"

#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "customnvsapi.h"
#include "ci1303moduletask.h"
#include "keymoduletask.h"
#include "qma6100pmoduletask.h"

#include "esp_log.h"
#define TAG "MAIN"

//  storage,  data,     spiffs,      0xc00000,   0x400000, ,

void app_main(void)
{
    esp_err_t ret;
    // esp_partition_t* partition_ptr = NULL;
    
    ret= nvs_flash_init();  /* 初始化NVS */

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    //NVS配置区读取，默认配置写入
    ESP_LOGI(TAG, "nvs_config_init \r\n");
	nvs_config_init();

    LED_Init();
    Xl9555_init();
    Key_Init();
    Usart_Init(115200);
    QMA6100P_Init();

    KEYScan_Start();
    KEYScan_Action_Start();

    CI1303_USART_Rec_Start();
    CI1303_USART_Action_Start();


    QMA6100P_Task_Start();

    while(1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
