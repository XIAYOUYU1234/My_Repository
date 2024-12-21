#ifndef BSP_XL9555_H
#define BSP_XL9555_H

#include "driver/i2c.h"

/* CHIP PHYSICAL DEVICE ADDRESS */
#define CHIP_Address        0x40
#define ACK_CHECK_EN        0x1

/* REGISTER Read/Write Cmd */
typedef enum{
    XL9555_Write_Cmd = 0,
    XL9555_Read_Cmd = 1
}XL9555_WR_Cmd;

/* REGISTER Define */
typedef enum
{
    Input_Reg_0 = 0x00,
    Input_Reg_1 = 0x01,
    Output_Reg_0 = 0x02,
    Output_Reg_1 = 0x03,
    Pol_Inversion_Reg_0 = 0x04,
    Pol_Inversion_Reg_1 = 0x05,
    Config_Reg_0 = 0x06,
    Config_Reg_1 = 0x07
}XL9555_REG;


void XL9555_Write_IO(XL9555_REG REG_NUM, uint8_t CmdData);
uint8_t XL9555_Read_IO(XL9555_REG REG_NUM);
void Xl9555_init(void);

#endif //BSP_XL9555_H