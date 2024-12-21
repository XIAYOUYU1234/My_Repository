#include "bsp_XL9555.h"
#include "bsp_i2c.h"

#include "esp_log.h"
#define TAG "Xl9555"

i2c_cmd_handle_t I2C0_Cmd_Handle;

void XL9555_WriteOrRead(XL9555_WR_Cmd Cmd)
{
    uint8_t Device_Addr = 0;

    Device_Addr = CHIP_Address | Cmd;
    // printf("Device_Addr = %d\r\n", Device_Addr);
    
    // if(Cmd == XL9555_Write_Cmd)
    //     printf("选择写XL9555\r\n");
    // else
    //     printf("选择读XL9555\r\n");

    i2c_master_write_byte(I2C0_Cmd_Handle, Device_Addr, ACK_CHECK_EN);
}

void XL9555_RegisterChoose(XL9555_REG REG_NUM)
{
    // switch(REG_NUM)
    // {
    //     case 0: printf("选择操作XL9555的 Input_Reg_0 寄存器\r\n"); break;
    //     case 1: printf("选择操作XL9555的 Input_Reg_1 寄存器\r\n"); break;
    //     case 2: printf("选择操作XL9555的 Output_Reg_0 寄存器\r\n"); break;
    //     case 3: printf("选择操作XL9555的 Output_Reg_1 寄存器\r\n"); break;
    //     case 4: printf("选择操作XL9555的 Pol_Inversion_Reg_0 寄存器\r\n"); break;
    //     case 5: printf("选择操作XL9555的 Pol_Inversion_Reg_1 寄存器\r\n"); break;
    //     case 6: printf("选择操作XL9555的 Config_Reg_0 寄存器\r\n"); break;
    //     case 7: printf("选择操作XL9555的 Config_Reg_1 寄存器\r\n"); break;
    //     default: break;
    // }

    i2c_master_write_byte(I2C0_Cmd_Handle, (0x00 | REG_NUM), ACK_CHECK_EN);
}

void XL9555_RegisterCmdWrite(uint8_t IO_WriteCmd)
{
//    printf("寄存器写入的字节指令为%d\r\n", IO_WriteCmd);

    i2c_master_write_byte(I2C0_Cmd_Handle, IO_WriteCmd, ACK_CHECK_EN);
}

uint8_t XL9555_RegisterCmdRead(void)
{
    uint8_t IO_Read_Data = 0;

    i2c_master_read_byte(I2C0_Cmd_Handle, &IO_Read_Data, I2C_MASTER_LAST_NACK);

//    printf("XL9555_IORead, IO_Read_Data = %08X\r\n", IO_Read_Data);

    return IO_Read_Data;
}

void XL9555_Write_IO(XL9555_REG REG_NUM, uint8_t CmdData)
{
    esp_err_t ret = ESP_FAIL;

    I2C0_Cmd_Handle = i2c_cmd_link_create();

    i2c_master_start(I2C0_Cmd_Handle);
    
    XL9555_WriteOrRead(XL9555_Write_Cmd);

    XL9555_RegisterChoose(REG_NUM);
    
    XL9555_RegisterCmdWrite(CmdData);

    i2c_master_stop(I2C0_Cmd_Handle);

    ret = i2c_master_cmd_begin(I2C_NUM_0 ,I2C0_Cmd_Handle, 400/portTICK_PERIOD_MS);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_cmd_begin 错误， 错误码：%d\r\n", ret);
    }

    i2c_cmd_link_delete(I2C0_Cmd_Handle);
}

uint8_t XL9555_Read_IO(XL9555_REG REG_NUM)
{
    esp_err_t ret = ESP_FAIL;
    uint8_t ReadData = 0;

    I2C0_Cmd_Handle = i2c_cmd_link_create();

    //启动
    i2c_master_start(I2C0_Cmd_Handle);
    
    //设备地址和写操作
    XL9555_WriteOrRead(XL9555_Write_Cmd);

    //寄存器选择
    XL9555_RegisterChoose(REG_NUM);

    //重启动
    i2c_master_start(I2C0_Cmd_Handle);

    //设备地址和读操作
    XL9555_WriteOrRead(XL9555_Read_Cmd);    
    
    //读取寄存器内容
 //   ReadData = XL9555_RegisterCmdRead();
    i2c_master_read_byte(I2C0_Cmd_Handle, &ReadData, I2C_MASTER_LAST_NACK);

    //停止
    i2c_master_stop(I2C0_Cmd_Handle);

    ret = i2c_master_cmd_begin(I2C_NUM_0 ,I2C0_Cmd_Handle, 400/portTICK_PERIOD_MS);
    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "i2c_master_cmd_begin 错误， 错误码：%d\r\n", ret);
    }

    i2c_cmd_link_delete(I2C0_Cmd_Handle);

    return ReadData;
}

void Xl9555_init(void)
{
    if(GetI2C1InitFlagVal() == 0)
    {
        ESP_LOGI(TAG, "QMA6100P I2C总线初始化");
        I2C_Init();
    }
    else
        ESP_LOGI(TAG, "XL9555 I2C总线已初始化");

    XL9555_Write_IO(Config_Reg_0, 0xFF);
    XL9555_Write_IO(Config_Reg_1, 0xFF);
}