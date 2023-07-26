#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "demod.h"
#include "utils.h"

// 获取日期
void GetDate(uint8_t second,    // 由 P1 推算秒数（data 下标 0）
             uint8_t hour_1,    // 小时第一位（data 下标 2）
             uint8_t hour_2,    // 小时第二位（data 下标 3）
             uint8_t minute_1,  // 分钟第一位（data 下标 4）
             uint8_t minute_2,  // 分钟第二位（data 下标 5）
             uint8_t minute_3,  // 分钟第三位（data 下标 6）
             uint8_t week_1,    // 星期数第一位（data 下标 7）
             uint8_t week_2,    // 星期数第二位（data 下标 8）
             uint8_t forenoon,  // 午前或午后（data 下标 9）
             uint8_t day_1,     // 日期第一位（data 下标 10）
             uint8_t day_2,     // 日期第二位（data 下标 11）
             uint8_t day_3,     // 日期第三位（data 下标 12）
             uint8_t month_1,   // 月份第二位（data 下标 13）
             uint8_t month_2,   // 月份第三位（data 下标 14）
             uint8_t year_1,    // 年份第一位（data 下标 15）
             uint8_t year_2,    // 年份第二位（data 下标 16）
             uint8_t year_3     // 年份第三位（data 下标 17）
) {
    // 计算秒数
    switch (second) {
        case 0:
            DateInfo.carry = false;
            DateInfo.second = 21;
            break;
        case 1:
            DateInfo.carry = false;
            DateInfo.second = 41;
            break;
        case 2:
            // 来到下一分钟，启用进位
            DateInfo.carry = true;
            DateInfo.second = 1;
            break;
    }
    // 计算小时
    DateInfo.hour = Quaternary2Decimal(hour_1 * 10 + hour_2);
    // 计算分钟
    DateInfo.minute =
        Quaternary2Decimal(minute_1 * 100 + minute_2 * 10 + minute_3);
    // 计算星期
    switch (Quaternary2Decimal(week_1 * 10 + week_2)) {
        case 1:
            strcpy(DateInfo.week, "Mon");
            break;
        case 2:
            strcpy(DateInfo.week, "Tue");
            break;
        case 3:
            strcpy(DateInfo.week, "Wed");
            break;
        case 4:
            strcpy(DateInfo.week, "Thu");
            break;
        case 5:
            strcpy(DateInfo.week, "Fri");
            break;
        case 6:
            strcpy(DateInfo.week, "Sat");
            break;
        // 待考证：BPC 实际发播数据中，7 或 0 都对应周日？
        default:
            strcpy(DateInfo.week, "Sun");
            break;
    }
    // 判断上午下午
    if (forenoon == 0 || forenoon == 1) {
        DateInfo.forenoon = true;
    } else {
        // 下午为 false
        DateInfo.forenoon = false;
        // 小时加 12
        DateInfo.hour += 12;
    }
    // 计算日期
    DateInfo.day = Quaternary2Decimal(day_1 * 100 + day_2 * 10 + day_3);
    // 计算月份
    DateInfo.month = Quaternary2Decimal(month_1 * 10 + month_2);
    // 计算年份
    DateInfo.year = Quaternary2Decimal(year_1 * 100 + year_2 * 10 + year_3);
    // 最后一步，若是有进位，则视情况为分钟、小时加一
    if (DateInfo.carry) {
        // 分钟加一
        ++DateInfo.minute;
        // 小时加一
        if (DateInfo.minute == 60) {
            ++DateInfo.hour;
            DateInfo.minute = 0;
        }
    }
}

