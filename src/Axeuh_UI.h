#ifndef AXEUH_UI_H
#define AXEUH_UI_H

#include <Arduino.h>
#include <U8g2lib.h>

#define CHINESE_KEYBOARD // 如果要使用拼音键盘，需要声明CHINESE_KEYBOARD

#ifdef CHINESE_KEYBOARD
#include "ChineseMap_keyboard.h"
#endif

class Axeuh_UI_Cube;
class Axeuh_UI_TextMenu;
class Axeuh_UI_Panel;
class Axeuh_UI_StatusBar;
class Axeuh_UI_slider;
class Axeuh_UI_Ebook;
class Axeuh_UI;

struct CharCache
{
    char utf8Char[4]; // UTF8字符最大4字节
    uint8_t width;
};

class CharLenMap // 此类为了实时记录每一个字符的像素长度，从而提高刷新率，因为在ui绘制中，计算字符像素长度是个废时间的操作
{
public:
    CharCache *Charlen;
    uint16_t maxSize; // 目前最大容量
    uint16_t size;    // 目前使用的容量
    U8G2 *D;          // U8G2实例

    CharLenMap(uint16_t initialCapacity = 10)
        : maxSize(initialCapacity), size(0)
    {
        Charlen = new CharCache[maxSize];
        memset(Charlen, 0, sizeof(CharCache) * maxSize);
    }

    void set_u8g2(U8G2 *D_) { D = D_; }

    void reset()
    {
        maxSize = 10;
        size = 0;
        delete[] Charlen;
        Charlen = new CharCache[maxSize];
        memset(Charlen, 0, sizeof(CharCache) * maxSize);
    }

    // 动态扩容函数
    void expand()
    {
        uint16_t newCapacity = maxSize + 10; // 容量增加
        CharCache *newArray = new CharCache[newCapacity];
        memset(newArray, 0, sizeof(CharCache) * newCapacity); // 初始化新数组

        // 复制旧数据
        memcpy(newArray, Charlen, sizeof(CharCache) * maxSize);

        // 释放旧内存并更新指针和容量
        delete[] Charlen;
        Charlen = newArray;
        maxSize = newCapacity;
    }

    bool fastCompare4(const char *a, const char *b)
    {
        uint32_t va, vb;
        memcpy(&va, a, 4);
        memcpy(&vb, b, 4);
        return (va == vb);
    }

    bool isChineseChar(const char str)
    {
        unsigned char c = str;
        if (c >= 0x80)
        { // 大于等于0x80的是UTF-8编码的字符
            // UTF-8 编码规则中，中文字符的第一个字节的高位为 1110，因此需要检查第一个字节的高位是否满足这个条件
            if ((c & 0xE0) == 0xE0)
            {
                // 如果满足条件，则该字符很可能是中文字符
                return true;
            }
        }
        // 如果不满足上述条件，则认为是英文字符或者符号
        return false;
    }

    uint8_t Get(String c_s)
    {
        char c[4] = {0, 0, 0, 0};
        strncpy(c, c_s.c_str(), sizeof(c) - 1);

        // if (isChineseChar(c[0]))//如果是中文字符直接输出长度，减少计算（没什么效果）
        //     return 12;

        for (int i = 0; i < size; i++)
        {
            if (fastCompare4(Charlen[i].utf8Char, c))
            {
                return Charlen[i].width;
            }
        }

        // 如果空间不足，自动扩容
        if (size >= maxSize)
        {
            expand();
        }

        if (D != nullptr)
        {
            // 添加新字符到数组末尾
            memcpy(Charlen[size].utf8Char, c, 4);
            Charlen[size].width = D->getUTF8Width(c) + 1;
            while (Charlen[size].width > 128)
            {
                Charlen[size].width = D->getUTF8Width(c) + 1;
            }
            // Serial.println((String)c + "  " + (String)Charlen[size].width + "  " + (String)size);
            size++; // 更新已用数量

            // Serial.println("已使用:" + (String)size);
            return Charlen[size - 1].width;
        }
        else
            return 0;
    }
};

enum IN_PUT_Mode
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    STOP
};

enum alignMode
{
    LEFT_CORNER, // 左上角
    LEFT_CENTER, // 左中间
    ALIGN_CENTER // 居中（单行）
};

enum OptionMode // 文本选项模式
{
    TEXT,        // 文本模式
    TEXT_MORE,   // 多行文本模式（可自动换行）
    PICTURE,     // 显示图片模式
    PICTURE_TEXT // 图文模式（图片在前，文字随后，要想实现不同的效果需要设置坐标）
};

enum OptiongifMode1
{
    AutoPlay,  // 自动播放GIF动画
    ManualPlay // 聚焦时播放
};
enum OptiongifMode2
{
    Hide, // 隐藏（聚焦时GIF会有个渐入动画）
    Show  // 不隐藏
};

enum OptionisSelectable
{
    Focused,    // 可选中
    No_Focusing // 不可选中（在菜单界面，不可选中选项会被自动跳过）
};

enum OptionistriggersAnimation
{
    Trigger,   // 触发菜单动画（菜单动画就是会让菜单变窄的动画）
    No_Trigger // 不触发
};

enum Option_hardware
{
    hardware_3, // 触发菜单动画（菜单动画就是会让菜单变窄的动画）
    hardware_5  // 不触发
};

// 定义选项结构体
struct Menu_slider
{
    String name = "";         // 滑动条标题
    float *num = 0;           // 修改值指针（float）
    int *num_ = 0;            // 修改值指针（int）
    uint16_t *num_uint16 = 0; // 修改值指针（uint16_t）
    int16_t min = 0;          // 最小值
    int16_t max = 0;          // 最大值
};

// GIF图片结构体
struct Menu_gif
{
    const unsigned char **jpg; // 图片数据指针
    uint16_t frameCount;       // 总帧数
    uint16_t x;                // x的目标坐标
    uint16_t y;
    uint16_t w;                // 图片宽度
    uint16_t h;                // 图片高度
    uint8_t frameCount_speed;  // 动画速度
    uint16_t frameCount_start; // 动画开始的帧

    uint16_t frameCount_now; // 当前帧
    float x_now;             // x的实时坐标
    float y_now;
    int16_t x_; // 聚焦隐藏x参数
    int16_t y_; // y参数

    OptiongifMode1 Autoplay; // 是否自动播放

    OptiongifMode2 Play_hide; // 是否隐藏

    unsigned long previousMillis; // 上一帧的时间刻

    Menu_gif(const unsigned char **frames = nullptr,
             uint16_t frameCount_ = 0,
             uint16_t xx = 0,
             uint16_t yy = 0,
             uint16_t width = 0,
             uint16_t height = 0,
             uint8_t speed = 100,
             uint16_t startFrame = 0,
             OptiongifMode1 aPlay = AutoPlay,
             OptiongifMode2 hPlay = Show)
        : jpg(frames),
          frameCount(frameCount_),
          x(xx),
          y(yy),
          w(width),
          h(height),
          frameCount_speed(speed),
          frameCount_start(startFrame),
          frameCount_now(startFrame),
          Autoplay(aPlay),
          Play_hide(hPlay)
    {
        if (hPlay == Hide)
            x_now = -width - x;
    }
};
typedef void (*Panel_draw_callback)(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m);
typedef void (*textMenuCallback)(Axeuh_UI_Panel *p, Axeuh_UI *menu);
typedef void (*MenuCallback_Ebook)(Axeuh_UI_Panel *p, Axeuh_UI *m);
typedef void (*StatusBar_Callback)(U8G2 *p, Axeuh_UI *m);
typedef IN_PUT_Mode (*Axeuh_UI_input_callback)();

class SimpleKalmanFilter // 卡尔曼滤波
{
private:
    float x_est; // 当前状态估计
    float P;     // 误差协方差
    float Q;     // 过程噪声
    float R;     // 测量噪声

public:
    // 初始化滤波器
    SimpleKalmanFilter()
    {
        P = 1.0;
    }
    SimpleKalmanFilter(float process_noise, float measurement_noise, float initial_value)
    {
        Q = process_noise;
        R = measurement_noise;
        x_est = initial_value;
        P = 1.0; // 初始协方差
    }

    // 输入新测量值，返回滤波后结果
    float update(float measurement)
    {
        // 预测阶段
        float x_temp = x_est;
        float P_temp = P + Q;

        // 更新阶段
        float K = P_temp / (P_temp + R); // 卡尔曼增益
        x_est = x_temp + K * (measurement - x_temp);
        P = (1 - K) * P_temp;

        return x_est;
    }
};

