#include "bsp_key.h"
#include "bsp_xl9555.h"
#include "bsp_led.h"
#include "esp_log.h"
#include "string.h"

#define TAG "KEY"

uint8_t Flag_KeySwitch = 0;

static void Key_Boot_GPIO_Init(void)
{
    gpio_config_t gpio_config_struct;

    gpio_config_struct.intr_type = GPIO_INTR_DISABLE;
    gpio_config_struct.mode = GPIO_MODE_INPUT;
    gpio_config_struct.pin_bit_mask = 1ull << KEY_BOOT_GPIO_PIN;
    gpio_config_struct.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config_struct.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&gpio_config_struct);
}

static void Key_Boot_KeyScan(uint8_t* Key_TimeBuf, uint8_t* Key_SelfLock)
{
    uint8_t times = 0;
    uint8_t Self_Lock = 0;

    Self_Lock = *Key_SelfLock;
    times = *Key_TimeBuf;

    if(KEY_BOOT_Read == PIN_RESET)
    {
        vTaskDelay(10);
        if(KEY_BOOT_Read == PIN_RESET)
        {
            if(Self_Lock == 0) times++;
        }
    } 
    else 
    {
        if(times)
            ESP_LOGI(TAG, "按下时间：%d *100ms\r\n\r\n", times);
        Self_Lock = 0;
    }

    *Key_SelfLock = Self_Lock;
    *Key_TimeBuf = times;
}

static void Key_Boot_KeyDownJudje(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf)
{
    uint8_t times = 0, ptimes = 0;

    times = *Key_TimeBuf;
    ptimes = *pKey_TimeBuf;

    if(ptimes != times)
    {
       ptimes = times;
       if(times >= 50)  times = 0;
    }
    else times = 0;

    *Key_TimeBuf = times;
    *pKey_TimeBuf = ptimes;
}

static uint8_t Key_Boot_KeyStatusCheck(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf, uint8_t* Key_SelfLock, StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t KeyStatus_Ret = 0;
    uint8_t times = 0, ptimes = 0;
    uint8_t Self_Lock = 0;

    times = *Key_TimeBuf;
    ptimes = *pKey_TimeBuf;
    Self_Lock = *Key_SelfLock;

    if(times == 0)
    {
        if(ptimes != 0)
        {
            ESP_LOGI(TAG, "ptimes = %d\r\n", ptimes);
            if(ptimes >= 50)
            {
                StructDef_KEY_Buf->KEY_BOOT.KEY_Status = KEY_LONG_DOWN;
            }
            else
            {
                StructDef_KEY_Buf->KEY_BOOT.KEY_Status = KEY_SHORT_DOWN;
            }
            KeyStatus_Ret = 1;
            Self_Lock = 1;
            ptimes = 0; 
        }
    }

    *pKey_TimeBuf = ptimes;
    *Key_SelfLock = Self_Lock;

    return KeyStatus_Ret;
}

static void XL9555_KeyScan(uint8_t* Key_TimeBuf, uint8_t* Key_SelfLock)
{
    uint8_t i = 0;
    uint8_t XL9555_ScanData = 0xFF;
    static uint16_t Times = 0;

    XL9555_ScanData = XL9555_Read_IO(Input_Reg_1);
    if(XL9555_ScanData != 0xFF)
    {
        /* 消抖 */
        vTaskDelay(10);
        XL9555_ScanData = XL9555_Read_IO(Input_Reg_1);

        if(XL9555_ScanData != 0xFF)
        {
            LED(1);

            for(i=0; i<4; i++)
            {
                if((0x80 & XL9555_ScanData) == 0)
                {
                    if(Key_SelfLock[i] == 0)
                    {          
                        Times++;      
                        Key_TimeBuf[i]++;
                    }
                }
                else
                {
                    Key_SelfLock[i] = 0;
                }
                XL9555_ScanData <<= 1;
            }
        }
    }
    else
    {
        LED(0);
        memset(Key_SelfLock, 0, 4);
        if(Times)
        {
            ESP_LOGI(TAG, "按下时间：%d *100ms\r\n\r\n", Times);
            Times = 0;
        }
    }
}

