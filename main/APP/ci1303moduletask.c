#include "ci1303moduletask.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/FreeRTOS.h"

#include "esp_log.h"

#include "bsp_usart.h"
#include "string.h" 

#define TAG "CI1303"
#define TAG1 "FreeRTOS"

/* CI1303_USART_Task_Handler 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define CI1303_USART_Task_PRIO      3                          /* 任务优先级 */
#define CI1303_USART_Task_STK_SIZE  1024*3                     /* 任务堆栈大小 */
TaskHandle_t CI1303_USART_Task_Handler;                        /* 任务句柄 */
static void CI1303_USART_Rec_Task(void *pvParameters);         /* 任务函数 */


/* USART_Task 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define USART_Action_Task_PRIO      3                          /* 任务优先级 */
#define USART_Action_Task_STK_SIZE  1024*3                     /* 任务堆栈大小 */
TaskHandle_t USART_Action_Task_Handler;                        /* 任务句柄 */
static void CI1303_USART_Action_Task(void *pvParameters);      /* 任务函数 */


/* VOICE_Queue 队列 配置
 * 包括: 队列句柄 队列长度 队列项目长度
 */
#define VOICE_Queue_Length    160                         /* 队列长度 */
#define VOICE_Queue_ItemSize  1                           /* 队列项目长度 */
QueueHandle_t VOICE_Queue;                                /* 队列句柄 */
static void CI1303_DataJudge(uint8_t* RecBuf, uint8_t RecLen, Enum_VoiceCmd* VoiceIpt_Cmd);

static void CI1303_USART_Rec_Task(void *pvParameters)
{
    pvParameters = pvParameters;
    uint8_t i = 0;
    uint8_t RecLen = 0;
    uint8_t Rec_Buf[RX_BUF_SIZE] = {0};
    TickType_t xLastWakeTime;
    BaseType_t Queue_ret = pdPASS;  //检查数列是否成功写入
    Enum_VoiceCmd VoiceIpt_Cmd[20] = {VoiceIpt_None};
    
    ESP_LOGI(TAG,"USART_Task Start\r\n");

    xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        i = 0;

        uart_get_buffered_data_len(USART_UX, (size_t*) &RecLen);

        if(RecLen)
        {
            memset(Rec_Buf, 0, RX_BUF_SIZE);
            memset(VoiceIpt_Cmd, VoiceIpt_None, 20*sizeof(Enum_VoiceCmd));
            uart_read_bytes(USART_UX, Rec_Buf, RecLen, 0);

            CI1303_DataJudge(Rec_Buf, RecLen, VoiceIpt_Cmd);
            ESP_LOGD(TAG, "VoiceIpt_Cmd[0] = %d\r\n", VoiceIpt_Cmd[0]);
            ESP_LOGD(TAG, "i = %d\r\n", i);
            while(VoiceIpt_Cmd[i] != VoiceIpt_None)
            {
                 if((Queue_ret = xQueueSend(VOICE_Queue, &VoiceIpt_Cmd[i], 0)) != pdPASS)
                 {
                    ESP_LOGE(TAG, "VOICE_Queue 写队列错误 ErrorCode: %d\r\n", Queue_ret);
                 }
                 i++;
            }
            for(i=0; i<RecLen; i++)
            {
                ESP_LOGD(TAG, "%02X\t", Rec_Buf[i]);
            }
            ESP_LOGD(TAG, "\r\n");
        }

        vTaskDelayUntil(&xLastWakeTime, 100);
    }
}

void CI1303_USART_Rec_Start(void)
{
    BaseType_t Ret = pdPASS;

/* 创建语音队列 */
    VOICE_Queue = xQueueCreate(VOICE_Queue_Length,     /* 队列项目长度 */
                            VOICE_Queue_ItemSize);     /* 队列项目长度 */


 /* 创建任务 */
    Ret = xTaskCreatePinnedToCore((TaskFunction_t )CI1303_USART_Rec_Task,            /* 任务函数 */
                            (const char*    )"CI1303_USART_Rec_Task",                /* 任务名称 */
                            (uint16_t       )CI1303_USART_Task_STK_SIZE,             /* 任务堆栈大小 */
                            (void*          )NULL,                                   /* 传入给任务函数的参数 */
                            (UBaseType_t    )CI1303_USART_Task_PRIO,                 /* 任务优先级 */
                            (TaskHandle_t*  )&CI1303_USART_Task_Handler,             /* 任务句柄 */
                            (BaseType_t     ) 0);                                    /* 该任务哪个内核运行 */
    if(Ret != pdPASS)
    {
        ESP_LOGD(TAG1, "CI1303_USART_Rec_Task 创建失败\r\n");
        vTaskDeleteWithCaps(CI1303_USART_Task_Handler);
    }
    else
        ESP_LOGI(TAG1, "CI1303_USART_Rec_Task 创建成功\r\n");
}

static void CI1303_USART_Action_Task(void *pvParameters)
{
    pvParameters = pvParameters;
    TickType_t xLastWakeTime;
    BaseType_t Queue_ret = pdPASS;
    Enum_VoiceCmd VoiceIpt_Cmd = VoiceIpt_None;

    ESP_LOGD(TAG, "CI1303_USART_Action_Task\r\n");
    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        if((Queue_ret = xQueueReceive(VOICE_Queue, &VoiceIpt_Cmd, portMAX_DELAY)) == pdPASS)
        {
            ESP_LOGE(TAG, "VoiceIpt_Cmd = %d\r\n", VoiceIpt_Cmd);
        }

        vTaskDelayUntil(&xLastWakeTime, 100);
    }
}

