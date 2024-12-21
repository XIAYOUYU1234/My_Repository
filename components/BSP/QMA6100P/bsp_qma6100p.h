#ifndef BSP_QMA61000P_H
#define BSP_QMA61000P_H

#include "bsp_i2c.h"

#define QMA6100P_I2C        I2C_NUM_0

#define QMA6100P_WRITE      0x24
#define QMA6100P_READ       0x25

#define M_PI                        3.141592653589793f
#define M_G                         9.80665f
#define RAD_TO_DEG                  (180.0f / M_PI)                         /* 0.017453292519943295 */

typedef enum
{
    FLAG_QMA6100A_WRITE = 0x01,
    FLAG_QMA6100A_READ = 0x02,
    FLAG_QMA6100A_STOP = 0x04
}QMA6100P_ACTFLAG;

typedef enum
{
    CHIP_ID_REGISTER = 0x00,
    X_OUT_L_REGISTER = 0x01,
    X_OUT_H_REGISTER = 0x02,
    Y_OUT_L_REGISTER = 0x03,
    Y_OUT_H_REGISTER = 0x04,
    Z_OUT_L_REGISTER = 0x05,
    Z_OUT_H_REGISTER = 0x06,
    STEP_CNT_L_REGISTER = 0x07,             //低八
    STEP_CNT_H_REGISTER = 0x08,             //高八
    STEP_CNT_RESV_REGISTER = 0x0D,          //保留reserve
    INT_STATUS_0_REGISTER = 0x09,   
    INT_STATUS_1_REGISTER = 0x0A,   
    INT_STATUS_2_REGISTER = 0x0B,   
    INT_STATUS_3_REGISTER = 0x0C,   
    FIFO_STATUS_REGISTER = 0x0E,    
    RANGE_REGISTER = 0x0F,                  //精度控制寄存器
    OUTPUT_DATA_RATE_REGISTER = 0x10,       //数据输出速率寄存器
    PM_REGISTER = 0x11,                     //模式控制以及时钟选择寄存器
    STEP_CONF0_REGISTER = 0x12,
    STEP_CONF1_REGISTER = 0x13,
    STEP_CONF2_REGISTER = 0x14,
    STEP_CONF3_REGISTER = 0x15,
    INT_EN0_REGISTER = 0x16,
    INT_EN1_REGISTER = 0x17,
    INT_EN2_REGISTER = 0x18,
    INT1_MAP0_REGISTER = 0x19,
    INT1_MAP1_REGISTER = 0x1A,
    INT2_MAP0_REGISTER = 0x1B,
    INT2_MAP1_REGISTER = 0x1C,
    STEP_CFG0_REGISTER = 0x1D,
    STEP_CFG1_REGISTER = 0x1E,
    STEP_CFG2_REGISTER = 0x1F,
    INTPIN_CONF_REGISTER = 0x20,
    INT_CFG_REGISTER = 0x21,
    RAISE_CFG0_REGISTER = 0x22,
    RAISE_CFG1_REGISTER = 0x23,
    RAISE_CFG2_REGISTER = 0x24,
    RAISE_CFG3_REGISTER = 0x25,
    RAISE_CFG4_REGISTER = 0x26,
    OS_CUST_X_REGISTER = 0x27,              //X轴 校准补偿寄存器
    OS_CUST_Y_REGISTER = 0x28,
    OS_CUST_Z_REGISTER = 0x29,
    TAP_CFG0_REGISTER = 0x2A,
    TAP_CFG1_REGISTER = 0x2B,
    MOTION_CFG0_REGISTER = 0x2C,
    MOTION_CFG1_REGISTER = 0x2D,
    MOTION_CFG2_REGISTER = 0x2E,
    MOTION_CFG3_REGISTER = 0x2F,
    RST_MOTION_CFG_REGISTER = 0x30,
    FIFO_WM_LVL_REGISTER = 0x31,
    SELFTEST_REGISTER = 0x32,
    NVM_REGISTER = 0x33,
    Y_TH_YZ_TH_SEL_REGISTER = 0x34,
    RAISE_WAKE_PERIOD_REGISTER = 0x35,
    SW_RESET_REGISTER = 0x36,
    FIFO_CFG0_REGISTER = 0x3E,
    FIFO_DATA_REGISTER = 0x3F,
    TST0_ANA_REGISTER = 0x4A,
    AFE_ANA_REGISTER = 0x56,
    TST1_ANA_REGISTER = 0x5F
}QMA6100P_Reg_Enum;

void QMA6100P_Init(void);
esp_err_t QMA6100P_Config(void);
void QMA6100P_RawDataGet(void);
uint16_t STEP_COUNT_READ(void);

#endif  //BSP_QMA61000P_H