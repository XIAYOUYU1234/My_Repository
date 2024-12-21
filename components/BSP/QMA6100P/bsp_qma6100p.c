#include "bsp_qma6100p.h"
#include "bsp_i2c.h"

#include "math.h"

#include "esp_log.h"
#define TAG "QMA6100P"

static esp_err_t QMA6100P_Turn_On_Sequence(void);
static esp_err_t Step_Init_Config(void);

static esp_err_t QMA6100P_ReadAndWrite(QMA6100P_Reg_Enum Reg, uint8_t* WriteBuf, uint8_t WriteLen, uint8_t* ReadBuf, uint8_t ReadLen, uint8_t ActionFlag)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t data_len = 0;

    i2c_cmd_handle_t QMA6100P_I2C_Cmd_Handle = i2c_cmd_link_create(); 

    i2c_master_start(QMA6100P_I2C_Cmd_Handle);
    i2c_master_write_byte(QMA6100P_I2C_Cmd_Handle, QMA6100P_WRITE, 1);
    i2c_master_write_byte(QMA6100P_I2C_Cmd_Handle, Reg, 1);
    data_len += 2;

    if(ActionFlag & FLAG_QMA6100A_WRITE)
    {
        if(WriteLen == 0)
        {
            ESP_LOGE(TAG, "写操作但WriteLen = 0\r\n");
            return ret = ESP_FAIL;
        }

        data_len += WriteLen;
        if(WriteLen > 1)
            i2c_master_write(QMA6100P_I2C_Cmd_Handle, WriteBuf, WriteLen-1, 1);
        
        i2c_master_write_byte(QMA6100P_I2C_Cmd_Handle, *(WriteBuf + WriteLen - 1), 1);
    }
    else if(ActionFlag & FLAG_QMA6100A_READ)
    {
        if(ReadLen == 0)
        {
            ESP_LOGE(TAG, "读操作但ReadLen = 0\r\n");
            return ret = ESP_FAIL;
        }

        data_len += ReadLen;
        i2c_master_start(QMA6100P_I2C_Cmd_Handle);
        i2c_master_write_byte(QMA6100P_I2C_Cmd_Handle, QMA6100P_READ, 1);
        if(ReadLen > 1)
            i2c_master_read(QMA6100P_I2C_Cmd_Handle, ReadBuf, ReadLen-1, 0);
        
        i2c_master_read_byte(QMA6100P_I2C_Cmd_Handle, (ReadBuf + ReadLen - 1), 1);
    }

    i2c_master_stop(QMA6100P_I2C_Cmd_Handle); 

    ret = i2c_master_cmd_begin(QMA6100P_I2C, QMA6100P_I2C_Cmd_Handle, 100 * (1 + data_len) / portTICK_PERIOD_MS);             /* 触发I2C控制器执行命令链接,即命令发送 */

    i2c_cmd_link_delete(QMA6100P_I2C_Cmd_Handle);    

    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_cmd_begin错误, ErrorCode = %d\r\n", ret);
        return ret;
    }
    else
        return ret = ESP_OK;                                                                       /* 释放命令链接使用的资源 */
}

