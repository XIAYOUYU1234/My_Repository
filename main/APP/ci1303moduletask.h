/**
 ****************************************************************************************************
 * @file        ci1303moduletask.h
 * @author      fzk
 * @version     V1.0
 * @date        2024-08-07
 * @brief       启英泰伦CI1303模块
 ****************************************************************************************************
**/

#ifndef __CI1303MODULETASK_H_
#define __CI1303MODULETASK_H_

/* 串口调试信息打印使能宏 */
#define DEBUG_CI1303 1

#define CI1303_CmdNum 20
#define CI1303_CmdLen 8

typedef enum{
    VoiceIpt_None = 0,          
    VoiceIpt_Wakeup,                      //小北小北                  
    VoiceIpt_AddVoiceLevel,               //声音调大  
    VoiceIpt_DecVoiceLevel,               //声音调小  
    VoiceIpt_AddVoiceLevelToMax,          //声音调到最大          
    VoiceIpt_DecVoiceLevelToMin,          //声音调到最小          
    VoiceIpt_Safe,                        //汇报平安
    VoiceIpt_FindTerminal,                //你在哪  
    VoiceIpt_FindBaseStation,             //找基站      
    VoiceIpt_PrepareToBaseStation,        //返回基站          
    VoiceIpt_ArriveBaseStation,           //已找到基站      
    VoiceIpt_SendVoice,                   //发送语音
    VoiceIpt_PlayVoice,                   //播放
    VoiceIpt_ChangeGroup,                 //切换群组  
    VoiceIpt_SOS,                         //开启报警
    VoiceIpt_RelieveAlarm,                //关闭  
    VoiceIpt_PowerCheck,                  //报告电量  
    VoiceIpt_TurnOnLED,                   //打开手电筒
    VoiceIpt_TurnOffLED,                  //关掉手电筒  
    VoiceIpt_TurnOffSys,                  //请关机  
    VoiceIpt_UpDataSys                    //系统更新
}Enum_VoiceCmd;

void CI1303_USART_Rec_Start(void);
void CI1303_USART_Action_Start(void);

#endif