class Axeuh_UI // 总UI类
{
private:
    TaskHandle_t displayTaskHandle = nullptr;            // ui进程句柄
    SemaphoreHandle_t xMutex_ = xSemaphoreCreateMutex(); // 互斥锁

public:
    U8G2 *D; // U8G2实例指针

    int16_t height = 64;       // 屏幕高度
    int16_t width = 128;       // 屏幕宽度
    int16_t font_offset_x = 0; // 字体显示偏移值
    int16_t font_offset_y = 10;

    IN_PUT_Mode IN_now = STOP; // 当前输入信号

    Axeuh_UI_Cube *cube = nullptr;           // 立方体实例指针
    Axeuh_UI_Panel *Panel = nullptr;         // 首级面板
    Axeuh_UI_StatusBar *StatusBar = nullptr; // 状态栏实例

    static CharLenMap len_c;              // 初始化字符长度词典
    static SimpleKalmanFilter fps_filter; // 初始化卡尔曼滤波器

    int16_t fps_speed_ = 500; // fps偏移值
    uint16_t fps_max = 200;   // fps上限

    unsigned long lastTime = 0;  // 上一次记录时间
    unsigned int frameCount = 0; // 帧计数器
    float fps = 0;               // 计算的FPS值

    Axeuh_UI_input_callback input_callback; // 输入回调函数

    Option_hardware hardware_;

    Axeuh_UI() {}
    Axeuh_UI(U8G2 *d)
    {
        D = d;
    }

    void begin()
    {
        D->begin();           // 初始化显示
        D->enableUTF8Print(); // 启用UTF8
        D->setFont(u8g2_font_wqy12_t_gb2312);
        D->setFontMode(1);
        len_c.set_u8g2(D);                          // 绑定U8G2显示库实例
        fps_filter = SimpleKalmanFilter(5, 10, 60); // 初始化卡尔曼滤波器
    }
    // 获取当前输入状态
    IN_PUT_Mode get_IN_now() { return IN_now; }
    uint16_t get_fps_max() { return fps_max; }
    float get_fps() { return fps; }

    SemaphoreHandle_t get_xMutex() { return xMutex_; }
    void xSemaphoreTake_xMutex(int t = 100)
    {
        xSemaphoreTake(xMutex_, t);
    }
    void xSemaphoreGive_xMutex()
    {
        xSemaphoreGive(xMutex_);
    }

    // 发生输入状态
    void set_IN_now(IN_PUT_Mode in)
    {
        delay(10);
        xSemaphoreTake(xMutex_, 100);
        IN_now = in;
        xSemaphoreGive(xMutex_);
    }
    // 设置FPS上限
    void set_fps_max(uint16_t f)
    {
        xSemaphoreTake(xMutex_, 100);
        fps_max = f;
        xSemaphoreGive(xMutex_);
    }

    // 修改U8G2实例指针
    void set_u8g2(U8G2 *d)
    {
        D = d;
        D->begin();           // 初始化显示
        D->enableUTF8Print(); // 启用UTF8
        D->setFont(u8g2_font_wqy12_t_gb2312);
        D->setFontMode(1);
    }

    // 配置实例
    void set(Option_hardware o) { hardware_ = o; }
    void set(Axeuh_UI_input_callback callback) { input_callback = callback; }
    void set(Axeuh_UI_Panel *p)
    {
        xSemaphoreTake(xMutex_, 100);
        Panel = p;
        xSemaphoreGive(xMutex_);
    }
    void set(Axeuh_UI_StatusBar *sb) { StatusBar = sb; }
    void set(Axeuh_UI_Cube *c) { cube = c; }
    // 渐进动画函数
    void animation(float *a, float a_trg_, float n, float precision = 0.50f)
    {
        // *a=a_trg_;
        // return;
        float target = a_trg_;
        if (n < 9)
            return;
        if (n < 15)
            n = 15;
        if (*a != target)
        {
            float diff = target - *a;
            if (fabs(diff) < precision)
                *a = target; // 接近目标值时直接对齐
            else
                *a += diff / (n / 10.0f); // 浮点步长计算
        }
    }
    void animation(float *a, int16_t a_trg_, float n)
    {
        // *a=a_trg_;
        // return;
        if (n < 9)
            return;
        if (n < 15)
            n = 15;
        if (*a != a_trg_)
        {
            float diff = a_trg_ - *a;
            if (fabs(diff) < 0.50f)
                *a = a_trg_; // 接近目标值时直接对齐
            else
                *a += diff / (n / 10.0f); // 浮点步长计算
        }
    }
    void animation(int16_t *a, int16_t *a_trg, int16_t n)
    {
        // *a=*a_trg;
        // return;
        if (n < 9)
            return;
        if (n < 15)
            n = 15;
        if (*a != *a_trg)
        {
            int16_t diff = *a_trg - *a;

            // 1. 计算基础步长
            int16_t step = diff / max(n / 10, 1); // 确保分母至少为1

            // 2. 如果步长为0但仍有差值，强制步长为±1
            if (step == 0)
                step = (diff > 0) ? 1 : -1;

            // 3. 应用步长并防止过冲
            *a += step;
            if ((diff > 0 && *a > *a_trg) || (diff < 0 && *a < *a_trg))
                *a = *a_trg;
        }
    }
    // 判断是否为中午字符函数
    bool isChineseChar(const char str)
    {
        unsigned char c = str;
        if (c >= 0x80)
        { // 大于等于0x80的是UTF-8编码的字符
            // UTF-8 编码规则中，中文字符的第一个字节的高位为 1110，因此需要检查第一个字节的高位是否满足这个条件
            if ((c & 0xE0) == 0xE0)
            {
                // 如果满足条件，则该字符很可能是中文字符
                return true;
            }
        }
        // 如果不满足上述条件，则认为是英文字符或者符号
        return false;
    }

    void menu_display();
    static void menu_display(void *xTask1) // 异步进程绘制函数
    {
        Axeuh_UI *ui = static_cast<Axeuh_UI *>(xTask1);
        ui->menu_display();
    }
    void menu_display_xtaskbegin(UBaseType_t Memory_size = 4096, BaseType_t xCoreID = 0)
    {
        // 确保任务只创建一次
        if (displayTaskHandle == nullptr)
        {
            xTaskCreatePinnedToCore(
                menu_display,       // 任务函数
                "menu_display",     // 任务名称
                Memory_size,        // 堆栈大小
                this,               // 传递this指针
                1,                  // 优先级
                &displayTaskHandle, // 任务句柄
                xCoreID             // 核心分配
            );
        }
    }
    void menu_display_xtaskstop()
    {
        if (displayTaskHandle == nullptr)
        {
            vTaskDelete(displayTaskHandle);
            displayTaskHandle = nullptr;
        }
    }

    IN_PUT_Mode handleInput() // 触发输入回调函数
    {
        if (input_callback != nullptr)
        {
            return input_callback();
        }
    }

    ~Axeuh_UI() // 没什么用的析构函数
    {
        if (displayTaskHandle)
        {
            vTaskDelete(displayTaskHandle);
            displayTaskHandle = nullptr;
        }
    }
};

// 定义选项结构体
// 定义选项结构体
struct MenuOption : public Axeuh_UI
{
public:
    // 使用 const char* 存储字符串
    const char *name;                            // 选项名称
    uint8_t height;                              // 选项高度（单位：像素）
    alignMode align;                             // 对齐方式
    OptionMode mode;                             // 选项模式
    OptionisSelectable isSelectable;             // 是否可选
    OptionistriggersAnimation triggersAnimation; // 是否触发动画

    Menu_gif *gif;             // 选项图片实例指针
    textMenuCallback callback; // 选项选中总回调函数

    int8_t x = 0; // 选项文本x偏移
    int8_t y = 0;

    // 添加私有成员来管理动态分配的内存（如果需要）
private:
    char *dynamic_name = nullptr;

public:
    // 实现字符串方法
    unsigned int length() const
    {
        return name ? strlen(name) : 0;
    }

    char charAt(unsigned int index) const
    {
        if (!name || index >= strlen(name))
            return '\0';
        return name[index];
    }

    String substring(unsigned int from, unsigned int to) const
    {
        if (!name || from >= strlen(name) || from > to)
            return "";

        // 确保 to 不超过字符串长度
        if (to > strlen(name))
            to = strlen(name);

        // 计算子字符串长度
        unsigned int len = to - from;

        // 创建临时缓冲区并复制子字符串
        char *temp = (char *)malloc(len + 1);
        if (!temp)
            return "";

        strncpy(temp, name + from, len);
        temp[len] = '\0';

        String result(temp);
        free(temp);
        return result;
    }

