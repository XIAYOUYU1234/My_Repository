#include "bsp_ci1303.h"
#include "bsp_usart.h"
#include "string.h"

// static const uint8_t recv_data[CI1303_CmdNum][CI1303_CmdLen] = {
//     ///tag-deal-uart-msg-by-play-id-after-recv-start
//     //play_msg_deal_by_cmd_id
//     {0xA5,0xFA,0x00,0x81,0x01,0x00,0x21,0xFB},  //小北小北
//     {0xA5,0xFA,0x00,0x81,0x02,0x00,0x22,0xFB},  //声音调大
//     {0xA5,0xFA,0x00,0x81,0x03,0x00,0x23,0xFB},  //声音调小
//     {0xA5,0xFA,0x00,0x81,0x04,0x00,0x24,0xFB},  //声音调到最大
//     {0xA5,0xFA,0x00,0x81,0x05,0x00,0x25,0xFB},  //声音调到最小
//     {0xA5,0xFA,0x00,0x81,0x06,0x00,0x26,0xFB},  //汇报平安
//     {0xA5,0xFA,0x00,0x81,0x07,0x00,0x27,0xFB},  //你在哪
//     {0xA5,0xFA,0x00,0x81,0x08,0x00,0x28,0xFB},  //找基站
//     {0xA5,0xFA,0x00,0x81,0x09,0x00,0x29,0xFB},  //返回基站
//     {0xA5,0xFA,0x00,0x81,0x0A,0x00,0x2A,0xFB},  //已找到基站
//     {0xA5,0xFA,0x00,0x81,0x0B,0x00,0x2B,0xFB},  //发送语音
//     {0xA5,0xFA,0x00,0x81,0x0C,0x00,0x2C,0xFB},  //播放
//     {0xA5,0xFA,0x00,0x81,0x0D,0x00,0x2D,0xFB},  //切换群组
//     {0xA5,0xFA,0x00,0x81,0x0E,0x00,0x2E,0xFB},  //开启报警
//     {0xA5,0xFA,0x00,0x81,0x0F,0x00,0x2F,0xFB},  //关闭
//     {0xA5,0xFA,0x00,0x81,0x10,0x00,0x30,0xFB},  //报告电量
//     {0xA5,0xFA,0x00,0x81,0x11,0x00,0x31,0xFB},  //打开手电筒
//     {0xA5,0xFA,0x00,0x81,0x12,0x00,0x32,0xFB},  //关掉手电筒
//     {0xA5,0xFA,0x00,0x81,0x13,0x00,0x33,0xFB},  //请关机
//     {0xA5,0xFA,0x00,0x81,0x14,0x00,0x34,0xFB},  //系统更新
// };