static uint8_t XL9555_KeyDownJudje(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf, StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t i = 0;
    uint8_t DownJudje_Ret = 0;
    static uint8_t SOS_Status = 0, PTT_Status = 0, UP_Status = 0, Down_Status = 0;
    
    for(i=0; i<4; i++)
    {
        /* Key_Group0 */
        if(Flag_KeySwitch == 0)
        {
            if(pKey_TimeBuf[i] != Key_TimeBuf[i])
            {
                pKey_TimeBuf[i] = Key_TimeBuf[i];
                switch(i)
                {
                    case 0: if(Key_TimeBuf[i] >= 50)  {Key_TimeBuf[i] = 0;}  break; /*KEY0*/
                    case 1: if(Key_TimeBuf[i] >= 20)  {Key_TimeBuf[i] = 0;}  break; /*KEY_POWER*/
                    case 2: if(Key_TimeBuf[i] >= 20)                                /*KEY_SOS*/
                            {
                                if(SOS_Status == 0)
                                {
                                    SOS_Status = 1;
                                    StructDef_KEY_Buf->KEY_SOS.KEY_Status = KEY_LONG_DOWN; 
                                    DownJudje_Ret++;
                                }
                                if(Key_TimeBuf[i] >= 100)
                                {
                                    Key_TimeBuf[i] = 0;
                                }
                            }  
                                                                             break;
                    case 3: if(Key_TimeBuf[i] >= 5)                                 /*KEY_PTT*/
                            {
                                if(PTT_Status == 0)
                                {
                                    PTT_Status = 1;
                                    StructDef_KEY_Buf->KEY_PTT.KEY_Status = KEY_SHORT_DOWN; 
                                    DownJudje_Ret++;
                                }

                                if(Key_TimeBuf[i] > 150)
                                {
                                    Key_TimeBuf[i] = 0; 
                                    PTT_Status = 0;
                                }
                            }
                                                                            break;
                    default: break;
                }
            }
            else
            {
                Key_TimeBuf[i] = 0;
                if(i == 2)
                    SOS_Status = 0;
                if(i == 3)
                    PTT_Status = 0;
            }
        }
        else /* Key_Group1 */
        {
            if(pKey_TimeBuf[i] != Key_TimeBuf[i])
            {
                pKey_TimeBuf[i] = Key_TimeBuf[i];
                switch(i)
                {
                    case 0: if(Key_TimeBuf[i] >= 50)  {Key_TimeBuf[i] = 0;} break; /*KEY0*/
                    case 1: if(Key_TimeBuf[i] >= 30)  {Key_TimeBuf[i] = 0;} break; /*KEY_OK*/
                    case 2: if(UP_Status == 0)
                            {
                                if(Key_TimeBuf[i] >= 30)
                                {   
                                    Key_TimeBuf[i] = 0; 
                                    UP_Status = 1;
                                }
                            }
                            else
                            {
                                if(Key_TimeBuf[i] >= 10)
                                {
                                    Key_TimeBuf[i] = 0; 
                                    pKey_TimeBuf[i] = 30;
                                }
                            }
                                                                                             break; /*KEY_UP*/
                    case 3: if(Down_Status == 0)
                            {
                                if(Key_TimeBuf[i] >= 30)  
                                {
                                    Key_TimeBuf[i] = 0; 
                                    Down_Status = 1;
                                }
                            }
                            else
                            {
                                if(Key_TimeBuf[i] >= 10)
                                {
                                    Key_TimeBuf[i] = 0; 
                                    pKey_TimeBuf[i] = 30;
                                }
                            } 
                                                                                            break; /*KEY_DOWN*/
                    default: break;
                }
            }
            else
            {
                Key_TimeBuf[i] = 0;
                if((i == 2) && (UP_Status))
                {
                    UP_Status = 0;
                    pKey_TimeBuf[i] = 0;
                }
                if((i == 3) && (Down_Status))
                {
                    Down_Status = 0;
                    pKey_TimeBuf[i] = 0;
                }
            }
        }
    }

    return DownJudje_Ret;
}