void QMA6100P_RawDataGet(void)
{
    uint8_t i = 0;
    uint8_t ReadBuf[6] = {0};
    short RawDataBuf[3] = {0};
    float Acc_Buf[3] = {0};
    float Div_Buf[3] = {0};
    double Total_Acc = 0;
    double Wonder_Acc = 0;
    esp_err_t ret = ESP_FAIL;
    float Pitch_Angle = 0, Roll_Angle = 0, Yaw_Angle = 0; //俯仰角（Pitch Angle） 横滚角（Roll Angle） 偏航角（Yaw Angle）

    ret = QMA6100P_ReadAndWrite(X_OUT_L_REGISTER, NULL, 0, ReadBuf, 6, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return;
    ESP_LOGI(TAG, "QMA6100P 获取各轴的加速度原始数据\r\n");

    for(i=0; i<3; i++)
    {
        RawDataBuf[i] = ReadBuf[2*i] | (ReadBuf[2*i+1] << 8);
        if((RawDataBuf[i] & 0x0001) == 0x0001)
        {
            RawDataBuf[i] >>= 2;
            //977： 1adc = 977ug
            //1g = M_G
            Acc_Buf[i] = (float)(RawDataBuf[i] * 977 * M_G / 1000000);   //单位 (m/s^2)

            switch(i)
            {
                case 0: ESP_LOGI(TAG, "QMA6100P X轴传感器原始数据: RawDataBuf[%d] = %d\r\n", i, RawDataBuf[i]); 
                        ESP_LOGI(TAG, "QMA6100P X轴加速度数据: Acc_Buf[%d] = %f\r\n", i, Acc_Buf[i]); 
                    break;
                case 1: ESP_LOGI(TAG, "QMA6100P Y轴传感器原始数据: RawDataBuf[%d] = %d\r\n", i, RawDataBuf[i]); 
                        ESP_LOGI(TAG, "QMA6100P Y轴加速度数据: Acc_Buf[%d] = %f\r\n", i, Acc_Buf[i]); 
                    break;
                case 2: ESP_LOGI(TAG, "QMA6100P Z轴传感器原始数据: RawDataBuf[%d] = %d\r\n", i, RawDataBuf[i]); 
                        ESP_LOGI(TAG, "QMA6100P Z轴加速度数据: Acc_Buf[%d] = %f\r\n", i, Acc_Buf[i]); 
                    break;

                default: break;
            }
        }
    }
    Total_Acc = sqrt(Acc_Buf[0] * Acc_Buf[0] + Acc_Buf[1] * Acc_Buf[1] + Acc_Buf[2] * Acc_Buf[2]);
    ESP_LOGI(TAG, "QMA6100P 加速度合力取模: Total_Acc = %f\r\n", Total_Acc); 

    Div_Buf[0] = Acc_Buf[0] / Total_Acc;
    Div_Buf[1] = Acc_Buf[1] / Total_Acc;
    Div_Buf[2] = Acc_Buf[2] / Total_Acc;

    Pitch_Angle = -atan2f(Acc_Buf[0] , Acc_Buf[2]) * RAD_TO_DEG;

    Wonder_Acc = sqrtf(Div_Buf[0] * Div_Buf[0] + Div_Buf[1] * Div_Buf[1] + Div_Buf[2] * Div_Buf[2]);

    Roll_Angle = asinf((Div_Buf[1] / Wonder_Acc)) * RAD_TO_DEG ;


    ESP_LOGI(TAG, "QMA6100P 俯仰角: Pitch_Angle = %f\r\n", Pitch_Angle); 
    ESP_LOGI(TAG, "QMA6100P 横滚角: Total_Acc = %f\r\n", Roll_Angle); 
    // ESP_LOGI(TAG, "QMA6100P 偏航角: Total_Acc = %f\r\n", Yaw_Angle); 
}

esp_err_t QMA6100P_Config(void)
{
    uint8_t Chip_ID = 0;
    uint8_t Reg_SetVal = 0;
    esp_err_t ret = ESP_FAIL;

    ret = QMA6100P_ReadAndWrite(CHIP_ID_REGISTER, NULL, 0, &Chip_ID, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   
        return ret = ESP_FAIL;
    if(Chip_ID == 0x90)
    {
        ESP_LOGI(TAG, "QMA6100P 芯片ID为: 0x%02X\r\n", Chip_ID);
    }
    else
    {
        ESP_LOGI(TAG, "QMA6100P 芯片ID错误\r\n");
        return ret = ESP_FAIL;
    }

    if(QMA6100P_Turn_On_Sequence() != ESP_OK)
        return ret = ESP_FAIL;

    Reg_SetVal = 0x04;
    ret = QMA6100P_ReadAndWrite(RANGE_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 加速度传感器传感器精度配置:+/-8g\r\n");

    Reg_SetVal = 0x00;
    ret = QMA6100P_ReadAndWrite(OUTPUT_DATA_RATE_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 数据输出速率配置:100Hz\r\n");

    ret = Step_Init_Config();
    if(ret != ESP_OK)   
    {
        ESP_LOGE(TAG, "QMA6100P 中断配置失败\r\n");
        return ret = ESP_FAIL;
    }
    return ret = ESP_OK;
}

static esp_err_t QMA6100P_Turn_On_Sequence(void)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t Reg_ReadVal = 0;
    uint8_t Reg_SetVal = 0;
    uint8_t TimeOut = 0;

    Reg_SetVal = 0x80;
    ret = QMA6100P_ReadAndWrite(SW_RESET_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 上电软复位\r\n");
    vTaskDelay(10/portTICK_PERIOD_MS);
    TimeOut = 10;
    do
    {
        TimeOut--;
        ret = QMA6100P_ReadAndWrite(NVM_REGISTER, NULL, 0, &Reg_ReadVal, 1, FLAG_QMA6100A_READ);
        ESP_LOGI(TAG, "QMA6100P NVM_REGISTER:0x%02X\r\n", Reg_ReadVal);
        if(ret != ESP_OK)   return ret = ESP_FAIL;
        ESP_LOGI(TAG, "QMA6100P 上电软复位完成判断，判断次数：%d\r\n", 10 - TimeOut);
        vTaskDelay(10/portTICK_PERIOD_MS);
    } while (((Reg_ReadVal & 0x05) != 0x05) && (TimeOut));

    if(TimeOut == 0)    
    {
        ESP_LOGI(TAG, "QMA6100P 上电软复位失败\r\n");
        return ret = ESP_FAIL;
    }

    Reg_SetVal = 0x00;
    ret = QMA6100P_ReadAndWrite(SW_RESET_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 软复位寄存器复位\r\n");

    Reg_SetVal = 0x84;
    ret = QMA6100P_ReadAndWrite(PM_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 将MCLK设置为51.2KHz, 设置为Avtive Mode模式\r\n");

    Reg_SetVal = 0x20;
    ret = QMA6100P_ReadAndWrite(TST0_ANA_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x20到TST0_ANA REGISTER(0x4A)寄存器\r\n");

    Reg_SetVal = 0x01;
    ret = QMA6100P_ReadAndWrite(AFE_ANA_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x01到AFE_ANA REGISTER(0x56)寄存器\r\n");

    Reg_SetVal = 0x80;
    ret = QMA6100P_ReadAndWrite(TST1_ANA_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x80到TST1_ANA REGISTER (0x5F)寄存器, 将TMODE设置为Take control FSM\r\n");

    vTaskDelay(10/portTICK_PERIOD_MS);

    Reg_SetVal = 0x00;
    ret = QMA6100P_ReadAndWrite(TST1_ANA_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x00到TST1_ANA REGISTER (0x5F)寄存器, 将TMODE设置为Normal mode\r\n");
    
    return ret = ESP_OK;
}

void QMA6100P_Init(void)
{
    if(GetI2C1InitFlagVal() == 0)
    {
        ESP_LOGI(TAG, "QMA6100P I2C总线初始化");
        I2C_Init();
    }
    else
        ESP_LOGI(TAG, "QMA6100P I2C总线已初始化");
}

static esp_err_t Step_Init_Config(void)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t Reg_SetVal = 0;

    //配置STEP INT LATCH
    ret = QMA6100P_ReadAndWrite(INT_CFG_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal |= 0x03;
    ret = QMA6100P_ReadAndWrite(INT_CFG_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x03 到INT_CFG_REGISTER (0x21)寄存器, 将Step中断latch使能\r\n");

    //使能STEP
    ret = QMA6100P_ReadAndWrite(STEP_CONF0_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal |= 0x80;
    ret = QMA6100P_ReadAndWrite(STEP_CONF0_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x80到STEP_CONF0_REGISTER (0x12)寄存器, 将Step功能进行使能\r\n");

    ret = QMA6100P_ReadAndWrite(STEP_CONF1_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal |= 0x80;
    ret = QMA6100P_ReadAndWrite(STEP_CONF1_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal &= 0x7F;
    ret = QMA6100P_ReadAndWrite(STEP_CONF1_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x80到STEP_CONF1_REGISTER (0x13)寄存器, 清除步数寄存器中的值\r\n");

    Reg_SetVal = 1;
    ret = QMA6100P_ReadAndWrite(STEP_CONF2_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P INT1_MAP1_REGISTER (0x14)寄存器, 设置一步最短时间为50*1/100 = 0.5s\r\n");
    Reg_SetVal = 1;
    ret = QMA6100P_ReadAndWrite(STEP_CONF3_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P INT1_MAP1_REGISTER (0x15)寄存器, 设置一步最短时间为12*8*1/100 = 0.96s\r\n");

    //使能STEP INT
    ret = QMA6100P_ReadAndWrite(INT_EN0_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal |= 0x08;
    ret = QMA6100P_ReadAndWrite(INT_EN0_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P 写0x48到INT_EN0_REGISTER (0x16)寄存器, 将Step和Sig_Step中断功能进行使能\r\n");

    //STEP INT MAP
    ret = QMA6100P_ReadAndWrite(INT1_MAP0_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal |= 0x08;
    ret = QMA6100P_ReadAndWrite(INT1_MAP0_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P INT1_MAP0_REGISTER (0x19)寄存器, 将Step和Sig_Step中断进行引脚映射, 映射到PIN_INT1\r\n");

    //设置STEP INT ACTIVE LEVEL = 0
    ret = QMA6100P_ReadAndWrite(INTPIN_CONF_REGISTER, NULL, 0, &Reg_SetVal, 1, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    Reg_SetVal &= 0xFE;
    ret = QMA6100P_ReadAndWrite(INTPIN_CONF_REGISTER, &Reg_SetVal, 1, NULL, 0, FLAG_QMA6100A_WRITE);
    if(ret != ESP_OK)   return ret = ESP_FAIL;
    ESP_LOGI(TAG, "QMA6100P INTPIN_CONF_REGISTER (0x20)寄存器, 将PIN_INT1的中断有效位设置为0\r\n");

    return ret;
}

uint16_t STEP_COUNT_READ(void)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t Reg_GetVal[2] = {0};

    ret = QMA6100P_ReadAndWrite(STEP_CNT_L_REGISTER, NULL, 0, Reg_GetVal, 2, FLAG_QMA6100A_READ);
    if(ret != ESP_OK)   return ret = ESP_FAIL;

    return ((Reg_GetVal[1] << 8) | Reg_GetVal[0]);
}