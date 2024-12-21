#include "keymoduletask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "bsp_key.h"

#define TAG "KEY"
#define TAG1 "FreeRTOS"


/* KEY_JudgeTask 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define KEY_Judge_PRIO      3                           /* 任务优先级 */
#define KEY_Judge_STK_SIZE  1024*3                      /* 任务堆栈大小 */
TaskHandle_t            KEY_JudgeTask_Handler;          /* 任务句柄 */
void KEY_JudgeTask(void *pvParameters);                 /* 任务函数 */

/* KEY_Queue 队列 配置
 * 包括: 队列句柄 队列长度 队列项目长度
 */
#define KEY_Queue_Length    160                         /* 队列长度 */
#define KEY_Queue_ItemSize  1                           /* 队列项目长度 */
QueueHandle_t KEY_Queue;                                /* 队列句柄 */

/* KEY_Timer 定时器 配置
 * 包括: 队列句柄 队列长度 队列项目长度
 */
#define KEY_Timer_ID        0                           /* 定时器ID */
#define KEY_Timer_Period    100/portTICK_PERIOD_MS      /* 定时器周期 */
TimerHandle_t KEY_Timer_Handler;                        /* 定时器句柄 */
void KEY_ScanTimer(void *pvParameters);

static void KEY0_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;

    /* KEY0 键值判断 */
    if(StructDef_KEY_RecBuf->KEY0.KEY_Status == KEY_SHORT_DOWN)
    {
        Ret_KeyScanNum--;
        ESP_LOGI(TAG, "KEY0 短按\r\n");
    }
    else if(StructDef_KEY_RecBuf->KEY0.KEY_Status == KEY_LONG_DOWN)
    {
        Ret_KeyScanNum--;
        ESP_LOGI(TAG,"/*********************************************\r\n");
        if(Flag_KeySwitch == 0)
        {
            ESP_LOGI(TAG,"KEY0 长按，已经切换到第一套按键\r\n");
            ESP_LOGI(TAG,"当前测试:\r\n 电源键(KEY1)\r\n PTT键(KEY2)\r\n SOS键(KEY3)\r\n 按键功能\r\n");
            ESP_LOGI(TAG,"如需测试 OK键、上键、下键功能, 请长按 KEY0键 5秒\r\n");
        }
        else
        {
            ESP_LOGI(TAG,"KEY0 长按，已经切换到第二套按键\r\n");
            ESP_LOGI(TAG,"当前测试:\r\n OK键(KEY1)\r\n 上键(KEY2)\r\n 下键(KEY3)\r\n 等按键功能\r\n");
            ESP_LOGI(TAG,"如需测试: 电源键、PTT键、SOS键功能, 请长按 KEY0键 5秒\r\n");
        }
       ESP_LOGI(TAG,"*********************************************/\r\n");
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void Boot_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;

    if(StructDef_KEY_RecBuf->KEY_BOOT.KEY_Status == KEY_SHORT_DOWN)
    {
        Ret_KeyScanNum--;
        ESP_LOGI(TAG, "KEY_BOOT 短按\r\n");
    }
    else if(StructDef_KEY_RecBuf->KEY_BOOT.KEY_Status == KEY_LONG_DOWN)
    {
        Ret_KeyScanNum--;
        ESP_LOGI(TAG, "KEY_BOOt 长按\r\n");
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void Power_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;

    /* KEY_POWER 键值判断 */
    if (StructDef_KEY_RecBuf->KEY_POWER.KEY_Status == KEY_SHORT_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_POWER 短按\r\n");
        Ret_KeyScan = KEY_POWER_SHORTDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if (StructDef_KEY_RecBuf->KEY_POWER.KEY_Status == KEY_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_POWER 长按\r\n");
        Ret_KeyScan = KEY_POWER_LONG;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void SOS_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;
    /* KEY_SOS 键值判断 */
    if (StructDef_KEY_RecBuf->KEY_SOS.KEY_Status == KEY_LONG_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_SOS 长按10s\r\n");
        Ret_KeyScan = KEY_SOS_LONG_LONGDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if (StructDef_KEY_RecBuf->KEY_SOS.KEY_Status == KEY_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_SOS 长按2s\r\n");
        Ret_KeyScan = KEY_SOS_LONGDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void OK_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;
    
 /* KEY_OK 键值判断 */
    if (StructDef_KEY_RecBuf->KEY_OK.KEY_Status == KEY_SHORT_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_OK 短按\r\n");
        Ret_KeyScan = KEY_OK_SHORTDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if (StructDef_KEY_RecBuf->KEY_OK.KEY_Status == KEY_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_OK 长按\r\n");
        Ret_KeyScan = KEY_OK_LONGDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if (StructDef_KEY_RecBuf->KEY_OK.KEY_Status == KEY_COMBINATE0)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_OK+KEY_UP 组合按键\r\n");
        Ret_KeyScan = KEY_OK_ADD_KEY_UP;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if (StructDef_KEY_RecBuf->KEY_OK.KEY_Status == KEY_COMBINATE1)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_OK+KEY_DOWN 组合按键\r\n");
        Ret_KeyScan = KEY_OK_ADD_KEY_DOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void UP_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;
/* KEY_UP 键值判断 */
    if(StructDef_KEY_RecBuf->KEY_UP.KEY_Status == KEY_SHORT_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_UP 短按\r\n");
        Ret_KeyScan = KEY_UP_SHORTDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_UP.KEY_Status == KEY_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_UP 长按\r\n");
        Ret_KeyScan = KEY_UP_LONGDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret); 
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_UP.KEY_Status == KEY_COMBINATE0)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_UP+KEY_DOWN 组合按键\r\n");
        Ret_KeyScan = KEY_UP_ADD_KEY_DOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_UP.KEY_Status == KEY_COMBINATE1)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_UP+KEY_OK 组合按键\r\n");
        Ret_KeyScan = KEY_OK_ADD_KEY_UP;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void DOWN_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;
     /* KEY_DOWN 键值判断 */
    if(StructDef_KEY_RecBuf->KEY_DOWN.KEY_Status == KEY_SHORT_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_DOWN 短按\r\n");
        Ret_KeyScan = KEY_DOWN_SHORTDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_DOWN.KEY_Status == KEY_LONG_DOWN)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_DOWN 长按\r\n");
        Ret_KeyScan = KEY_DOWN_LONGDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_DOWN.KEY_Status == KEY_COMBINATE0)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_DOWN+KEY_UP 组合按键\r\n");
        Ret_KeyScan = KEY_UP_ADD_KEY_DOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_DOWN.KEY_Status == KEY_COMBINATE1)
    {
        Key_ScanNum--;
        ESP_LOGI(TAG, "KEY_DOWN+KEY_OK 组合按键\r\n");
        Ret_KeyScan = KEY_OK_ADD_KEY_DOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

static void PTT_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t* Ret_KeyScanNum)
{
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_RetKeyScan Ret_KeyScan = 0;        //按键键值返回变量
    static uint8_t PPT_Status = 0;  //记录PPT按键的按键状态，在按键松开时发送一次松开键值
    uint8_t Key_ScanNum = 0;

    Key_ScanNum = *Ret_KeyScanNum;
     /* KEY_PTT 键值判断 */
    if(StructDef_KEY_RecBuf->KEY_PTT.KEY_Status == KEY_SHORT_DOWN)
    {
        Key_ScanNum--;
        PPT_Status = 1;
        ESP_LOGI(TAG, "KEY_PTT 按下\r\n");
        Ret_KeyScan = KEY_PTT_SHORTDOWN;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_PTT.KEY_Status == KEY_TIMEOUT)
    {
        Key_ScanNum--;
        PPT_Status = 0;
        ESP_LOGI(TAG, "KEY_PTT 按下超过15s 按键超时\r\n");
        Ret_KeyScan = KEY_PTT_TIMEOUT;
        if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
        {
            ESP_LOGE(TAG,"KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
        }
    }
    else if(StructDef_KEY_RecBuf->KEY_PTT.KEY_Status == KEY_RELEASE)
    {
        if(Key_ScanNum)
        {
            if(PPT_Status)
            {
                PPT_Status = 0;
                ESP_LOGI(TAG, "KEY_PTT 松开\r\n");
                Ret_KeyScan = KEY_PTT_RELEASE;
                if((Queue_ret = xQueueSendToFrontFromISR(KEY_Queue, &Ret_KeyScan, 0)) != pdPASS)
                {
                    ESP_LOGE(TAG, "KEY_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
                }
            }
        }
    }
    Ret_KeyScanNum = &Key_ScanNum;
}

/**
 * @brief       KEY_ValGet
 * @param       StructDef_KEY_RecBuf : 按键状态结构体
 * @param       Ret_KeyScanNum : 应该产生的按键键值数量
 * @retval      无
 */
static void KEY_ValGet(StructDef_KEY* StructDef_KEY_RecBuf, uint8_t Ret_KeyScanNum)
{
    if(Ret_KeyScanNum)
        KEY0_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);

    if(Ret_KeyScanNum)
        Boot_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);

    if(Ret_KeyScanNum)
        Power_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
    
    if(Ret_KeyScanNum)
        SOS_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
    
    if(Ret_KeyScanNum)
        OK_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
    
    if(Ret_KeyScanNum)
        UP_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
    
    if(Ret_KeyScanNum)
        DOWN_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
    
    if(Ret_KeyScanNum)
        PTT_ValGet(StructDef_KEY_RecBuf, &Ret_KeyScanNum);
}


/**
 * @brief       KEY_ScanTimer
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void KEY_ScanTimer(void *pvParameters)
{
    pvParameters = pvParameters;
    StructDef_KEY KEY_Buf_Structure = {KEY_RELEASE};
    uint8_t Ret_KeyScanNum = 0;

    if((Ret_KeyScanNum = Total_KeyScan(&KEY_Buf_Structure)) != 0)
    {
        KEY_ValGet(&KEY_Buf_Structure, Ret_KeyScanNum);
    }
}

void KEYScan_Start(void)
{
    BaseType_t Ret = pdPASS;

 /* 创建按键队列 */
    KEY_Queue = xQueueCreate(KEY_Queue_Length,      /* 队列项目长度 */
                            KEY_Queue_ItemSize);    /* 队列项目长度 */

 /* 创建定时器 */
    KEY_Timer_Handler = xTimerCreate((const char*   )"KEY_ScanTimer",           /* 定时器名称 */
                            (const TickType_t       )KEY_Timer_Period,          /* 定时器定时周期 */
                            (const BaseType_t       )Timer_AutoReloadEnable,    /* 定时器自动重装载 */
                            (void * const           )KEY_Timer_ID,              /* 定时器ID */
                            (TimerCallbackFunction_t) KEY_ScanTimer);           /* 定时器任务 */
    
    if(KEY_Timer_Handler != NULL)
    {
        ESP_LOGI(TAG1, "KEY_Timer_Handler 创建成功\r\n");
        Ret = xTimerStart(KEY_Timer_Handler, 0);
        if(Ret != pdPASS)
        {
            ESP_LOGE(TAG1,"KEY_Timer_Handler 启动失败\r\n");
        }
        else
        {
            ESP_LOGI(TAG1,"KEY_Timer_Handler 启动成功\r\n");
        }
    }
    else
    {
        ESP_LOGI(TAG1, "KEY_Timer_Handler 创建失败\r\n");
    }
}

/**
 * @brief       KEY_JudgeTask
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void KEY_JudgeTask(void *pvParameters)
{
    pvParameters = pvParameters;
    TickType_t xLastWakeTime;
    BaseType_t Queue_ret = pdPASS;
    Enum_RetKeyScan Queue_KeyVal = 0;
    
    ESP_LOGI(TAG, "KEY_JudgeTask Start\r\n");
    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        if((Queue_ret = xQueueReceive(KEY_Queue, &Queue_KeyVal, portMAX_DELAY)) == pdPASS)
        {
            ESP_LOGI(TAG,"Queue_KeyVal = %d\r\n", Queue_KeyVal);
        }

        vTaskDelayUntil(&xLastWakeTime, 100);
    }
}

void KEYScan_Action_Start(void)
{
    BaseType_t Ret = pdPASS;

 /* 创建任务 */
    Ret = xTaskCreatePinnedToCore((TaskFunction_t )KEY_JudgeTask,           /* 任务函数 */
                            (const char*    )"KEY_JudgeTask",               /* 任务名称 */
                            (uint16_t       )KEY_Judge_STK_SIZE,            /* 任务堆栈大小 */
                            (void*          )NULL,                          /* 传入给任务函数的参数 */
                            (UBaseType_t    )KEY_Judge_PRIO,                /* 任务优先级 */
                            (TaskHandle_t*  )&KEY_JudgeTask_Handler,        /* 任务句柄 */
                            (BaseType_t     ) 0);                           /* 该任务哪个内核运行 */
    
    if(Ret != pdPASS)
    {
        ESP_LOGI(TAG1, "KEY_JudgeTask 创建失败\r\n");
        vTaskDeleteWithCaps(KEY_JudgeTask_Handler);
    }
    else
        ESP_LOGI(TAG1, "KEY_JudgeTask 创建成功\r\n");
}