    String substring(unsigned int from) const
    {
        return substring(from, strlen(name));
    }

    const char *c_str() const
    {
        return name ? name : "";
    }

    // 简化构造函数
    MenuOption(const char *n = "",
               uint8_t h = 12,
               alignMode a = LEFT_CENTER,
               OptionMode m = TEXT,
               Menu_gif *g = nullptr,
               OptionistriggersAnimation tr = No_Trigger,
               textMenuCallback cb = nullptr,
               OptionisSelectable is = Focused)
        : name(n), height(h), align(a), mode(m),
          gif(g), callback(cb), triggersAnimation(tr), isSelectable(is),
          x(0), y(0)
    {
    }
    // MenuOption(String n = "",
    //            uint8_t h = 12,
    //            alignMode a = LEFT_CENTER,
    //            OptionMode m = TEXT,
    //            Menu_gif *g = nullptr,
    //            OptionistriggersAnimation tr = No_Trigger,
    //            textMenuCallback cb = nullptr,
    //            OptionisSelectable is = Focused)
    //     :  height(h), align(a), mode(m),
    //       gif(g), callback(cb), triggersAnimation(tr), isSelectable(is),
    //       x(0), y(0)
    // {
    //     set_str(n);
    // }

    // 修改 set 方法以接受 const char*
    void set(const char *n = "",
             uint8_t h = 12,
             alignMode a = LEFT_CENTER,
             OptionMode m = TEXT,
             Menu_gif *g = nullptr,
             OptionistriggersAnimation tr = No_Trigger,
             textMenuCallback cb = nullptr,
             OptionisSelectable is = Focused)
    {
        // 释放旧的动态内存
        if (dynamic_name)
        {
            free(dynamic_name);
            dynamic_name = nullptr;
        }

        // 设置新值
        if (n && n[0] != '\0')
        {
            dynamic_name = (char *)malloc(strlen(n) + 1);
            if (dynamic_name)
            {
                strcpy(dynamic_name, n);
                name = dynamic_name;
            }
            else
            {
                name = "";
            }
        }
        else
        {
            name = "";
        }
        name = n;
        height = h;
        align = a;
        mode = m;
        gif = g;
        callback = cb;
        triggersAnimation = tr;
        isSelectable = is;
        // x = 0;
        // y = 0;
    }

    // 添加接受 String 的 set 方法
    void set(String n = "",
             uint8_t h = 12,
             alignMode a = LEFT_CENTER,
             OptionMode m = TEXT,
             Menu_gif *g = nullptr,
             OptionistriggersAnimation tr = No_Trigger,
             textMenuCallback cb = nullptr,
             OptionisSelectable is = Focused)
    {
        // 释放旧的动态内存
        if (dynamic_name)
        {
            free(dynamic_name);
            dynamic_name = nullptr;
        }

        // 从 String 复制内容
        if (n.length() > 0)
        {
            dynamic_name = (char *)malloc(n.length() + 1);
            if (dynamic_name)
            {
                strcpy(dynamic_name, n.c_str());
                name = dynamic_name;
            }
            else
            {
                name = "";
            }
        }
        else
        {
            name = "";
        }

        height = h;
        align = a;
        mode = m;
        gif = g;
        callback = cb;
        triggersAnimation = tr;
        isSelectable = is;
        // x = 0;
        // y = 0;
    }

    // void set_x(int16_t x_)
    // {
    //     x = x_;
    // }

    // void set_y(int16_t y_)
    // {
    //     y = y_;
    // }

    void set_str(String n = "")
    {
        // 释放旧的动态内存
        if (dynamic_name)
        {
            free(dynamic_name);
            dynamic_name = nullptr;
        }

        // 从 String 复制内容
        if (n.length() > 0)
        {
            dynamic_name = (char *)malloc(n.length() + 1);
            if (dynamic_name)
            {
                strcpy(dynamic_name, n.c_str());
                name = dynamic_name;
            }
            else
            {
                name = "";
            }
        }
        else
        {
            name = "";
        }
    }

    // 析构函数 - 释放动态内存
    ~MenuOption()
    {
        if (dynamic_name)
        {
            free(dynamic_name);
            dynamic_name = nullptr;
            name = nullptr;
        }
    }
};

class Axeuh_UI_TextMenu : public Axeuh_UI
{
public:
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex(); // 互斥锁

    IN_PUT_Mode in_put_now = STOP; // 当前菜单输入状态

    MenuOption *menuOptions;             // 选项菜单指针
    const MenuOption *const_menuOptions; // 选项菜单指针
    uint16_t *menu_str_len;

    uint8_t menuOptions_index = 0; // 选项数量

    int16_t pointer_x = 0; // 聚焦选项的小方块目前位置
    int16_t pointer_y = 0;
    int16_t pointer_w = 0;
    int16_t pointer_h = 0;

    uint8_t meun_number_now = 0; // 选项在屏幕的位置

    int16_t *x = nullptr; // 页面目标位置（绑定为面板坐标位置）
    int16_t *y = nullptr;
    int16_t *w = nullptr;
    int16_t *h = nullptr;

    int16_t *interlude_x = nullptr; // 动画偏移值
    int16_t *interlude_y = nullptr;
    int16_t *interlude_w = nullptr;
    int16_t *interlude_h = nullptr;

    float *x_now = nullptr; // 页面当前位置（绑定为面板坐标位置）
    float *y_now = nullptr;
    float *w_now = nullptr;
    float *h_now = nullptr;

    int16_t interface_text_x = 0; // 所有文本菜单坐标位置
    int16_t interface_text_y = 0;

    uint8_t font_height = 12; // 字体高度

    uint8_t isSelectable_start = 0; // 开头不可选中数量
    uint8_t isSelectable_end = 0;   // 结尾不可选中数量

    float pointer_x_now = 0; // 实时位置
    float pointer_y_now = 0;
    float pointer_w_now = 0;
    float pointer_h_now = 0;

    float interface_text_x_now = 0; // 实时位置
    float interface_text_y_now = 0;
    float progress_bar_black_x_now = 0; // 实时位置

    float progress_bar_x1_now = 6; // 实时位置
    float progress_bar_x2_now = 6;
    int16_t progress_bar_x1 = 0;      // 滚动条点1
    int16_t progress_bar_x2 = 0;      // 滚动条点2
    int16_t progress_bar_black_x = 0; // 滚动条外框
    bool progress_bar_isok = 0;       // 滚动条外框是否显现

    int time_ = 0;
    bool *lucency = nullptr;    // 是否背景透明
    bool *if_display = nullptr; // 是否显示
    bool *if_Input = nullptr;   // 是否输入

    bool handleInput = 0;   // 选中选项判断开关
    IN_PUT_Mode last_input; // 上一次输入状态

    bool keyboard_isok = 0;
    short keyboard_isok_number = 0;

    unsigned long lastMillis = 0;

    textMenuCallback callback;

    bool *trigger;

    Axeuh_UI_TextMenu() {}

    Axeuh_UI_TextMenu(MenuOption *menuOptions_, uint8_t count)
    {
        this->menuOptions = menuOptions_; // 保存指针
        this->menuOptions_index = count;  // 保存数量
        uint16_t *newmenu_str_len = new uint16_t[count];
        memset(newmenu_str_len, 0, sizeof(uint16_t) * count);
        menu_str_len = newmenu_str_len;
    }
    Axeuh_UI_TextMenu(const MenuOption *menuOptions_, uint8_t count)
    {
        this->const_menuOptions = menuOptions_; // 保存指针
        this->menuOptions_index = count;        // 保存数量
        uint16_t *newmenu_str_len = new uint16_t[count];
        memset(newmenu_str_len, 0, sizeof(uint16_t) * count);
        menu_str_len = newmenu_str_len;
    }