// BPC 解码器
void BPCDecoder() {
    // 若解码器状态标记为 false 则退出
    if (!DecoderStatus.status) {
        Serial.print("当前没有解码任务，不进行解码");
        Serial.print("\r\n");
        return;
    } else {
        // 计算显示进度
        Serial.print(DecoderStatus.counter * 100 / 19);
        Serial.print("% 正在解码中，请稍候");
        Serial.print("\r\n");
    }
    // cycle 小于 900 或 pulse 小于 80 或 pulse 大于 520，则帧同步失锁
    if (BPCStatus.cycle < 900 || BPCStatus.pulse < 80 ||
        BPCStatus.pulse > 520) {
        // 中断本次任务
        DecoderStatus.counter = 0;
        DecoderStatus.status = false;
        Serial.print("帧同步失锁，等待下一帧数据");
        Serial.print("\r\n");
        return;
    } else if  // 若 pulse 在 80 至 100 之间则做一次补偿
        (BPCStatus.pulse > 80 && BPCStatus.pulse < 100) {
        BPCStatus.pulse += 20;
    } else if  // 若 pulse 在 500 至 520 之间则做一次补偿
        (BPCStatus.pulse > 500 && BPCStatus.pulse < 520) {
        BPCStatus.pulse -= 20;
    }
    // 将数据推送到数组
    if (DecoderStatus.counter < 19) {
        DecoderStatus.data[DecoderStatus.counter++] = BPCStatus.pulse / 100 - 1;
    } else {
        // 解析解码后的数据
        GetDate(
            DecoderStatus.data[0], DecoderStatus.data[2], DecoderStatus.data[3],
            DecoderStatus.data[4], DecoderStatus.data[5], DecoderStatus.data[6],
            DecoderStatus.data[7], DecoderStatus.data[8], DecoderStatus.data[9],
            DecoderStatus.data[10], DecoderStatus.data[11],
            DecoderStatus.data[12], DecoderStatus.data[13],
            DecoderStatus.data[14], DecoderStatus.data[15],
            DecoderStatus.data[16], DecoderStatus.data[17]);
        // 打印数据
        Serial.print("20");
        Serial.print(DateInfo.year);
        Serial.print("-");
        Serial.printf("%02d", DateInfo.month);
        Serial.print("-");
        Serial.printf("%02d", DateInfo.day);
        Serial.print(" ");
        Serial.print(DateInfo.week);
        Serial.print(" ");
        Serial.printf("%02d", DateInfo.hour);
        Serial.print(":");
        Serial.printf("%02d", DateInfo.minute);
        Serial.print(":");
        Serial.printf("%02d", DateInfo.second);
        Serial.print(" HKT\r\n");
        // 将本次任务标记为已完成
        DecoderStatus.done = true;
    }
    // 返回提示
    if (DecoderStatus.done) {
        DecoderStatus.counter = 0;
        DecoderStatus.done = !DecoderStatus.done;
        DecoderStatus.status = !DecoderStatus.status;
        Serial.print("本次解码完成，下次解码在 20s 后开始");
        Serial.print("\r\n");
    }
}

void setup() {
    // 启动串口
    Serial.begin(9600);
    // 使能管脚设置为输出模式
    pinMode(EN_PIN, OUTPUT);
    // 使能管脚设置为低电平
    digitalWrite(EN_PIN, LOW);
}

void loop() {
    // 读取 BPC 数据
    if (digitalRead(DATA_PIN) == HIGH) {
        // 若 BPC 状态为 false 则记录下时间
        if (!BPCStatus.status) {
            // BPC 状态取反
            BPCStatus.status = !BPCStatus.status;
            BPCStatus.time = millis();
        }
    } else {
        // 若 BPC 状态为 true 则计算周期和脉冲
        if (BPCStatus.status) {
            // 计算方波周期时间
            BPCStatus.cycle = BPCStatus.time - BPCStatus.previous;
            // 计算脉冲时间
            BPCStatus.pulse = millis() - BPCStatus.time;
            BPCStatus.previous = BPCStatus.time;
            BPCStatus.status = !BPCStatus.status;
            // 方波总时间大于 1800 且小于 2100 则为定位符
            if (BPCStatus.cycle >= 1800 && BPCStatus.cycle <= 2100 &&
                !DecoderStatus.status) {
                Serial.print("成功获取到定位符");
                Serial.print("\r\n");
                DecoderStatus.status = true;
            }
            // 执行解码函数
            BPCDecoder();
        }
    }
}
