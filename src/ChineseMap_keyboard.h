#ifndef CHINESEMAP_KEYBOAED_H
#define CHINESEMAP_KEYBOAED_H

#include "ChineseMap_keyboard.h"
#include <Arduino.h>

// 定义结构体存储拼音对应关系
typedef struct
{
    const char *pinyin;
    const unsigned char *hanzi;
} PinyinMap;
// 定义拼音映射表
typedef struct
{
    const char *this_c;
    const char *up;
    const char *down;
    const char *left;
    const char *right;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    const char *this_now;
} PINYIN_MEUN;

class PINYIN_MEUN_ : PINYIN_MEUN
{
public:
    float x_now = 48; // 指向动画
    float y_now = -12;
    float w_now = 32;
    float h_now = 12;

    int16_t input_box = 0;       // 输入框y轴
    int16_t key_boa_ui = 0;      // 键盘UI x轴
    int16_t cn_box_ui = 0;       // 中文选项x轴
    float input_box_now = -12; // 动画
    float key_boa_ui_now = -128;
    float cn_box_ui_now = 24;

    bool capslk = 0; // 大小写

    const char *text_now; // 当前指向字符
};

extern const PinyinMap *pinyinMapArray[];
extern const char *PINYIN_MEUN_ARR[];
extern const char *PINYIN_MEUN_ARR_CN[];
extern const char *PINYIN_MEUN_arr[];
extern const char *PINYIN_MEUN_arr_num[];
extern PINYIN_MEUN pinyin_MEUN[];
char *PYSearch(const char *msg, int *num);


#endif