    static void staticCallback(Axeuh_UI_TextMenu *arg, Axeuh_UI_Panel *p, Axeuh_UI *m)
    {
        if (arg->const_menuOptions == nullptr)
        {
            if (arg->menuOptions[arg->meun_number_now].callback != nullptr)
            {
                arg->menuOptions[arg->meun_number_now].callback(p, m); // 触发当前选中项的回调
            }
            else if (arg->callback != nullptr)
            {
                arg->callback(p, m);
            }
        }
        else
        {
            if (arg->callback != nullptr)
            {
                arg->callback(p, m);
            }
        }
    }
    // 配置回调函数
    void set(textMenuCallback cb) { callback = cb; }
    // 配置选项菜单
    void set(MenuOption menuOptions_[], uint8_t count)
    {
        xSemaphoreTake(xMutex, 100);
        this->menuOptions = menuOptions_; // 保存指针
        this->menuOptions_index = count;  // 保存数量
        uint16_t *newmenu_str_len = new uint16_t[count];
        memset(newmenu_str_len, 0, sizeof(uint16_t) * count);
        delete[] menu_str_len;
        memcpy(newmenu_str_len, menu_str_len, sizeof(uint16_t) * count);
        menu_str_len = newmenu_str_len;
        xSemaphoreGive(xMutex);
    }
    void set_index(uint8_t count)
    {
        xSemaphoreTake(xMutex, 100);
        this->menuOptions_index = count; // 保存数量
        uint16_t *newmenu_str_len = new uint16_t[count];
        memset(newmenu_str_len, 0, sizeof(uint16_t) * count);
        delete[] menu_str_len;
        memcpy(newmenu_str_len, menu_str_len, sizeof(uint16_t) * count);
        menu_str_len = newmenu_str_len;
        xSemaphoreGive(xMutex);
    }
    // 计算所有选项的长度
    void init_text_more()
    {
        for (int i = 0; i < menuOptions_index; i++)
        {
            MenuOption *mm;
            if (menuOptions != nullptr && const_menuOptions == nullptr)
                mm = menuOptions;
            else if (menuOptions == nullptr && const_menuOptions != nullptr)
                mm = (MenuOption *)const_menuOptions;
            MenuOption &m = mm[i];
            int name_leng = m.length();
            int all_len_ = 0;
            for (int alen = 0; alen < name_leng; alen++)
            {
                char currentChar = m.charAt(alen);
                if (currentChar == '\r' || currentChar == '\t' || currentChar == '\n')
                {
                    continue;
                }
                if (isChineseChar(currentChar))
                {
                    all_len_ += len_c.Get((m.substring(alen, alen + 3)).c_str());
                    alen += 2;
                }
                else
                {
                    all_len_ += len_c.Get((m.substring(alen, alen + 1)).c_str());
                }
            }
            menu_str_len[i] = all_len_;
        }
    }
    void draw_MenuOption(U8G2 *D, unsigned long currentMillis, int &text_height_now);
    void draw_textmenu(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m);
    void set_munber(uint8_t num) // 修改当前所聚焦的选项，会直接跳转到指定选项
    {
        xSemaphoreTake(xMutex, 100);
        if (num > meun_number_now && num < menuOptions_index)
        {
            in_put_now = DOWN;
            meun_number_now = num;
        }
        else if (num < meun_number_now && num >= 0)
        {
            in_put_now = UP;
            meun_number_now = num;
        }
        else
            in_put_now = STOP;
        xSemaphoreGive(xMutex);
    }
    void set_munber_start(uint8_t num, uint8_t i_y_n) // 修改当前所聚焦的选项，且设置页面y坐标
    {
        xSemaphoreTake(xMutex, 100);
        interface_text_y_now = i_y_n;
        interface_text_y = i_y_n;

        if (num > meun_number_now && num < menuOptions_index - 1)
        {
            in_put_now = DOWN;
            meun_number_now = num;
        }
        else if (num < meun_number_now && num > 0)
        {
            in_put_now = UP;
            meun_number_now = num;
        }
        else
            in_put_now = STOP;
        xSemaphoreGive(xMutex);
    }
    void set_meun_number_now(uint8_t num)
    {
        xSemaphoreTake(xMutex, 100);
        meun_number_now = num;
        xSemaphoreGive(xMutex);
    }

    void set_menuOptions_name(int n, const char *name_) // 设置选项名称
    {
        xSemaphoreTake(xMutex, 100);
        menuOptions[n].name = name_;
        init_text_more();
        xSemaphoreGive(xMutex);
    }
    void set_menuOptions_name(int n, String name_) // 设置选项名称
    {
        xSemaphoreTake(xMutex, 100);
        menuOptions[n].set_str(name_);
        init_text_more();
        xSemaphoreGive(xMutex);
    }

    // void set_menuOptions_x(int n, int x_) // 设置选项x偏移
    // {
    //     xSemaphoreTake(xMutex, 100);
    //     menuOptions[n].x = x_;
    //     xSemaphoreGive(xMutex);
    // }
    // void set_menuOptions_y(int n, int y_) // 设置选项y偏移
    // {
    //     xSemaphoreTake(xMutex, 100);
    //     menuOptions[n].y = y_;
    //     xSemaphoreGive(xMutex);
    // }

    bool get_animation_all_isok()
    {
        if ((int16_t)pointer_x == (int16_t)pointer_x_now &&
            (int16_t)pointer_y == (int16_t)pointer_y_now &&
            (int16_t)pointer_w == (int16_t)pointer_w_now &&
            (int16_t)pointer_h == (int16_t)pointer_h_now &&
            (int16_t)*x + (int16_t)*interlude_x == (int16_t)*x_now &&
            (int16_t)*y + (int16_t)*interlude_y == (int16_t)*y_now &&
            (int16_t)*w + (int16_t)*interlude_w == (int16_t)*w_now &&
            (int16_t)*h + (int16_t)*interlude_h == (int16_t)*h_now &&
            (int16_t)interface_text_x == (int16_t)interface_text_x_now &&
            (int16_t)interface_text_y == (int16_t)interface_text_y_now)
            return 1;
        else
            return 0;
    }
    bool get_animation_pointer_isok()
    {
        if ((int16_t)pointer_x == (int16_t)pointer_x_now &&
            (int16_t)pointer_y == (int16_t)pointer_y_now &&
            (int16_t)pointer_w == (int16_t)pointer_w_now &&
            (int16_t)pointer_h == (int16_t)pointer_h_now)
            return 1;
        else
            return 0;
    }
    bool get_animation_interface_isok()
    {
        // Serial.print((String)(*x + *interlude_x) + (String) " == " + (String)*x_now + (String) "  \t");
        // Serial.print((String)(*y + *interlude_y) + (String) " == " + (String)*y_now + (String) "  \t");
        // Serial.print((String)(*w + *interlude_w) + (String) " == " + (String)*w_now + (String) "  \t");
        // Serial.print((String)(*h + *interlude_h) + (String) " == " + (String)*h_now + (String) "  \t");
        // Serial.println();
        if (
            (int16_t)*x + (int16_t)*interlude_x == (int16_t)*x_now &&
            (int16_t)*y + (int16_t)*interlude_y == (int16_t)*y_now &&
            (int16_t)*w + (int16_t)*interlude_w == (int16_t)*w_now &&
            (int16_t)*h + (int16_t)*interlude_h == (int16_t)*h_now)
            return 1;
        else
            return 0;
    }
    bool get_animation_interface_text_isok()
    {
        if (
            (int16_t)interface_text_x == (int16_t)interface_text_x_now &&
            (int16_t)interface_text_y == (int16_t)interface_text_y_now)
            return 1;
        else
            return 0;
    }
    void inti_textmenu()
    {
        MenuOption *mm;
        if (menuOptions != nullptr && const_menuOptions == nullptr)
            mm = menuOptions;
        else if (menuOptions == nullptr && const_menuOptions != nullptr)
            mm = (MenuOption *)const_menuOptions;
        isSelectable_start = 0;
        isSelectable_end = 0;
        for (int i = 0; i < menuOptions_index; i++)
        {
            MenuOption &opt_isSelectable = mm[i];
            if (opt_isSelectable.isSelectable == No_Focusing)
                isSelectable_start++;
            else
                break;
        }
        for (int i = menuOptions_index - 1; i >= 0; i--)
        {
            MenuOption &opt_isSelectable = mm[i];
            if (opt_isSelectable.isSelectable == No_Focusing)
                isSelectable_end++;
            else
                break;
        }
    }
};

class Axeuh_UI_slider : public Axeuh_UI
{
private:
    unsigned long holdStartTime = 0;
    bool initialDelayPassed = false;
    int num_speed_up = 0;

public:
    Menu_slider s;

    // int16_t page_x = 10;
    // float page_x_now = 10;
    // int16_t page_y = -64;
    // float page_y_now = -64;

    int16_t *x = nullptr;
    int16_t *y = nullptr;
    int16_t *w = nullptr;
    int16_t *h = nullptr;
    int16_t *r = nullptr;

