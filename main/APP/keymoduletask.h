#ifndef KEYMODULETASK_H
#define KEYMODULETASK_H

/* 串口调试信息打印使能宏 */
#define DEBUG_KEY_Task 1

enum Timer_AutoReload
{
    Timer_AutoReloadDisable = 0,
    Timer_AutoReloadEnable = 1
};

typedef enum
{
    KEY_POWER_LONG = 1,
    KEY_POWER_SHORTDOWN = 2,
    KEY_SOS_LONGDOWN = 3,
    KEY_SOS_LONG_LONGDOWN = 4,
    KEY_PTT_SHORTDOWN = 5,
    KEY_PTT_RELEASE = 6,
    KEY_PTT_TIMEOUT = 7,
    KEY_OK_SHORTDOWN = 8,
    KEY_OK_LONGDOWN = 9,
    KEY_UP_SHORTDOWN = 10,
    KEY_UP_LONGDOWN = 11,
    KEY_DOWN_SHORTDOWN = 12,
    KEY_DOWN_LONGDOWN = 13,
    KEY_OK_ADD_KEY_UP = 100,
    KEY_OK_ADD_KEY_DOWN = 101,
    KEY_UP_ADD_KEY_DOWN = 102
}Enum_RetKeyScan;

void KEYScan_Start(void);
void KEYScan_Action_Start(void);

#endif //KEYMODULETASK_H