void CI1303_USART_Action_Start(void)
{
    BaseType_t Ret = pdPASS;

    Ret = xTaskCreatePinnedToCore((TaskFunction_t )CI1303_USART_Action_Task,         /* 任务函数 */
                            (const char*    )"CI1303_USART_Action_Task",             /* 任务名称 */
                            (uint16_t       )USART_Action_Task_STK_SIZE,             /* 任务堆栈大小 */
                            (void*          )NULL,                                   /* 传入给任务函数的参数 */
                            (UBaseType_t    )USART_Action_Task_PRIO,                 /* 任务优先级 */
                            (TaskHandle_t*  )&USART_Action_Task_Handler,             /* 任务句柄 */
                            (BaseType_t     ) 0);                                    /* 该任务哪个内核运行 */
    if(Ret != pdPASS)
    {
        ESP_LOGE(TAG1, "CI1303_USART_Action_Start 创建失败\r\n");
        vTaskDeleteWithCaps(USART_Action_Task_Handler);
    }
    else
        ESP_LOGI(TAG1, "CI1303_USART_Action_Start 创建成功\r\n");
}

static void CI1303_DataJudge(uint8_t* RecBuf, uint8_t RecLen, Enum_VoiceCmd* VoiceIpt_Cmd)
{
    uint8_t i = 0, j = 0;
    uint8_t Add_Check = 0;

    if((RecLen % CI1303_CmdLen) != 0)
    {
        ESP_LOGE(TAG, "指令长度错误, RecLen = %d\r\n", RecLen);
        return;
    }
    ESP_LOGD(TAG, "CI1303_CmdLen = %d, RecLen = %d\r\n", CI1303_CmdLen, RecLen);

    for(i=0; i<(RecLen/CI1303_CmdLen); i++)
    {
        if((RecBuf[i*CI1303_CmdLen] != 0xA5) 
        || (RecBuf[i*CI1303_CmdLen + 1] != 0xFA) 
        || (RecBuf[i*CI1303_CmdLen + 2] != 0x00)
        || (RecBuf[i*CI1303_CmdLen + 3] != 0x81)
        || (RecBuf[i*CI1303_CmdLen + 7] != 0xFB))
        {
            ESP_LOGE(TAG, "第%d条指令, 协议头/尾错误\r\n", i);
            continue;
        }

        Add_Check = ((RecBuf[i*CI1303_CmdLen] + RecBuf[i*CI1303_CmdLen + 1] + RecBuf[i*CI1303_CmdLen + 2] + RecBuf[i*CI1303_CmdLen + 3] + RecBuf[i*CI1303_CmdLen + 4]) & 0xFF);

        if(Add_Check != RecBuf[i*CI1303_CmdLen + 6])
        {
            ESP_LOGE(TAG, "第%d条指令, 和校验错误\r\n", i);
            continue;
        }

        switch(RecBuf[i*CI1303_CmdLen + 4])
        {
            case 0x01: VoiceIpt_Cmd[j] = VoiceIpt_Wakeup; break;
            case 0x02: VoiceIpt_Cmd[j] = VoiceIpt_AddVoiceLevel; break;
            case 0x03: VoiceIpt_Cmd[j] = VoiceIpt_DecVoiceLevel; break;
            case 0x04: VoiceIpt_Cmd[j] = VoiceIpt_AddVoiceLevelToMax; break;
            case 0x05: VoiceIpt_Cmd[j] = VoiceIpt_DecVoiceLevelToMin; break;
            case 0x06: VoiceIpt_Cmd[j] = VoiceIpt_Safe; break;
            case 0x07: VoiceIpt_Cmd[j] = VoiceIpt_FindTerminal; break;
            case 0x08: VoiceIpt_Cmd[j] = VoiceIpt_FindBaseStation; break;
            case 0x09: VoiceIpt_Cmd[j] = VoiceIpt_PrepareToBaseStation; break;
            case 0x0A: VoiceIpt_Cmd[j] = VoiceIpt_ArriveBaseStation; break;
            case 0x0B: VoiceIpt_Cmd[j] = VoiceIpt_SendVoice; break;
            case 0x0C: VoiceIpt_Cmd[j] = VoiceIpt_PlayVoice; break;
            case 0x0D: VoiceIpt_Cmd[j] = VoiceIpt_ChangeGroup; break;
            case 0x0E: VoiceIpt_Cmd[j] = VoiceIpt_SOS; break;
            case 0x0F: VoiceIpt_Cmd[j] = VoiceIpt_RelieveAlarm; break;
            case 0x10: VoiceIpt_Cmd[j] = VoiceIpt_PowerCheck; break;
            case 0x11: VoiceIpt_Cmd[j] = VoiceIpt_TurnOnLED; break;
            case 0x12: VoiceIpt_Cmd[j] = VoiceIpt_TurnOffLED; break;
            case 0x13: VoiceIpt_Cmd[j] = VoiceIpt_TurnOffSys; break;
            case 0x14: VoiceIpt_Cmd[j] = VoiceIpt_UpDataSys; break;
            default: break;
        }

        j++;
    }
}