    int16_t *interlude_x = nullptr; // 过场动画
    int16_t *interlude_y = nullptr;
    int16_t *interlude_w = nullptr;
    int16_t *interlude_h = nullptr;
    int16_t *interlude_r = nullptr;

    float *x_now = nullptr;
    float *y_now = nullptr;
    float *w_now = nullptr;
    float *h_now = nullptr;
    float *r_now = nullptr;

    int16_t progress_w = 0;
    float progress_w_now = 0;
    int time_ = 0;
    int keyboard_isok_number = 0;
    bool keyboard_isok = 0;
    bool *lucency = nullptr;
    bool *if_display = 0;
    bool *if_Input = 0;
    bool handleInput = 0;

    bool *trigger;
    bool **return_num_pointer; // 返回所触发的开关

    float unit = 1; // 刻度单位

    unsigned long lastMillis = 0;

    MenuCallback_Ebook callback;

    Axeuh_UI_slider()
    {
        s.name = "";
        s.num = nullptr;
        s.min = 1;
        s.max = 100;
        unit = 1;
    }

    Axeuh_UI_slider(String name, float *num, int16_t min, int16_t max, float unit_ = 1)
    {
        s.name = name;
        s.num = num;
        s.min = min;
        s.max = max;
        unit = unit_;
    }

    void set(MenuCallback_Ebook cb) { callback = cb; } // 设置回调函数

    void set(String name, float *num, float min, float max, float unit_ = 1)
    {
        s.num_ = nullptr;
        s.num_uint16 = nullptr;
        s.name = name;
        s.num = num;
        s.min = min;
        s.max = max;
        unit = unit_;
    }
    void set(String name, int *num, float min, float max, float unit_ = 1)
    {
        s.num_uint16 = nullptr;
        s.num = nullptr;
        s.name = name;
        s.num_ = num;
        s.min = min;
        s.max = max;
        unit = unit_;
    }
    void set(String name, uint16_t *num, float min, float max, float unit_ = 1)
    {
        s.num_uint16 = num;
        s.num = nullptr;
        s.name = name;
        s.num_ = nullptr;
        s.min = min;
        s.max = max;
        unit = unit_;
    }

    void set_num()
    {
    }

    // 设置返回启用输入值
    void set_textmenu_Input_of(Axeuh_UI_TextMenu *t) { *return_num_pointer = t->if_Input; }

    bool get_animation_all_isok()
    {
        if ((int16_t)*x + (int16_t)*interlude_x == (int16_t)*x_now &&
            (int16_t)*y + (int16_t)*interlude_y == (int16_t)*y_now &&
            (int16_t)*w + (int16_t)*interlude_w == (int16_t)*w_now &&
            (int16_t)*h + (int16_t)*interlude_h == (int16_t)*h_now &&
            (int16_t)progress_w == (int16_t)progress_w_now)
            return 1;
        else
            return 0;
    }
    void drawSlider(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m);
};

// 定义面的结构体
struct Face
{
    byte vertices[4]; // 构成面的4个顶点索引
    float normal[3];  // 面法线向量（未旋转时的方向）
};
class Axeuh_UI_Cube : public Axeuh_UI
{
public:
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
    Axeuh_UI_Cube()
    {
    }

    // 旋转角度变量
    int16_t cube_x = 120, cube_y = 6;
    float cube_x_now = 64, cube_y_now = 32;
    float angleX = 0.06, angleY = 0.04, angleZ = 0.02;
    float angleX_now = 0, angleY_now = 0, angleZ_now = 0;
    float angleX_speed = 0.03, angleY_speed = 0.04, angleZ_speed = 0.02;

    float cube_scale = 3.5; // 显示缩放系数（将单位立方体放大到屏幕尺寸）
    float cube_scale_now = 15;

    // 定义立方体的8个顶点坐标（单位立方体，边长为2）
    // 坐标范围：x,y,z ∈ [-1,1]
    float cube[8][3] = {
        {-1, -1, -1}, // 顶点0：左前下
        {1, -1, -1},  // 顶点1：右前下
        {1, 1, -1},   // 顶点2：右前上
        {-1, 1, -1},  // 顶点3：左前上
        {-1, -1, 1},  // 顶点4：左后下
        {1, -1, 1},   // 顶点5：右后下
        {1, 1, 1},    // 顶点6：右后上
        {-1, 1, 1}    // 顶点7：左后上
    };

    // 定义立方体的6个面及其法线方向
    Face faces[6] = {
        {{0, 1, 2, 3}, {0, 0, -1}}, // 前面：法线指向-Z方向
        {{4, 5, 6, 7}, {0, 0, 1}},  // 后面：法线指向+Z方向
        {{0, 3, 7, 4}, {-1, 0, 0}}, // 左面：法线指向-X方向
        {{1, 5, 6, 2}, {1, 0, 0}},  // 右面：法线指向+X方向
        {{3, 2, 6, 7}, {0, 1, 0}},  // 顶面：法线指向+Y方向
        {{0, 4, 5, 1}, {0, -1, 0}}  // 底面：法线指向-Y方向
    };

    float rotatedCube[8][3]; // 存储旋转后的顶点坐标

    // 三维旋转函数（XYZ旋转顺序）
    void rotatePoint(float x, float y, float z,
                     float &outX, float &outY, float &outZ)
    {
        // X轴旋转（绕X轴旋转angleX弧度）
        float x1 = x;
        float y1 = y * cos(angleX_now) - z * sin(angleX_now);
        float z1 = y * sin(angleX_now) + z * cos(angleX_now);

        // Y轴旋转（绕Y轴旋转angleY弧度）
        float x2 = x1 * cos(angleY_now) + z1 * sin(angleY_now);
        float y2 = y1;
        float z2 = -x1 * sin(angleY_now) + z1 * cos(angleY_now);

        // Z轴旋转（绕Z轴旋转angleZ弧度）
        float x3 = x2 * cos(angleZ_now) - y2 * sin(angleZ_now);
        float y3 = x2 * sin(angleZ_now) + y2 * cos(angleZ_now);

        // 输出旋转后的坐标
        outX = x3; // 旋转后X坐标
        outY = y3; // 旋转后Y坐标
        outZ = z2; // 保留Z坐标用于深度判断
    }

    void set_cube(int16_t x_, int16_t y_) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        cube_x = x_;
        cube_y = y_;
        xSemaphoreGive(xMutex);
    }
    void set_cube(int16_t x_, int16_t y_, float scale_) // 设置立方体
    {
        xSemaphoreTake(xMutex, 100);
        cube_x = x_;
        cube_y = y_;
        cube_scale = scale_;
        xSemaphoreGive(xMutex);
    }

    void set_cube_now(int16_t x_, int16_t y_) // 设置实时坐标
    {
        xSemaphoreTake(xMutex, 100);
        cube_x_now = x_;
        cube_y_now = y_;
        xSemaphoreGive(xMutex);
    }
    void set_cube_now(int16_t x_, int16_t y_, float scale_) // 设置实时坐标
    {
        xSemaphoreTake(xMutex, 100);
        cube_x_now = x_;
        cube_y_now = y_;
        cube_scale_now = scale_;
        xSemaphoreGive(xMutex);
    }
    void set_scale(float scale) // 设置大小
    {
        xSemaphoreTake(xMutex, 100);
        cube_scale = scale;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_speed_x(float x) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleX_speed = x;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_speed_y(float y) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleY_speed = y;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_speed_z(float z) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleZ_speed = z;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_speed(float x, float y, float z) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleX_speed = x;
        angleY_speed = y;
        angleZ_speed = z;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_x(float x) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleX = x;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_y(float y) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleY = y;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate_z(float z) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleZ = z;
        xSemaphoreGive(xMutex);
    }
    void set_cube_rotate(float x, float y, float z) // 设置坐标
    {
        xSemaphoreTake(xMutex, 100);
        angleX = x;
        angleY = y;
        angleZ = z;
        xSemaphoreGive(xMutex);
    }

    int16_t get_cube_x() { return cube_x; }
    int16_t get_cube_y() { return cube_y; }

    float get_angleX() { return angleX; }
    float get_angleY() { return angleY; }
    float get_angleZ() { return angleZ; }
    float get_angleX_speed() { return angleX_speed; }
    float get_angleY_speed() { return angleY_speed; }
    float get_angleZ_speed() { return angleZ_speed; }
    float get_scale() { return cube_scale; } // 获取大小

    void drawCube(U8G2 *D, Axeuh_UI *m);
};
class Axeuh_UI_StatusBar : public Axeuh_UI
{
public:
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
    SimpleKalmanFilter adc_filter;
    StatusBar_Callback draw;
    unsigned long previousMillis;

