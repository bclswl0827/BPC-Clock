#include "utils.h"

// 自行实现 pow()
uint8_t _pow(uint8_t x, uint8_t y) {
    uint8_t p = 1;
    while (y) {
        p *= x;
        --y;
    }
    return p;
}

// 四进制转十进制
uint8_t Quaternary2Decimal(uint16_t n) {
    uint8_t dec = 0, i = 0, rem;
    while (n != 0) {
        rem = n % 10;
        n /= 10;
        dec += rem * _pow(4, i);
        ++i;
    }
    return dec;
}