static uint8_t XL9555_KeyStatusCheck(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf, uint8_t* Key_SelfLock, StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t i = 0;
    uint8_t KeyStatus_Ret = 0;

    for(i=0; i<4; i++)
    {
        if(Key_TimeBuf[i] == 0)
        {
            if(pKey_TimeBuf[i] != 0)
            {
                 /* Key_Group0 */
                 if(Flag_KeySwitch == 0)
                 {
                    switch(i)
                    {
                        case 0: /* KEY0 */
                                if(pKey_TimeBuf[i] >= 50)
                                {
                                    StructDef_KEY_Buf->KEY0.KEY_Status = KEY_LONG_DOWN;
                                    Flag_KeySwitch = (uint8_t)((Flag_KeySwitch>0)?0:1);
                                }
                                else
                                {
                                    StructDef_KEY_Buf->KEY0.KEY_Status = KEY_SHORT_DOWN;
                                }

                                KeyStatus_Ret++;
                        break;

                        case 1: /* KEY_POWER */
                                if(pKey_TimeBuf[i] >= 20)
                                    StructDef_KEY_Buf->KEY_POWER.KEY_Status = KEY_LONG_DOWN;
                                else
                                    StructDef_KEY_Buf->KEY_POWER.KEY_Status = KEY_SHORT_DOWN;

                                KeyStatus_Ret++;
                        break;

                        case 2: /* KEY_SOS */
                                if(pKey_TimeBuf[i] >= 20)
                                {
                                    if (pKey_TimeBuf[i] >= 100)
                                    {
                                        StructDef_KEY_Buf->KEY_SOS.KEY_Status = KEY_LONG_LONG_DOWN;
                                        KeyStatus_Ret++;
                                    }
                                }

                        break;

                        case 3: /* KEY_PTT */
                                if(pKey_TimeBuf[i] >= 5)  
                                {
                                    if(pKey_TimeBuf[i] >= 150)
                                        StructDef_KEY_Buf->KEY_PTT.KEY_Status = KEY_TIMEOUT;

                                    KeyStatus_Ret++;
                                }
                                else
                                {
                                    /* 按键时间低于0.5s 默认为抖动 */
                                    if(KeyStatus_Ret) KeyStatus_Ret--;
                                }
                        break;
                        default: break;
                    }
                }
                else /* Key_Group1 */
                {
                    switch(i)
                    {
                        case 0: /* KEY0 */
                                if(pKey_TimeBuf[i] >= 50)
                                {
                                    StructDef_KEY_Buf->KEY0.KEY_Status = KEY_LONG_DOWN;
                                    Flag_KeySwitch = (uint8_t)((Flag_KeySwitch>0)?0:1);
                                }
                                else
                                {
                                    StructDef_KEY_Buf->KEY0.KEY_Status = KEY_SHORT_DOWN;
                                }

                                KeyStatus_Ret++;
                        break;

                        case 1: /* KEY_OK */
                                if(pKey_TimeBuf[i] >= 30)
                                {
                                    if(pKey_TimeBuf[2]) /* 判断 KEY_UP*/
                                    {
                                        pKey_TimeBuf[2] = 0;
                                        Key_TimeBuf[2] = 0;
                                        Key_SelfLock[2] = 1;
                                        StructDef_KEY_Buf->KEY_OK.KEY_Status = KEY_COMBINATE0;
                                    }
                                    else if(pKey_TimeBuf[3]) /* 判断 KEY_DOWN*/
                                    {
                                        pKey_TimeBuf[3] = 0;
                                        Key_TimeBuf[3] = 0;
                                        Key_SelfLock[3] = 1;
                                        StructDef_KEY_Buf->KEY_OK.KEY_Status = KEY_COMBINATE1;
                                    }
                                    else
                                    {
                                        StructDef_KEY_Buf->KEY_OK.KEY_Status = KEY_LONG_DOWN;
                                    }
                                }
                                else
                                {
                                    StructDef_KEY_Buf->KEY_OK.KEY_Status = KEY_SHORT_DOWN; 
                                }

                                KeyStatus_Ret++;
                        break;

                        case 2: /* KEY_UP */
                                if(pKey_TimeBuf[i] >= 30)
                                {
                                    if(pKey_TimeBuf[3]) /* 判断 KEY_DOWN*/
                                    {
                                        pKey_TimeBuf[3] = 0;
                                        Key_TimeBuf[3] = 0;
                                        Key_SelfLock[3] = 1;
                                        StructDef_KEY_Buf->KEY_UP.KEY_Status = KEY_COMBINATE0;
                                    }
                                    else if(pKey_TimeBuf[1]) /* 判断 KEY_OK*/
                                    {
                                        pKey_TimeBuf[1] = 0;
                                        Key_TimeBuf[1] = 0;
                                        Key_SelfLock[1] = 1;
                                        StructDef_KEY_Buf->KEY_UP.KEY_Status = KEY_COMBINATE1;
                                    }
                                    else
                                    {
                                        StructDef_KEY_Buf->KEY_UP.KEY_Status = KEY_LONG_DOWN;
                                    }
                                }
                                else
                                {
                                    StructDef_KEY_Buf->KEY_UP.KEY_Status = KEY_SHORT_DOWN; 
                                }

                                KeyStatus_Ret++;
                        break;

                         case 3: /* KEY_DOWN */
                                if(pKey_TimeBuf[i] >= 30)
                                {
                                    if(pKey_TimeBuf[2]) /* 判断 KEY_UP*/
                                    {
                                        pKey_TimeBuf[2] = 0;
                                        Key_TimeBuf[2] = 0;
                                        Key_SelfLock[2] = 1;
                                        StructDef_KEY_Buf->KEY_DOWN.KEY_Status = KEY_COMBINATE0;
                                    }
                                    else if(pKey_TimeBuf[1]) /* 判断 KEY_OK*/
                                    {
                                        pKey_TimeBuf[1] = 0;
                                        Key_TimeBuf[1] = 0;
                                        Key_SelfLock[1] = 1;
                                        StructDef_KEY_Buf->KEY_DOWN.KEY_Status = KEY_COMBINATE1;
                                    }
                                    else
                                    {
                                        StructDef_KEY_Buf->KEY_DOWN.KEY_Status = KEY_LONG_DOWN;
                                    }
                                }
                                else
                                {
                                    StructDef_KEY_Buf->KEY_DOWN.KEY_Status = KEY_SHORT_DOWN; 
                                }

                                KeyStatus_Ret++;
                        break;
                        default: break;
                    }
                }

                pKey_TimeBuf[i] = 0;
                Key_SelfLock[i] = 1;

                if(Flag_KeySwitch == 0)
                {
                    
                }
                else
                {
                    if(i == 2)
                    {
                        if(StructDef_KEY_Buf->KEY_UP.KEY_Status == KEY_LONG_DOWN)
                        {
                            Key_SelfLock[i] = 0;
                        }
                    }
                    else if(i == 3)
                    {
                        if(StructDef_KEY_Buf->KEY_DOWN.KEY_Status == KEY_LONG_DOWN)
                        {
                            Key_SelfLock[i] = 0;
                        }
                    }
                }
            }
        }
    }

    return KeyStatus_Ret;
}