    Axeuh_UI_StatusBar()
    {
        adc_filter = SimpleKalmanFilter(1, 10, 2000);
    }

    int16_t y = 0;
    float y_now = 0;

    bool if_display = 0;
    int16_t font_offset_x = 0;
    int16_t font_offset_y = 10;
    void set_y(int16_t y_)
    {
        xSemaphoreTake(xMutex, 100);
        y = y_;
        xSemaphoreGive(xMutex);
    }
    void set_y_now(int16_t y_)
    {
        xSemaphoreTake(xMutex, 100);
        y_now = y_;
        xSemaphoreGive(xMutex);
    }

    void set_draw(StatusBar_Callback d)
    {
        xSemaphoreTake(xMutex, 100);
        draw = d;
        xSemaphoreGive(xMutex);
    }

    void drawStatusBar(U8G2 *D, Axeuh_UI *m)
    {
        // 状态栏随便写的，可根据自己需求写，后续会添加自定义回调绘制函数
        xSemaphoreTake(xMutex, 100);

        if (draw != nullptr)
            draw(D, m);
        else
        {
            D->setDrawColor(1);
            D->drawUTF8(0, y_now + font_offset_y, ((String) "贺吸呼" + (String)m->fps + (String) "fps").c_str());
            animation(&y_now, y, m->fps / 3 * 5);
        }

        xSemaphoreGive(xMutex);
    }
};

class Axeuh_UI_Ebook : public Axeuh_UI
{
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

public:
    MenuCallback_Ebook callback;

    int16_t *x = nullptr;
    int16_t *y = nullptr;
    int16_t *w = nullptr;
    int16_t *h = nullptr;

    int16_t *interlude_x = nullptr; // 过场动画
    int16_t *interlude_y = nullptr;
    int16_t *interlude_w = nullptr;
    int16_t *interlude_h = nullptr;

    float *x_now = nullptr;
    float *y_now = nullptr;
    float *w_now = nullptr;
    float *h_now = nullptr;

    int16_t page_x = 0;
    int16_t page_y = 0;
    float page_x_now = 0;
    float page_y_now = 0;

    float progress_bar_black_x_now = 0;
    float progress_bar_x1_now = 6;
    float progress_bar_x2_now = 6;
    int16_t progress_bar_x1 = 0;
    int16_t progress_bar_x2 = 0;
    int16_t progress_bar_black_x = 0;
    bool progress_bar_isok = 0;

    String s = "";                 // 文本内容
    int16_t s_x = 0;               // x坐标
    int16_t s_y = 0;               // y坐标
    OptionMode mode = TEXT_MORE;   // 模式
    alignMode align = LEFT_CORNER; // 对齐方式
    uint16_t s_len = 0;            // 文本长度
    uint8_t font_height = 12;      // 字体高度

    uint16_t s_h = 0; // 文本高度

    bool *lucency = nullptr;
    bool *if_display = nullptr;
    bool *if_Input = nullptr;

    bool handleInput = 0;
    bool **return_num_pointer; // 返回所触发的开关
    bool *trigger;

    int text_height_now = 0;

    Axeuh_UI_Ebook()
    {
    }

    Axeuh_UI_Ebook(String s_, int16_t s_x_ = 0, int16_t s_y_ = 0, alignMode a = LEFT_CORNER)
    {
        s = s_;
        s_x = s_x_;
        s_y = s_y_;
        align = a;
    }
    void drawEbook(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m);

    void down()
    {
        xSemaphoreTake(xMutex, 100);
        int x_ = 0, y_ = 0;
        for (int j = 0; j < s.length(); j++)
        {
            char currentChar = s.charAt(j);

            if (currentChar == '\r' || currentChar == '\t' || currentChar == '\n')
            {
                y_ += font_height;
                x_ = 0;
            }
            else if (isChineseChar(currentChar))
            {
                if (x_ + font_height > *w_now - font_height - 6)
                {
                    y_ += font_height;
                    x_ = 0;
                }
                else
                    x_ += len_c.Get((s.substring(j, j + 3)).c_str());
                j += 2;
            }
            else
            {
                int sub = len_c.Get((s.substring(j, j + 1)).c_str());
                if (x_ + sub > *w_now - sub - 6)
                {
                    y_ += font_height;
                    x_ = 0;
                }
                else
                    x_ += sub;
            }
        }

        page_y = -y_ + *interlude_h + *h - 3;
        xSemaphoreGive(xMutex);
    }
    void print(String a)
    {
        xSemaphoreTake(xMutex, 100);
        s += a;
        xSemaphoreGive(xMutex);
        down();
    }
    void println(String a = "")
    {
        xSemaphoreTake(xMutex, 100);
        s += a;
        s += "\n";
        xSemaphoreGive(xMutex);
        down();
    }

    void set(MenuCallback_Ebook cb) { callback = cb; } // 设置回调函数
    void set(String s_, int16_t s_x_ = 0, int16_t s_y_ = 0, alignMode a = LEFT_CENTER)
    {
        s = s_;
        s_x = s_x_;
        s_y = s_y_;
        align = a;
    }

    static void staticCallback(Axeuh_UI_Ebook *arg, Axeuh_UI_Panel *P, Axeuh_UI *m)
    {
        if (arg->callback != nullptr)
        {
            arg->callback(P, m); // 如果有回调函数，则触发，没有则触发开关
        }
        else
        {
            **arg->return_num_pointer = 1;
            *arg->if_display = 0;
            *arg->if_Input = 0;
            *arg->interlude_y = -64;
        }
    }

    bool get_animation_all_isok()
    {
        if ((int16_t)*x + (int16_t)*interlude_x == (int16_t)*x_now &&
            (int16_t)*y + (int16_t)*interlude_y == (int16_t)*y_now &&
            (int16_t)*w + (int16_t)*interlude_w == (int16_t)*w_now &&
            (int16_t)*h + (int16_t)*interlude_h == (int16_t)*h_now)
            return 1;
        else
            return 0;
    }

    void init_text_more() // 计算文本长度
    {
        if (mode == TEXT_MORE)
        {
            int name_leng = s.length();
            int all_len_ = 0;
            for (int alen = 0; alen < name_leng; alen++)
            {
                char currentChar = s.charAt(alen);
                if (currentChar == '\r' || currentChar == '\t' || currentChar == '\n')
                {
                    continue;
                }
                if (isChineseChar(currentChar))
                {
                    all_len_ += len_c.Get((s.substring(alen, alen + 3)).c_str());
                    alen += 2;
                }
                else
                {
                    all_len_ += len_c.Get((s.substring(alen, alen + 1)).c_str());
                }
            }
            s_len = all_len_;
        }
    }
};

#ifdef CHINESE_KEYBOARD
class Axeuh_UI_Keyboard : public Axeuh_UI // 史山Keyboard
{
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();

public:
    int16_t *x = nullptr;
    int16_t *y = nullptr;
    int16_t *w = nullptr;
    int16_t *h = nullptr;

    int16_t *interlude_x = nullptr; // 过场动画
    int16_t *interlude_y = nullptr;
    int16_t *interlude_w = nullptr;
    int16_t *interlude_h = nullptr;

    float *x_now = nullptr;
    float *y_now = nullptr;
    float *w_now = nullptr;
    float *h_now = nullptr;

    PINYIN_MEUN_ key;
    int keyboard_now;

    bool Pointer_dir = 0;

    int time_ = 0;

    bool CE_of = 0;
    int CE_num = 0;
    int CE_num_max = 0;
    String CE_out = "";

    bool num_of = 0;

    // MenuOption::AutolenString *Aoutput = nullptr;
    String *output = nullptr;

    bool *lucency = nullptr;
    bool *if_display = nullptr;
    bool *if_Input = nullptr;
    bool **return_num_pointer;
    bool handleInput = 0;
    bool *trigger;

    unsigned long lastMillis = 0;

    bool SELECT_isok = 0;
    unsigned long SELECT_time = 0;

    MenuCallback_Ebook callback;

