# Arduino 交互式 UI 系统

基于 Arduino 平台和 Axeuh_UI 库开发的综合界面系统，集成多级菜单、动态图形、3D 渲染和硬件交互功能。基于 U8G2 库实现高性能显示驱动。

GIF 或图片的转换在这里 https://javl.github.io/image2cpp/ 记得勾 swap 选项
(GIF 需要先转换成一帧一帧的图片)

![系统演示](image.png)

## 目录

- [功能特性](#功能特性)
- [硬件要求](#硬件要求)
- [快速开始](#快速开始)
  - [安装依赖](#安装依赖)
  - [硬件连接](#硬件连接)
  - [代码配置](#代码配置)
- [系统架构](#系统架构)
- [核心类说明](#核心类说明)
- [使用示例](#使用示例)
- [API 参考](#api参考)
- [开发指南](#开发指南)
- [常见问题](#常见问题)
- [许可证](#许可证)

---

## 功能特性

### 核心功能

- **多级菜单系统**

  - 支持无限级菜单嵌套
  - 动态菜单项生成（`MenuOption`结构）
  - 图文混合显示（`PICTURE_TEXT`模式）
  - 菜单动画过渡效果（渐进动画函数）

- **图形渲染引擎**

  - 128x64 OLED 显示驱动（U8G2 集成）
  - GIF 动画支持（`Menu_gif`结构）
  - 实时 3D 立方体渲染（`Axeuh_UI_Cube`类）

- **交互组件库**

  - 参数滑动调节条（`Axeuh_UI_slider`类）
  - 中文拼音输入键盘（`Axeuh_UI_Keyboard`类）
  - 状态栏组件（`Axeuh_UI_StatusBar`类）
  - 弹窗（`Axeuh_UI_Panel`类）

- **系统特性**
  - 异步 UI 刷新（`menu_display_xtaskbegin`）
  - 硬件中断优化（`xSemaphore`互斥锁）
  - 动态内存管理（`CharLenMap`字符缓存）
  - 低功耗模式支持

### 扩展功能

- 串口配置接口
- 拼音键盘
- 适配不同大小的屏幕
- 屏幕旋转支持（0°/90°/180°/270°）
- 适配不同字库（后续添加）

---

## 硬件要求

### 必需组件

| 组件        | 规格要求          | 推荐型号     | 接口说明 |
| ----------- | ----------------- | ------------ | -------- |
| 主控板      | 支持 Arduino 框架 | ESP32 DevKit | -        |
| OLED 显示屏 | 128x64 分辨率     | SSD1306      | SPI/I2C  |
| 输入设备    | 5 向导航+确认键   | 无           | ADC      |

### 推荐配置

- **处理器性能**
  - Flash 存储：≥​​512KB（用于存储图形资源）
  - SRAM：≥48KB
  - 时钟速度：≥72MHz（流畅动画）

---

## 快速开始

### 安装依赖

#### PlatformIO

```ini
lib_deps =
    https://github.com/Axeuh/Axeuh_UI.git
```

#### Arduino IDE

1. 通过库管理器安装 `U8g2`
2. 下载[Axeuh_UI 库 ZIP](https://github.com/Axeuh/Axeuh_UI/archive/main.zip)
3. 菜单栏：项目 > 加载库 > 添加.ZIP 库

### 硬件连接

```cpp
/* 典型接线示例 (ESP32) */
#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_CS 5
#define OLED_DC 17
#define OLED_Reset 16
#define HW_X 34  // 摇杆x
#define HW_Y 35  // 摇杆y
#define HW_SW 36 // 摇杆按键
```

### 示例

```cpp
/*
 * 基于Arduino的UI系统示例代码，使用Axeuh_UI库实现复杂界面交互
 * 包含矩阵键盘输入、OLED显示、多级菜单、动画、滑动条、3D立方体等多种功能
 */

#include <Arduino.h>
#include "Axeuh_UI.h"
#include "gif.h"
#include <Wire.h>
#include <SPI.h>
// 硬件配置 --------------------------------------------------------

#define OLED_MOSI 23
#define OLED_CLK 18
#define OLED_CS 5
#define OLED_DC 17
#define OLED_Reset 16
#define HW_X 34  // 摇杆x
#define HW_Y 35  // 摇杆y
#define HW_SW 36 // 摇杆按键

// 显示驱动配置 ----------------------------------------------------
// 使用硬件SPI的OLED显示配置（参数：旋转方向, CS引脚, DC引脚, Reset引脚）
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2_(U8G2_R0); //iic方案
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2_(U8G2_R0, OLED_CS, OLED_DC, OLED_Reset);
Axeuh_UI myui(&u8g2_); // 初始化UI系统

// 动画资源定义 ----------------------------------------------------
// 声明图片列表参数信息
// const unsigned char **frames = nullptr,       图片指针
// uint16_t frameCount_ = 0,       GIF帧数
// uint16_t xx = 0,       坐标偏移x
// uint16_t yy = 0,       坐标偏移y
// uint16_t width = 16,       图像宽度
// uint16_t height = 16,       图像高度
// uint8_t speed = 100,       刷新速度
// uint16_t startFrame = 0,       起始帧位置
// OptiongifMode1 aPlay = AutoPlay,       是否自动播放
// OptiongifMode2 hPlay = Show       是否有显现动画
Menu_gif my_gif_1 = {icons_Homer_SimpsonallArray, 1, 5, 2, 50, 50, 30, 0, AutoPlay, Hide}; // 动图
Menu_gif my_gif_2 = {epd_bitmap_allArray1, 30, 40, 2, 50, 50, 30, 0, ManualPlay, Show};    // 动图
Menu_gif my_gif_3 = {epd_bitmap_setallArray, 1, 4, 4, 16, 16, 30, 0, AutoPlay, Hide};      // 图片
Menu_gif my_gif_4 = {epd_bitmap_allArray1, 30, 40, 2, 50, 50, 30, 0, AutoPlay, Show};      // 动图

// 声明菜单列表参数信息
// String n = "",       选项文本内容
// uint8_t h = 12,       选项高度
// alignMode a = LEFT_CENTER,       选项文本对齐方式
// OptionMode m = TEXT,       选项模式
// Menu_gif *g = nullptr,       图片指针（如果要显示图片的话）
// OptionistriggersAnimation tr = No_Trigger,       是否触发切换菜单动画
// textMenuCallback cb = nullptr,       当前选项的回调函数
// OptionisSelectable is = Focused       该选项是否可选

MenuOption myOptions1[] = // 菜单信息
    {
        {"[ 首页 ]", 14, ALIGN_CENTER, TEXT, nullptr, No_Trigger, nullptr, No_Focusing},
        {"Axeuh_UI 2.0", 14},
        {"~ 设置fps上限", 14},
        {"~ 当前选项高度", 12},
        {"多行文本测试12345678", 24, LEFT_CENTER, TEXT_MORE},
        {"荷马辛普森", 50, LEFT_CENTER, PICTURE_TEXT, &my_gif_1},
        {"动图", 50, LEFT_CENTER, PICTURE_TEXT, &my_gif_2},
        {"~ 设置立方体", 14},
        {"设置", 20, LEFT_CENTER, PICTURE_TEXT, &my_gif_3, Trigger},
        {"~ 重启", 14},
        {"~ 键盘", 14, LEFT_CENTER, TEXT, nullptr, No_Trigger},
};
// 立方体设置子菜单
static MenuOption myOptions2[] = // 菜单信息
    {
        {"[ 设置立方体 ]", 14, LEFT_CENTER, TEXT, nullptr, No_Trigger, nullptr, No_Focusing},
        {"设为背景", 14},
        {"设置X速度", 14},
        {"设置Y速度", 14},
        {"设置Z速度", 14},
        {"设置大小", 14},
        {"~ 返回", 14},
};
static MenuOption myOptions3[] = // 菜单信息
    {
        {"[ 设置 ]", 14, LEFT_CENTER, TEXT, nullptr, No_Trigger, nullptr, No_Focusing},
        {"这里没有任何设置", 14},
        {"1", 14},
        {"2", 14},
        {"3", 14},
        {"4", 14},
        {"5", 14},
        {"~ 返回", 14, LEFT_CENTER, TEXT, nullptr, Trigger},
};
String version_t = R"(Axeuh_UI 2.0
折腾了几个星期
重构了代码
主要功能和特性:
采用异步运行ui
极高的可自定义化

可自定义选项高度,文本(图片)x,y位置
可设置的文字对齐模式（居中，坐上等）
选项可以显示图片(或者gif)或图文结合
选项支持多行文本（根据行宽自动换行）
选项gif可设置聚焦是否播放动)
选项gif或图片可设置是否聚焦显现(从左边界闪出)

新增文本框
选项界面支持创建子选项界面,类似于画中画
还加了个好玩的立方体
)";

// UI组件声明 ------------------------------------------------------
Axeuh_UI_StatusBar my_statusbar; // 声明状态栏
Axeuh_UI_Cube cube;              // 3D立方体

Axeuh_UI_TextMenu my_text_1_Panel(myOptions1, sizeof(myOptions1) / sizeof(myOptions1[0])); // 声明菜单
Axeuh_UI_TextMenu my_text_2(myOptions2, sizeof(myOptions2) / sizeof(myOptions2[0]));       // 声明菜单
Axeuh_UI_TextMenu my_text_3(myOptions3, sizeof(myOptions3) / sizeof(myOptions3[0]));       // 声明菜单

// 传递文本 String
Axeuh_UI_Ebook my_Ebook_1(version_t); // 声明文本查看窗口

// String name,        滑动条标题文本
// float *num,        对应修改值的指针
// int16_t min,        最小值
// int16_t max,        最大值
// float unit_ = 1       刻度单位
Axeuh_UI_slider my_slider("当前选项高度", nullptr, 12, 50); // 声明滑动条窗体

Axeuh_UI_Keyboard keyborad; // 声明拼音键盘

// 面板声明 ----------------------------------------------------
// 不重复的面板，可以保留多层级的菜单的位置等属性，而不是每次都要重新设置
Axeuh_UI_Panel my_Panel_1;
Axeuh_UI_Panel my_Panel_2;
Axeuh_UI_Panel my_Panel_3;
Axeuh_UI_Panel my_Panel_4;
Axeuh_UI_Panel my_Panel_text;
Axeuh_UI_Panel my_Panel_slider;
Axeuh_UI_Panel my_Panel_keyboard;

// 输入处理函数 ----------------------------------------------------
IN_PUT_Mode my_ui_input()
{
  if (!digitalRead(HW_SW))
    return SELECT; // 选中
  else if (analogRead(HW_Y) >= 3995)
    return DOWM; // 向下
  else if (analogRead(HW_Y) <= 100)
    return UP; // 向上
  else if (analogRead(HW_X) >= 3995)
    return LEFT; // 向左
  else if (analogRead(HW_X) <= 100)
    return RIGHT; // 向右
  return STOP;    // 无输入
}
// 回调函数组 ------------------------------------------------------
// 设置菜单回调
void AllCallback_my_text3(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
  // Axeuh_UI_Panel *p            为当前面板类
  // Axeuh_UI *m                总ui类
  // p->get_textmenu_num_now();  返回当前选项的索引
  int key = p->get_textmenu_num_now();
  if (key == 7)
  {
    m->set(&my_Panel_text);             // 将首级面板设置为my_Panel_text（替换当前面板）
    my_Panel_text.set_interface_now(p); // 继承当前面板的实时坐标
    my_Panel_text.set_interlude_w(0);   // 重置动画偏移值
    my_Panel_text.of();                 // 开启（打开显示，打开输入）
  }
  else if (key == 1)
  {
    p->set(&my_Panel_3);                  // 为当前面板添加子级面板，为my_Panel_3
    my_Panel_3.of();                      // 打开
    my_Panel_3.set_interlude(0, 0, 0, 0); // 重置动画偏移值
    p->Input_off();                       // 关闭当前面板输入
  }
}

void AllCallback_my_text2(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
  int key = p->get_textmenu_num_now(); // 获取当前选中选项

  if (key == 6)
  {
    // 此选项是返回上一级菜单
    p->Parent_Panel->of();   // 将当前面板的父级面板打开
    p->off();                // 关闭当前面板（关闭显示，关闭输入）
    p->set_interlude_y(-64); // 设置当前面板y动画偏移-64，以形成动画效果
  }
  else if (key == 1)
  {
    if (cube.get_scale() != 15) // 判断当前立方体大小是否为15
    {
      p->text->set_menuOptions_name(1, "~ 设为右上角"); // 将当前面板的菜单的选项1的名称设置为“设为右上角”
      cube.set_cube(64, 32, 15);                        // 设置立方体位置x为64，y为32和大小为15
    }
    else
    {
      p->text->set_menuOptions_name(1, "~ 设为背景");
      cube.set_cube(122, 6, 3.5);
    }
  }
  else if (key == 2)
  {
    p->set(&my_Panel_slider); // 添加子级面板
    p->Input_off();           // 关闭当前面板输入
    // 第一个为标题
    // 第二个为要修改的数值的指针  此处将立方体的X轴速度绑定给了滑动条
    // 第三个为最小值
    // 第四个为最大值
    // 第五个为单位
    my_Panel_slider.slider_->set("设置X速度", (float *)&m->cube->angleX_speed, 0, 1, 0.01); // 设置滑动条的属性
    my_Panel_slider.of();                                                                   // 打开
    my_Panel_slider.set_interface(19, 20, 90, 34);                                          // 设置坐标位置
    my_Panel_slider.set_interlude(0, 0, 0, 0);                                              // 重置动画偏移值
  }
  else if (key == 3)
  {
    p->set(&my_Panel_slider);
    p->Input_off();
    my_Panel_slider.slider_->set("设置Y速度", (float *)&m->cube->angleY_speed, 0, 1, 0.01);
    my_Panel_slider.of();
    my_Panel_slider.set_interface(19, 20, 90, 34); // 如果所有都共用一个滑动条，那么此处修改坐标位置信息是必要的，
    my_Panel_slider.set_interlude(0, 0, 0, 0);     // 不然会使用上一次的信息，无所谓的话也可以不写，但是在setup要设置好坐标位置信息
  }
  else if (key == 4)
  {
    p->set(&my_Panel_slider);
    p->Input_off();
    my_Panel_slider.slider_->set("设置Z速度", (float *)&m->cube->angleZ_speed, 0, 1, 0.01);
    my_Panel_slider.of();
    my_Panel_slider.set_interface(19, 20, 90, 34);
    my_Panel_slider.set_interlude(0, 0, 0, 0);
  }
  else if (key == 5)
  {
    p->set(&my_Panel_slider);
    p->Input_off();
    my_Panel_slider.slider_->set("设置大小", (float *)&m->cube->cube_scale, 0, 18, 0.1);
    my_Panel_slider.of();
    my_Panel_slider.set_interface(19, 20, 90, 34);
    my_Panel_slider.set_interlude(0, 0, 0, 0);
  }
}

void AllCallback_my_Popup_text1(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
  int key = 0;
  key = p->get_textmenu_num_now();

  if (key == 0)
  {
  }
  else if (key == 1)
  {
    // 此选项是弹出文本显示窗口
    p->set(&my_Panel_2);
    my_Panel_2.of();
    my_Panel_2.set_interlude(0, 0, 0, 0);
    p->Input_off();
  }
  else if (key == 2)
  {
    p->set(&my_Panel_slider);
    p->Input_off();
    my_Panel_slider.slider_->set("fps上限", &m->fps_max, 10, 240);
    my_Panel_slider.of();
    my_Panel_slider.set_interlude(0, 0, 0, 0);
  }
  else if (key == 3)
  {
    p->set(&my_Panel_slider);
    p->Input_off();
    my_Panel_slider.slider_->set("当前选项高度", (int *)&my_text_1_Panel.menuOptions[3].height, 10, 50);
    my_Panel_slider.of();
    my_Panel_slider.set_interlude(0, 0, 0, 0);
  }
  else if (key == 4)
  {
    // 此选项改变my_Panel_text的菜单界面的宽度和x坐标
    if (my_Panel_text.get_w() == 64)
    {
      my_Panel_text.set_w(128);
      my_Panel_text.set_x(0);
    }
    else
    {
      my_Panel_text.set_w(64);
      my_Panel_text.set_x(64);
    }
  }
  else if (key == 7)
  {
    // 此选项是弹出子级面板，设置立方体参数
    p->set(&my_Panel_1);
    my_Panel_1.set_interlude_y(0);
    my_Panel_1.of();
    p->Input_off(); // 关闭当前面板的按键输入
  }
  else if (key == 8)
  {
    // 此选项是切换当前菜单面板，切换到设置界面
    m->set(&my_Panel_4);
    my_Panel_4.set_interface_now(p); // 继承当前面板的实时坐标信息
    my_Panel_4.set_interlude_w(0);   // 重置宽度动画偏移值
    my_Panel_4.of();
  }
  else if (key == 9)
  {
    ESP.restart(); // 重启
  }
  else if (key == 10)
  {                                                      // 此选项是打开键盘
    p->set(&my_Panel_keyboard);                          // 添加了有键盘实例的面板为当前面板的子面板
    my_Panel_keyboard.keyboard->keyboard_init();         // 初始化键盘（实际上将动画偏移值重置）
    my_Panel_keyboard.of();                              // 启动
    my_Panel_keyboard.keyboard->set(myOptions1[1].name); // 设置要修改的字符串指针，（当前设置的是选项1的文本）
    p->Input_off();
  }
}

void my_ebook_callback(Axeuh_UI_Panel *p, Axeuh_UI *m) // 文本显示窗口的回调函数  在退出时触发
{
  p->Parent_Panel->Input_of(); // 打开父级面板的输入
  p->off();                    // 关闭
  p->set_interlude_y(-64);     // 设置动画向上偏移64
}

// 初始化设置 ------------------------------------------------------
void setup()
{
  Serial.begin(115200);                    // 初始化串口
  SPI.begin(OLED_CLK, OLED_MOSI, OLED_CS); // 初始化SPI
  // 按键（摇杆）引脚初始化
  pinMode(HW_Y, INPUT);
  pinMode(HW_X, INPUT);
  pinMode(HW_SW, INPUT_PULLUP);

  // UI系统初始化
  myui.begin();          // 初始化（必要！）
  myui.set(my_ui_input); // 配置输入接口

  myui.set(&cube);          // 添加立方体实例
  myui.set(&my_statusbar);  // 添加状态栏实例
  myui.set(&my_Panel_text); // 添加首级面板
  // 面板关联
  my_Panel_1.set(&my_text_2);          // 将设置立方体参数菜单实例添加到my_text_2面板
  my_Panel_2.set(&my_Ebook_1);         // 将文本窗口实例添加到my_Ebook_1面板
  my_Panel_3.set(&my_gif_4);           // 将动图添加到面板
  my_Panel_4.set(&my_text_3);          // 将设置菜单实例添加到面板
  my_Panel_text.set(&my_text_1_Panel); // 将首页的菜单实例添加到首级面板
  my_Panel_slider.set(&my_slider);     // 将滑动条实例添加到my_Panel_slider面板
  my_Panel_keyboard.set(&keyborad);    // 将键盘实例添加到面板
  // 回调函数绑定
  // 在菜单实例中，可以为每一个选项配置一个回调函数，但这不是必要的。而如果当前选项没有配置回调函数，就会触发该菜单实例总回调函数
  // 在总回调函数中，需要获取当前所选中的选项来进行判断
  // 在文本窗口中，返回会触发回调函数
  my_Panel_4.set(AllCallback_my_text3); // 设置面板所绑定的实例的回调函数
  my_Panel_1.set(AllCallback_my_text2);
  my_Panel_text.set(AllCallback_my_Popup_text1);
  my_Panel_2.set(my_ebook_callback);

  // 布局配置
  my_statusbar.set_y(-12);     // 设置状态栏y目标位置
  my_statusbar.set_y_now(-12); // 设置状态栏y实时位置

  cube.set_cube(64, 32, 15); // 初始立方体位置和大小

  my_Panel_text.text->set_menuOptions_x(6, -38); // 设置第六个选项的x偏移-38
  my_Panel_text.display_off();                   // 关闭显示
  my_Panel_text.set_lucency(1);                  // 设置背景透明

  my_Panel_1.set_interlude(0, -64, 0, 0);        // 设置动画偏移值
  my_Panel_1.set_interface_now(16, -64, 96, 46); // 设置坐标实时位置
  my_Panel_1.set_interface(16, 10, 96, 46);      // 设置坐标目标位置

  my_Panel_4.set_lucency(1); //

  my_Panel_slider.set_interface(0, 10, 110, 46, 8);
  my_Panel_slider.set_interface_now(12, -64, 96, 46);
  my_Panel_slider.set_interlude(0, -64, 0, 0);

  my_Panel_2.set_interface_now(16, -64, 96, 46);
  my_Panel_2.set_interface(16, 10, 96, 46);
  my_Panel_2.set_interlude(0, -64, 0, 0);

  my_Panel_3.set_interface_now(12, -64, 96, 46);
  my_Panel_3.set_lucency(1);

  // 启动显示任务 --------------------------------------------------

  myui.menu_display_xtaskbegin(); // 异步运行ui

  delay(2000);

  my_Panel_text.of(); // 打开首级菜单

  my_Panel_text.set_interlude(0, 0, 0, 0);

  my_statusbar.set_y(0);

  cube.set_cube(122, 6, 3.5);
}

void loop()
{
}
```

---

## 系统架构

### 组件框图

```
┌─────────────────┐
│   用户输入       │←[硬件中断]
└───────┬─────────┘
        ↓
┌─────────────────┐
│  事件处理器      │→[消息队列]
└───────┬─────────┘
        ↓
┌─────────────────┐
│  UI渲染引擎      │←[帧同步]
└───────┬─────────┘
        ↓
┌─────────────────┐
│ 显示驱动(U8G2)   │→[SPI/I2C]
└─────────────────┘
```

### 关键设计

1. **事件驱动模型**

   - 采集输入（`IN_PUT_Mode`枚举）
   - 非阻塞式事件处理（`xTaskCreatePinnedToCore`）

2. **资源管理**
   - 预编译位图资源（`gif.h`）
   - 动态内存分配策略（`expand()`函数）
   - 对象复用池

---

## 核心类说明

### Axeuh_UI (主控类)

| 方法      | 说明           |
| --------- | -------------- |
| `begin()` | 初始化 UI 系统 |
| `set()`   | 添加菜单项     |

### Axeuh_UI_Panel (面板容器)

```cpp
// 典型用法
Axeuh_UI_Panel mainPanel;
mainPanel.set(textMenu);  // 绑定文本菜单
mainPanel.set_interlude(0,0,0,0); // 设置动画参数
```

### Axeuh_UI_TextMenu (文本菜单)

```cpp
MenuOption options[] = {
  {"温度设置", 12, LEFT_CENTER, TEXT},
  {"亮度调节", 12, LEFT_CENTER, TEXT}
};
Axeuh_UI_TextMenu menu(options, 2);
```

### 特殊功能类

- `Axeuh_UI_Cube`：3D 立方体渲染
- `Axeuh_UI_Keyboard`：中文输入法
- `Axeuh_UI_slider`：参数滑动条

---

## API 参考

### 关键方法

| 类                  | 方法              | 说明           |
| ------------------- | ----------------- | -------------- |
| `Axeuh_UI`          | `set_u8g2()`      | 绑定显示驱动   |
| `Axeuh_UI_TextMenu` | `set_munber()`    | 设置当前选中项 |
| `Axeuh_UI_Panel`    | `set_interlude()` | 设置动画参数   |

### 回调函数类型

```cpp
typedef void (*textMenuCallback)(Axeuh_UI_Panel*, Axeuh_UI*);
typedef IN_PUT_Mode (*Axeuh_UI_input_callback)();
```

---

## 许可证

[Apache License v2](./LICENSE)