static void KeyScan(uint8_t* Key_TimeBuf, uint8_t* Key_SelfLock)
{
    Key_Boot_KeyScan(Key_TimeBuf+4, Key_SelfLock+4);
    XL9555_KeyScan(Key_TimeBuf, Key_SelfLock);
}

static uint8_t KeyDownJudje(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf, StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t DownJudje_Ret = 0;

    Key_Boot_KeyDownJudje(Key_TimeBuf+4, pKey_TimeBuf+4);
    DownJudje_Ret |= XL9555_KeyDownJudje(Key_TimeBuf, pKey_TimeBuf, StructDef_KEY_Buf);

    return DownJudje_Ret;
}

static uint8_t KeyStatusCheck(uint8_t* Key_TimeBuf, uint8_t* pKey_TimeBuf, uint8_t* Key_SelfLock, StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t KeyStatus_Ret = 0;

    KeyStatus_Ret |= Key_Boot_KeyStatusCheck(Key_TimeBuf+4, pKey_TimeBuf+4, Key_SelfLock+4, StructDef_KEY_Buf);
    KeyStatus_Ret |= XL9555_KeyStatusCheck(Key_TimeBuf, pKey_TimeBuf, Key_SelfLock, StructDef_KEY_Buf);

    return KeyStatus_Ret;
}

uint8_t Total_KeyScan(StructDef_KEY* StructDef_KEY_Buf)
{
    uint8_t KeyStatus_Ret = 0;
    static uint8_t Key_Down_Times[5] = {0}, pKey_Down_Times[5] = {0};
    static uint8_t Key_SelfLock[5] = {0};

    KeyScan(&Key_Down_Times, &Key_SelfLock);
    KeyStatus_Ret = KeyDownJudje(&Key_Down_Times, &pKey_Down_Times, StructDef_KEY_Buf);
    KeyStatus_Ret += KeyStatusCheck(&Key_Down_Times, &pKey_Down_Times, &Key_SelfLock, StructDef_KEY_Buf);

    return KeyStatus_Ret;
}

void Key_Init(void)
{
    Key_Boot_GPIO_Init();
}