    Axeuh_UI_Keyboard()
    {
    }
    // Axeuh_UI_Keyboard(MenuOption::AutolenString *Aoutput)
    // {
    //     xSemaphoreTake(xMutex, 100);
    //     this->Aoutput = Aoutput;
    //     xSemaphoreGive(xMutex);
    // }
    Axeuh_UI_Keyboard(String *output)
    {
        xSemaphoreTake(xMutex, 100);
        this->output = output;
        xSemaphoreGive(xMutex);
    }
    // void set(MenuOption::AutolenString *Aoutput)
    // {
    //     xSemaphoreTake(xMutex, 100);
    //     this->Aoutput = Aoutput;
    //     xSemaphoreGive(xMutex);
    // }
    void set(String *output)
    {
        xSemaphoreTake(xMutex, 100);
        this->output = output;
        xSemaphoreGive(xMutex);
    }

    void keyboard_init()
    {
        key.input_box = 0;
        key.key_boa_ui = 0;
        key.cn_box_ui = 0;
        key.input_box_now = -12; // 动画
        key.key_boa_ui_now = -128;
        key.cn_box_ui_now = 24;
    }

    bool get_animation_interface_isok()
    {
        if (
            key.input_box == key.input_box_now &&
            key.key_boa_ui == key.key_boa_ui_now &&
            key.cn_box_ui == key.cn_box_ui_now)
            return 1;
        else
            return 0;
    }
    void drawKeyboard(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m);
    void SELECT_(String output_str, String mystring, const char **key_arr);
};
#endif
class Axeuh_UI_Panel : public Axeuh_UI // 面板类
{
public:
    SemaphoreHandle_t xMutex = xSemaphoreCreateMutex(); // 互斥锁

    int16_t x = 0; // 页面期望位置
    int16_t y = 12;
    int16_t w = 128;
    int16_t h = 64 - 12;
    int16_t r = 0;

    int16_t interlude_x = 0; // 动画偏移值
    int16_t interlude_y = 0;
    int16_t interlude_w = -125;
    int16_t interlude_h = 0;
    int16_t interlude_r = 0;

    float x_now = 0; // 页面实时位置
    float y_now = 12;
    float w_now = 3;
    float h_now = 64 - 12;
    float r_now = 0;

    bool lucency = 0;         // 是否透明
    bool *return_num_pointer; // 返回所触发的指针（返回为真）
    bool if_display = 1;      // 是否显示
    bool if_Input = 1;        // 按键是否输入控件

    bool handleInput = 0;

    bool trigger = 0;

    Axeuh_UI_TextMenu *text = nullptr;      // 菜单实例
    Axeuh_UI_Panel *Panel_ = nullptr;       // 子面板
    Axeuh_UI_Panel *Parent_Panel = nullptr; // 父级面板
    Axeuh_UI_slider *slider_ = nullptr;     // 滑动条实例
    Axeuh_UI_Ebook *Ebook = nullptr;        // 文本窗口实例
#ifdef CHINESE_KEYBOARD
    Axeuh_UI_Keyboard *keyboard = nullptr; // 键盘实例
#endif
    Menu_gif *gif = nullptr; // 图片实例

    Panel_draw_callback Panel_draw;

    uint8_t font_height = 12; // 字体高度

    Axeuh_UI_Panel() {}

    Axeuh_UI_Panel(Axeuh_UI_Ebook *Ebook_)
    {
        set(Ebook_);
    }
    Axeuh_UI_Panel(Axeuh_UI_TextMenu *text_)
    {
        set(text_);
    }
    Axeuh_UI_Panel(Axeuh_UI_slider *slider)
    {
        set(slider);
    }
    Axeuh_UI_Panel(Menu_gif *gif_)
    {
        set(gif_);
    }
    Axeuh_UI_Panel(Axeuh_UI_Keyboard *k)
    {
        set(k);
    }
    Axeuh_UI_Panel(Axeuh_UI_Panel *k)
    {
        set(k);
    }

#ifdef CHINESE_KEYBOARD
    void set(Axeuh_UI_Keyboard *keyboard_) // 绑定keyboard实例
    {
        xSemaphoreTake(xMutex, 10);
        keyboard = keyboard_;
        keyboard->x = &x;
        keyboard->y = &y;
        keyboard->w = &w;
        keyboard->h = &h;

        keyboard->interlude_x = &interlude_x;
        keyboard->interlude_y = &interlude_y;
        keyboard->interlude_w = &interlude_w;
        keyboard->interlude_h = &interlude_h;

        keyboard->x_now = &x_now;
        keyboard->y_now = &y_now;
        keyboard->w_now = &w_now;
        keyboard->h_now = &h_now;

        keyboard->lucency = &lucency;
        keyboard->if_Input = &if_Input;
        keyboard->if_display = &if_display;
        keyboard->return_num_pointer = &return_num_pointer;
        keyboard->trigger = &trigger;

        xSemaphoreGive(xMutex);
    }
#endif
    void set(Axeuh_UI_Ebook *Ebook_) // 绑定Ebook实例
    {
        xSemaphoreTake(xMutex, 10);
        Ebook = Ebook_;
        Ebook->x = &x;
        Ebook->y = &y;
        Ebook->w = &w;
        Ebook->h = &h;

        Ebook->interlude_x = &interlude_x;
        Ebook->interlude_y = &interlude_y;
        Ebook->interlude_w = &interlude_w;
        Ebook->interlude_h = &interlude_h;

        Ebook->x_now = &x_now;
        Ebook->y_now = &y_now;
        Ebook->w_now = &w_now;
        Ebook->h_now = &h_now;

        Ebook->lucency = &lucency;
        Ebook->if_Input = &if_Input;
        Ebook->if_display = &if_display;
        Ebook->return_num_pointer = &return_num_pointer;
        Ebook->trigger = &trigger;
        xSemaphoreGive(xMutex);
    }
    void set(Axeuh_UI_TextMenu *text_) // 绑定菜单实例
    {
        xSemaphoreTake(xMutex, 100);
        text = text_;
        text->x = &x;
        text->y = &y;
        text->w = &w;
        text->h = &h;

        text->interlude_x = &interlude_x;
        text->interlude_y = &interlude_y;
        text->interlude_w = &interlude_w;
        text->interlude_h = &interlude_h;

        text->x_now = &x_now;
        text->y_now = &y_now;
        text->w_now = &w_now;
        text->h_now = &h_now;

        text->lucency = &lucency;
        text->if_display = &if_display;
        text->if_Input = &if_Input;
        text->trigger = &trigger;

        text->inti_textmenu();
        xSemaphoreGive(xMutex);
    }
    void set(Axeuh_UI_slider *slider) // 绑定滑动条实例
    {
        xSemaphoreTake(xMutex, 10);
        slider_ = slider;
        slider_->x = &x;
        slider_->y = &y;
        slider_->w = &w;
        slider_->h = &h;
        slider_->r = &r;

        slider_->interlude_x = &interlude_x;
        slider_->interlude_y = &interlude_y;
        slider_->interlude_w = &interlude_w;
        slider_->interlude_h = &interlude_h;
        slider_->interlude_r = &interlude_r;

        slider_->x_now = &x_now;
        slider_->y_now = &y_now;
        slider_->w_now = &w_now;
        slider_->h_now = &h_now;
        slider_->r_now = &r_now;

        slider_->lucency = &lucency;
        slider_->if_display = &if_display;
        slider_->if_Input = &if_Input;

        slider_->return_num_pointer = &return_num_pointer;
        slider_->trigger = &trigger;

        xSemaphoreGive(xMutex);
    }
    void set(Menu_gif *gif_) // 绑定图片实例
    {
        xSemaphoreTake(xMutex, 100);
        gif = gif_;
        xSemaphoreGive(xMutex);
    }

    // void replace(Axeuh_UI_Panel *p) // 替换本体
    // {
    //     xSemaphoreTake(xMutex, 100);

    //     if (Parent_Panel != nullptr)
    //         Parent_Panel->Panel_ = p;

    //     xSemaphoreGive(xMutex);
    // }

    void set(Axeuh_UI_Panel *p) // 绑定子级
    {
        xSemaphoreTake(xMutex, 100);
        Panel_ = p;
        Panel_->Parent_Panel = this;
        p->return_num_pointer = &this->if_Input;
        xSemaphoreGive(xMutex);
    }

    void set_Callback(textMenuCallback cb) // 绑定回调函数
    {
        xSemaphoreTake(xMutex, 100);
        if (text != nullptr)
            text->callback = cb;
        if (Ebook != nullptr)
            Ebook->callback = cb;
        if (slider_ != nullptr)
            slider_->callback = cb;
        if (keyboard != nullptr)
            keyboard->callback = cb;
        xSemaphoreGive(xMutex);
    }

