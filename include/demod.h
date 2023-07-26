#ifndef __DEMOD_H__
#define __DEMOD_H__

#include <stdint.h>

// 用于记录讯号携带的数据
struct BPCStatus {
    bool status;        // 是否有数据
    uint16_t time;      // 记录起始时间用于后续计算
    uint16_t previous;  // 记录上一次起始时间
    uint16_t cycle;     // 记录方波周期时间
    uint16_t pulse;     // 记录脉冲时间
} BPCStatus;

// 用于记录解码器状态
struct DecoderStatus {
    bool status;       // 解码器运行状态
    bool done;         // 解码是否完成
    uint8_t counter;   // 数据缓存位置计数器
    uint8_t data[19];  // 除去定位符后的数据缓存
    // 示例数据
    // 组数  0    1    2    4     7     9     10    13   15    18
    // 数据  0    0    23   321   13    0     032   20   112   1
    // 说明  P1   P2   时   分   星期   P3    日    月   年    P4
} DecoderStatus;

// 用于记录解码出来的时间
struct DateInfo {
    bool carry;      // 根据秒数判断是否需进位
    uint8_t second;  // 秒
    uint8_t hour;    // 时
    uint8_t minute;  // 分
    char week[4];    // 星期
    bool forenoon;   // 午前
    uint8_t day;     // 日
    uint8_t month;   // 月
    uint8_t year;    // 年
} DateInfo;

#endif