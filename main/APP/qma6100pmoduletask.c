#include "qma6100pmoduletask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

#include "bsp_qma6100p.h"
#include "bsp_xl9555.h"

#include "esp_log.h"
#define TAG "QMA6100P"
#define TAG1 "FreeRTOS"

/* QMA6100P_Task_Handler 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define QMA6100P_Task_PRIO      3                          /* 任务优先级 */
#define QMA6100P_Task_STK_SIZE  1024*5                     /* 任务堆栈大小 */
TaskHandle_t  QMA6100P_Task_Handler;                       /* 任务句柄 */
static void QMA6100P_Rec_Task(void *pvParameters);       /* 任务函数 */

void QMA6100P_Task_Start(void)
{
    BaseType_t Ret = pdPASS;

// /* 创建语音队列 */
//     VOICE_Queue = xQueueCreate(VOICE_Queue_Length,     /* 队列项目长度 */
//                             VOICE_Queue_ItemSize);     /* 队列项目长度 */

 /* 创建任务 */
    Ret = xTaskCreatePinnedToCore((TaskFunction_t )QMA6100P_Rec_Task,            /* 任务函数 */
                            (const char*    )"QMA6100P_Rec_Task",                /* 任务名称 */
                            (uint16_t       )QMA6100P_Task_STK_SIZE,             /* 任务堆栈大小 */
                            (void*          )NULL,                               /* 传入给任务函数的参数 */
                            (UBaseType_t    )QMA6100P_Task_PRIO,                 /* 任务优先级 */
                            (TaskHandle_t*  )&QMA6100P_Task_Handler,             /* 任务句柄 */
                            (BaseType_t     ) 0);                                /* 该任务哪个内核运行 */
    if(Ret != pdPASS)
    {
        ESP_LOGD(TAG1, "QMA6100P_Task_Handler 创建失败\r\n");
        vTaskDeleteWithCaps(QMA6100P_Task_Handler);
    }
    else
        ESP_LOGD(TAG1, "QMA6100P_Task_Handler 创建成功\r\n");
    
}

static void QMA6100P_Rec_Task(void *pvParameters)
{
    QMA6100P_Config();
    vTaskDelay(100/portTICK_PERIOD_MS);

    while(1)
    {
        if((XL9555_Read_IO(Input_Reg_0) & 0x02) == 0)
        {
            QMA6100P_RawDataGet();
        }
        else
        {
            ESP_LOGI(TAG, "STEP COUNT寄存器值 = %d\r\n", STEP_COUNT_READ());
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