    void set_draw(Panel_draw_callback cb) // 绑定回调函数
    {
        xSemaphoreTake(xMutex, 100);
        Panel_draw = cb;
        xSemaphoreGive(xMutex);
    }

    // void set(MenuCallback_Ebook cb)//绑定回调函数
    // {
    //     if (Ebook != nullptr)
    //         Ebook->callback = cb;
    // }

    void display_off() // 不显示
    {
        xSemaphoreTake(xMutex, 100);
        if_display = 0;
        xSemaphoreGive(xMutex);
    }
    void display_of() // 显示
    {
        xSemaphoreTake(xMutex, 100);
        if_display = 1;
        xSemaphoreGive(xMutex);
    }
    void Input_off() // 禁用输入
    {
        xSemaphoreTake(xMutex, 100);
        if_Input = 0;
        xSemaphoreGive(xMutex);
    }
    void Input_of() // 开启输入
    {
        xSemaphoreTake(xMutex, 100);
        if_Input = 1;
        xSemaphoreGive(xMutex);
    }
    void off() // 关闭控件
    {
        xSemaphoreTake(xMutex, 100);
        if_display = 0;
        if_Input = 0;
        xSemaphoreGive(xMutex);
    }
    void of() // 开启控件
    {
        xSemaphoreTake(xMutex, 100);
        if_display = 1;
        if_Input = 1;
        xSemaphoreGive(xMutex);
    }
    void set_display_of(bool if_) // 改变显示状态
    {
        xSemaphoreTake(xMutex, 100);
        if_display = if_;
        xSemaphoreGive(xMutex);
    }
    void set_Input_of(bool if_) // 改变输入状态
    {
        xSemaphoreTake(xMutex, 100);
        if_Input = if_;
        xSemaphoreGive(xMutex);
    }
    void set_lucency(bool l) // 改变是否透明
    {
        xSemaphoreTake(xMutex, 100);
        lucency = l;
        xSemaphoreGive(xMutex);
    }
    void set_trigger(bool t) // 改变是否透明
    {
        xSemaphoreTake(xMutex, 100);
        trigger = t;
        xSemaphoreGive(xMutex);
    }
    void set_x(int16_t x_)
    {
        xSemaphoreTake(xMutex, 100);
        x = x_;
        xSemaphoreGive(xMutex);
    }
    void set_y(int16_t y_)
    {
        xSemaphoreTake(xMutex, 100);
        y = y_;
        xSemaphoreGive(xMutex);
    }
    void set_w(int16_t w_)
    {
        xSemaphoreTake(xMutex, 100);
        w = w_;
        xSemaphoreGive(xMutex);
    }
    void set_h(int16_t h_)
    {
        xSemaphoreTake(xMutex, 100);
        h = h_;
        xSemaphoreGive(xMutex);
    }
    void set_r(int16_t r_)
    {
        xSemaphoreTake(xMutex, 100);
        r = r_;
        xSemaphoreGive(xMutex);
    }
    void set_interface(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r) // 设置页面目标坐标
    {
        xSemaphoreTake(xMutex, 100);
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        this->r = r;
        xSemaphoreGive(xMutex);
    }
    void set_interface(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        xSemaphoreTake(xMutex, 100);
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now_w(int16_t w_)
    {
        xSemaphoreTake(xMutex, 100);
        w_now = w_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now_x(int16_t x_)
    {
        xSemaphoreTake(xMutex, 100);
        x_now = x_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now_y(int16_t y_)
    {
        xSemaphoreTake(xMutex, 100);
        y_now = y_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now_h(int16_t h_)
    {
        xSemaphoreTake(xMutex, 100);
        h_now = h_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now_r(int16_t r_)
    {
        xSemaphoreTake(xMutex, 100);
        r_now = r_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now(Axeuh_UI_Panel *p) // 继承实时坐标
    {
        xSemaphoreTake(xMutex, 100);
        set_interface_now(p->x_now, p->y_now, p->w_now, p->h_now, p->r_now);
        xSemaphoreGive(xMutex);
    }
    void set_interface_now(int16_t x_, int16_t y_, int16_t w_, int16_t h_, int16_t r_) // 设置实时坐标
    {
        xSemaphoreTake(xMutex, 100);
        x_now = x_;
        y_now = y_;
        w_now = w_;
        h_now = h_;
        r_now = r_;
        xSemaphoreGive(xMutex);
    }
    void set_interface_now(int16_t x_, int16_t y_, int16_t w_, int16_t h_)
    {
        xSemaphoreTake(xMutex, 100);
        x_now = x_;
        y_now = y_;
        w_now = w_;
        h_now = h_;
        xSemaphoreGive(xMutex);
    }
    void set_interface(Axeuh_UI_Panel *p)
    {
        xSemaphoreTake(xMutex, 100);
        set_interface_now(p->x, p->y, p->w, p->h, p->r);
        xSemaphoreGive(xMutex);
    }
    void set_interlude(int16_t x_, int16_t y_, int16_t w_, int16_t h_) // 设置动画偏移值
    {
        xSemaphoreTake(xMutex, 100);
        interlude_x = x_;
        interlude_y = y_;
        interlude_w = w_;
        interlude_h = h_;

        xSemaphoreGive(xMutex);

        if (Panel_ != nullptr)
        {
            if (Panel_->if_display)
            {
                if (w_ == -(w - 3) || w_ < -(Panel_->w - 3))
                    Panel_->set_interlude(x_, y_, -(Panel_->w - 3), h_);
                else
                    Panel_->set_interlude(x_, y_, w_, h_);
            }
        }
    }
    void set_interlude(int16_t x_, int16_t y_, int16_t w_, int16_t h_, int16_t r_)
    {
        xSemaphoreTake(xMutex, 100);
        interlude_x = x_;
        interlude_y = y_;
        interlude_w = w_;
        interlude_h = h_;
        interlude_r = r_;
        xSemaphoreGive(xMutex);
    }
    void set_interlude_x(int16_t x_)
    {
        xSemaphoreTake(xMutex, 10);
        interlude_x = x_;
        xSemaphoreGive(xMutex);
    }
    void set_interlude_y(int16_t y_)
    {
        xSemaphoreTake(xMutex, 10);
        interlude_y = y_;
        xSemaphoreGive(xMutex);
    }
    void set_interlude_w(int16_t w_)
    {
        xSemaphoreTake(xMutex, 10);
        interlude_w = w_;
        xSemaphoreGive(xMutex);
    }
    void set_interlude_h(int16_t h_)
    {
        xSemaphoreTake(xMutex, 10);
        interlude_h = h_;
        xSemaphoreGive(xMutex);
    }
    void set_interlude_r(int16_t r_)
    {
        xSemaphoreTake(xMutex, 10);
        interlude_r = r_;
        xSemaphoreGive(xMutex);
    }

    bool get_trigger() { return trigger; }

    int16_t get_x() { return x; }
    int16_t get_y() { return y; }
    int16_t get_w() { return w; }
    int16_t get_h() { return h; }

    float get_x_now() { return x_now; }
    float get_y_now() { return y_now; }
    float get_w_now() { return w_now; }
    float get_h_now() { return h_now; }

    int16_t get_interlude_x() { return interlude_x; }
    int16_t get_interlude_y() { return interlude_y; }
    int16_t get_interlude_w() { return interlude_w; }
    int16_t get_interlude_h() { return interlude_h; }

    int16_t get_textmenu_num_now() // 获取当前文本菜单聚焦选项
    {
        if (text != nullptr)
            return text->meun_number_now;
        else
            return -1;
    }

    bool get_animation_all_isok() // 检查动画是否结束
    {
        if (text != nullptr)
            return text->get_animation_interface_isok();
        else if (slider_ != nullptr)
            return slider_->get_animation_all_isok();
        else if (Ebook != nullptr)
            return Ebook->get_animation_all_isok();
#ifdef CHINESE_KEYBOARD
        else if (keyboard != nullptr)
            return keyboard->get_animation_interface_isok();
#endif
        else if ((int16_t)x + (int16_t)interlude_x == (int16_t)x_now &&
                 (int16_t)y + (int16_t)interlude_y == (int16_t)y_now &&
                 (int16_t)w + (int16_t)interlude_w == (int16_t)w_now &&
                 (int16_t)h + (int16_t)interlude_h == (int16_t)h_now)
            return 1;
        else
            return 0;
    }

    void drawPanel(U8G2 *D, Axeuh_UI *m, IN_PUT_Mode IN);
};

#endif
