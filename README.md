# Arduino 交互式 UI 系统

基于 Arduino 平台和 Axeuh_UI 库开发的综合界面系统，集成多级菜单、动态图形、3D 渲染和硬件交互功能。基于 U8G2 库实现高性能显示驱动。

GIF 或图片的转换在这里 https://javl.github.io/image2cpp/ 记得勾 swap 选项
(GIF 需要先转换成一帧一帧的图片)

![系统演示](image.png)

---

## 功能特性

### 核心功能

- **多级菜单系统**

  - 支持无限级菜单嵌套
  - 动态菜单项生成（ `MenuOption` 结构）
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

| 组件     | 规格要求          | 推荐型号     | 接口说明 |
| -------- | ----------------- | ------------ | -------- |
| 主控板   | 支持 Arduino 框架 | ESP32 DevKit | -        |
| 显示屏   | 支持 U8G2         | SSD1306      | SPI/I2C  |
| 输入设备 | 5 向导航+确认键   | 无           | ADC      |

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
      p->text->set_menuOptions_name(1, "~ 设为右上角"); // 将当前面板的菜单的选项1的名称
                                                       //设置为“设为右上角”
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

# 系统架构

## UI 结构示意

```
Axeuh_UI
├── Axeuh_UI_Panel (首级面板)
│ └── [Axeuh_UI_Panel*] (可无限递归的子面板)
├── Axeuh_UI_Cube (立方体组件)
└── Axeuh_UI_StatusBar (状态栏)

Axeuh_UI_Panel
├── [Axeuh_UI_Panel*] (子面板)
├── Axeuh_UI_TextMenu (文本浏览窗口)
├── Axeuh_UI_Ebook (菜单)
├── Axeuh_UI_slider (滑动条)
├── Axeuh_UI_Keyboard (拼音键盘)
└── Menu_gif (图片)
```

---

# 核心类说明

## Axeuh_UI（框架）

### 初始化：

同时会初始化 u8g2，启用 UTF8 模式，设置字体 `u8g2_font_wqy12_t_gb2312` ，初始化滤波器。

```cpp
begin();
```

### 开始运行 UI：

异步运行。有两个可以传递的参数：

- `Memory_size`(`unsigned int`)分配内存大小，默认为 8192
- `xCoreID`(`int`)分配的内核，默认为 0

```cpp
menu_display_xtaskbegin(Memory_size,xCoreID);
```

### 添加实例：

`Axeuh_UI`是所有实例类的父类(`public`)，除了一些表单类之外。因此，所有实例都会受`Axeuh_UI`参数的改变而影响。以下是`Axeuh_UI`可以承载的实例：

- `Axeuh_UI_Panel` 面板
- `Axeuh_UI_Cube` 立方体
- `Axeuh_UI_StatusBar` 状态栏

```cpp
Axeuh_UI myui();
Axeuh_UI_Cube cube;
void setup()
{
  myui.begin();
  myui.set(&cube);//添加立方体
}
```

### 配置输入回调函数

`set`(&`你的回调函数`);

在每一帧都会运行这个函数。这是操作 UI 必要的函数。

```cpp
Axeuh_UI_TextMenu mymenu;
void my_Callback(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
}
void setup()
{
  mymenu.set(my_Callback);
}
```

回调函数类型

```cpp
typedef IN_PUT_Mode (*Axeuh_UI_input_callback)();
```

### 添加`u8g2`

作为基于 u8g2 的 UI 库，所以`Axeuh_UI`需要获得 u8g2 实例的指针，才能完成绘制工作，下面是添加 u8g2 指针的方法：

方法一

```cpp
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_Reset);
Axeuh_UI myui(&u8g2);//直接传参
```

方法二

```cpp
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_Reset);
void setup()
{
  myui.begin();
  myui.set_u8g2(&u8g2);//添加u8g2实例指针
}
```

### 设置当前输入状态

有效值为：`UP` `DOWN` `LEFT` `RIGHT` `SELECT` `STOP`

```cpp
set_IN_now(IN_PUT_Mode);
```

### 设置 FPS 上限

类型为`uint16_t`，默认为 200hz

```cpp
set_fps_max(120);
```

### 获取当前输入状态

返回值为`IN_PUT_Mode`：`UP` `DOWN` `LEFT` `RIGHT` `SELECT` `STOP`

```cpp
get_IN_now();
```

### 获取帧率上限

返回为`uint16_t`

```cpp
get_fps_max();
```

### 获取当前帧率

返回为`float`

```cpp
get_fps_max();
```

---

## Axeuh_UI_Panel（面板）

### 添加实例

**`Axeuh_UI_Panel`是一个面板，需要承载一个实例，而实例则继承面板的参数，以下是可以承载的实例：**

- `Axeuh_UI_TextMenu` 菜单
- `Axeuh_UI_Ebook` 文本浏览窗口
- `Axeuh_UI_slider` 滑动条
- `Axeuh_UI_Keyboard` 拼音键盘
- `Menu_gif` 图片
- `Axeuh_UI_Panel` 面板

你只需要`set`(&`你要添加的实例`);就行。

例如：

```cpp
static Axeuh_UI_Panel my_Panel;
static Axeuh_UI_Ebook my_Ebook("hello，这里是Axeuh");

my_Panel.set(&my_menu);
```

**同时你也可以直接使用构造函数承载实例。**

```cpp
static Axeuh_UI_Ebook my_Ebook("hello，这里是Axeuh");
static Axeuh_UI_Panel my_Panel(&my_Ebook);
```

**添加子面板也同样是`set`(&`你要添加的面板`);**

```cpp
static Axeuh_UI_Panel my_Panel1;
static Axeuh_UI_Panel my_Panel2;
my_Panel2.set(&my_Panel1);
```

或者

```cpp
static Axeuh_UI_Panel my_Panel1;
static Axeuh_UI_Panel my_Panel2(&my_Panel1);
```

### 添加回调函数

你只需要`set`(&`你的回调函数`);

该函数会把回调函数传递给当前面板的实例，注意要先绑定实例再传递回调函数。

```cpp
Axeuh_UI_Ebook my_Ebook("hello，这里是Axeuh");
Axeuh_UI_Panel my_Panel;
void my_Callback(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
}
void setup()
{
  my_Panel2.set(&my_Ebook);
  my_Panel.set(my_Callback);
}
```

### 打开与关闭

需要注意的是关闭显示开关，只有当动画结束，才会不显示该面板。显示开关和输入开关默认为开。

```cpp
of();
off();
```

### 显示打开与关闭

```cpp
display_of();
display_off();
```

### 输入打开与关闭

```cpp
Input_of();
Input_off();
```

### 设置背景是否透明

`set_lucency`(bool)，默认 0。

- `0`：不透明，面板会完全盖在父级面板上
- `1`：透明，面板会和屏幕上其他内容重合

```cpp
set_lucency(1);
```

### 设置面板位置和大小

`x`为 x 轴，`y`为 y 轴，`w`为宽，`h`为高，`r`为圆角大小，圆角大小仅`Axeuh_UI_slider`生效。都是`int16_t`类型。

- `set_x`(`x`);
- `set_y`(`y`);
- `set_h`(`h`);
- `set_w`(`w`);
- `set_r`(`r`);
- `set_interface`(`x`,`y`,`w`,`h`,`r`);
- `set_interface`(`x`,`y`,`w`,`h`);

### 设置面板动画目前位置和大小

`x_now`为 x 轴，`y_now`为 y 轴，`w_now`为宽，`h_now`为高，`r_now`为圆角大小，圆角大小仅`Axeuh_UI_slider`生效。都是`int16_t`类型。

- `set_interface_now_x`(`x_now`);
- `set_interface_now_y`(`y_now`);
- `set_interface_now_h`(`h_now`);
- `set_interface_now_w`(`w_now`);
- `set_interface_now_r`(`r_now`);
- `set_interface_now`(`x_now`,`y_now`,`w_now`,`h_now`,`r_now`);
- `set_interface_now`(`x_now`,`y_now`,`w_now`,`h_now`);

### 设置面板动画偏移值

`x`为 x 轴，`y`为 y 轴，`w`为宽，`h`为高，`r`为圆角大小，圆角大小仅`Axeuh_UI_slider`生效。都是`int16_t`类型。

要想实现动画修改这个数值就可以了，当然你直接改页面参数也是有动画效果的。但是不会保留这个面板本来的位置和大小。

- `set_interlude_x`(`x`);
- `set_interlude_y`(`y`);
- `set_interlude_h`(`h`);
- `set_interlude_w`(`w`);
- `set_interlude_r`(`r`);
- `set_interlude`(`x`,`y`,`w`,`h`,`r`);
- `set_interlude`(`x`,`y`,`w`,`h`);

### 获取面板位置和大小

```cpp
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
```

### 获取当前文本菜单聚焦选项

返回值为`int16_t`

```cpp
int key = get_textmenu_num_now();
```

---

## Axeuh_UI_TextMenu (文本菜单)

### 添加菜单内容

```cpp
MenuOption options[] = {
  {"温度设置", 12, LEFT_CENTER, TEXT},
  {"亮度调节", 12, LEFT_CENTER, TEXT}
};
Axeuh_UI_TextMenu menu(options, 2);
```

或

```cpp
MenuOption options[] = {
  {"温度设置", 12, LEFT_CENTER, TEXT},
  {"亮度调节", 12, LEFT_CENTER, TEXT}
};
Axeuh_UI_TextMenu menu;
menu.set(options, 2);
```

### 修改当前所聚焦的选项

会直接跳转到指定选项

```cpp
set_munber(2);
```

### 修改选项文本

`set_menuOptions_name`(`n`，`name`);

- `n`为选项索引
- `name`为要设置的文本

```cpp
set_menuOptions_name(2,"hello");
```

### 设置选项偏移

- `n`为选项索引
- `num`为要设置的数值

设置 x

```cpp
set_menuOptions_x(2,10);
```

设置 y

```cpp
set_menuOptions_y(2,10);
```

### 设置回调函数

`set`(&`你的回调函数`);

`Axeuh_UI_TextMenu`的回调函数在选中任意选项时执行。如果选中的选项有回调函数，则不会触发`Axeuh_UI_TextMenu`的回调函数，而是触发选项的回调函数。

与`Axeuh_UI_Panel`中设置回调函数功能一致。

```cpp
Axeuh_UI_TextMenu mymenu;
void my_Callback(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
}
void setup()
{
  mymenu.set(my_Callback);
}
```

回调函数类型

```cpp
typedef void (*textMenuCallback)(Axeuh_UI_Panel*, Axeuh_UI*);
```

---

## MenuOption（菜单表单类）

### 初始化列表

- `String` `n` = "" 选项名称
- `uint8_t` `h` = 12 选项高度，默认 12
- `alignMode` `a` = `LEFT_CENTER`/`LEFT_CORNER`/`ALIGN_CENTER` 对齐方式，默认`LEFT_CENTER`（左中对齐）
- `OptionMode` `m` = `TEXT`/`TEXT_MORE`/`PICTURE`/`PICTURE_TEXT` 选项模式，默认`TEXT`
- `Menu_gif` `*g` = `nullptr` 图片指针，默认`nullptr`
- `OptionistriggersAnimation` `tr` = `Trigger`/`No_Trigger` 是否触发动画（菜单跳转动画），默认`No_Trigger`（不触发动画）
- `textMenuCallback` `cb` = `nullptr` 选项回调函数，默认`nullptr`
- `OptionisSelectable` `is` = `Focused`/`No_Focusing` 选项是否可被选中，默认`Focused`（可被选中）

`MenuOption` myOptions={`n`,`h`,`a`,`m`,`*g`,`tr`,`cb`,`is`};

```cpp
MenuOption myOptions[] = // 菜单信息
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

Axeuh_UI_TextMenu my_menu(myOptions, sizeof(myOptions) / sizeof(myOptions[0]));
```

需要注意的是，菜单表单的选项文本在元数据中并不是`String`类型，而是`MenuOption::AutolenString`类型。

使用上与`String`没有太大差异，但是在`MenuOption::AutolenString`类型下修改字符串会自动计算并保存选项宽度。

如果将`MenuOption::AutolenString`类型传递或引用成`String`类型，修改`String`类型的字符串虽然菜单文本会发生改变，但不会自动更新选项的宽度，这会导致选项宽度和文本长度不匹配。

如果想更新选项宽度，可手动运行`Axeuh_UI_TextMenu`中的`init_text_more()`函数计算所有选项的宽度并保存。

如果遇到本应该能够使用的`String`的函数未定义，可手动添加并实现`MenuOption::AutolenString`的`String`的函数

为什么会存在`MenuOption::AutolenString`？是因为在绘制 UI 中，实时计算选项长度会降低屏幕刷新速度。

---

## Menu_gif（图片）

### 初始化

`const unsigned char` `**frames` 图片或 GIF 指针  
`uint16_t` `frameCount_` 总帧数  
`uint16_t` `x` x 坐标  
`uint16_t` `y` y 坐标  
`uint16_t` `w` 图片宽度  
`uint16_t` `h` 图片高度  
`uint8_t` `speed` 动画速度，默认为 30  
`uint16_t` `startFrame` 开始帧，默认为 0  
`OptiongifMode1` `aPlay` 是否自动播放，默认为 AutoPlay  
`OptiongifMode2` `hPlay` 是否聚焦时滑入界面，默认为 Show

`Menu_gif` `my_gif` = {`**frames`, `frameCount_`, `x`, `y`, `w`, `h`, `speed`, `startFrame`, `aPlay`, `hPlay`};

```cpp
Menu_gif my_gif_1 = {icons_Homer_SimpsonallArray, 1, 5, 2, 50, 50, 30, 0, AutoPlay, Hide};

Menu_gif my_gif_2 = {icons_Homer_SimpsonallArray2, 1, 5, 2, 50, 50};
```

---

## Axeuh_UI_slider（滑动条）

### 构造及设置

- `String` `name` 标题
- `float` `*num` 修改值指针
- `int16_t` `min` 最小值
- `int16_t` `max` 最大值
- `float` `unit_` 刻度单位，默认为 1

`set`(`name`,&`*num`,`min`,`max`);  
`set`(`name`,&`*num`,`min`,`max`,`unit_`);

示例：

```cpp
int a=0;
static Axeuh_UI_slider my_slider("设置a", &a, 0, 50, 0.5);
```

或者

```cpp
int a=0;
static Axeuh_UI_slider my_slider;
void setup()
{
  my_slider.set("设置a", &a, 0, 50, 0.5);
}
```

### 你需要知道的一些事

滑动条随面板高度 `h_now` 变化会有不同的形态  
一共三种

---

## Axeuh_UI_Cube（立方体）

### 设置位置和大小

- `int16_t` `cube_x` x 轴坐标
- `int16_t` `cube_y` y 轴坐标
- `float` `cube_scale` 立方体大小

`set_cube`( `cube_x`, `cube_y`);  
`set_cube`( `cube_x`, `cube_y`, `cube_scale`);  
`set_scale`( `cube_scale`);设置大小

`set_cube_now`( `cube_x`, `cube_y`, `cube_scale`);设置实时坐标位置和大小

```cpp
static Axeuh_UI_Cube cube;

cube.set_cube(10,10);
cube.set_cube(10,10,15);
cube.set_scale(15);

cube.set_cube_now(10,10,15);
```

### 设置角度

- `float` `angleX` x 轴角度
- `float` `angleY` y 轴角度
- `float` `angleZ` z 轴角度

`set_cube_rotate_speed_x`(`angleX`);
`set_cube_rotate_speed_y`(`angleY`);
`set_cube_rotate_speed_z`(`angleZ`);
`set_cube_rotate_speed`(`angleX`,`angleY`,`angleZ`);

```cpp
static Axeuh_UI_Cube cube;

cube.set_cube_rotate_x(1);
cube.set_cube_rotate_y(2);
cube.set_cube_rotate_z(3);

cube.set_cube_rotate(1,2,3);
```

### 设置角速度

- `float` `angleX_speed` x 轴角速度
- `float` `angleY_speed` y 轴角速度
- `float` `angleZ_speed` z 轴角速度

`set_cube_rotate_speed_x`(`angleX_speed`);
`set_cube_rotate_speed_y`(`angleY_speed`);
`set_cube_rotate_speed_z`(`angleZ_speed`);
`set_cube_rotate_speed`(`angleX_speed`,`angleY_speed`,`angleZ_speed`);

```cpp
static Axeuh_UI_Cube cube;

cube.set_cube_rotate_speed_x(0.01);
cube.set_cube_rotate_speed_y(0.02);
cube.set_cube_rotate_speed_z(0.03);

cube.set_cube_rotate_speed(0.01,0.02,0.03);
```

### 获取参数

`get_cube_x()` 返回类型是`int16_t`  
`get_cube_y()` 返回类型是`int16_t`  
`get_angleX()` 返回类型是`float`  
`get_angleY()` 返回类型是`float`  
`get_angleZ()` 返回类型是`float`  
`get_angleX_speed()` 返回类型是`float`  
`get_angleY_speed()` 返回类型是`float`  
`get_angleZ_speed()` 返回类型是`float`  
`get_scale()` 返回类型是`float`

```cpp
static Axeuh_UI_Cube cube;

cube.get_cube_x()
cube.get_cube_y()
cube.get_angleX()
cube.get_angleY()
cube.get_angleZ()
cube.get_angleX_speed()
cube.get_angleY_speed()
cube.get_angleZ_speed()
cube.get_scale()
```

## Axeuh_UI_StatusBar（状态栏）

### 设置参数

```cpp
static Axeuh_UI_StatusBar bar;
bar.set_y(10);
bar.set_y_now(10);
```

状态栏随便写的，可根据自己需求改库文件，后续会添加自定义回调函数

## Axeuh_UI_Ebook（文本查看框）

### 构建

`String` `s` 文本内容  
`int16_t` `x` x 坐标，默认为 0  
`int16_t` `y` y 坐标，默认为 0  
`alignMode` `a` 对齐方式，默认为 LEFT_CENTER（文本查看框暂不支持对齐方式）

`Axeuh_UI_Ebook`(`s`,`x`,`y`,`a`);  
`set`(`s`,`x`,`y`,`a`);

```cpp
static Axeuh_UI_Ebook Ebook;
Ebook.set("你好，这里是Axeuh");
Ebook.set("你好，这里是Axeuh",0,0,LEFT_CENTER);

static Axeuh_UI_Ebook Ebook1("你好，这里是Axeuh");
static Axeuh_UI_Ebook Ebook2("你好，这里是Axeuh",0,0,LEFT_CENTER);
```

### 设置回调函数

`set`(&`你的回调函数`);

退出文本查看框时，如果设置了回调函数，则执行回调函数，没设置回调函数，则自动将父级面板的实例的输入开关打开，并关闭`Axeuh_UI_Ebook`的显示开关和输入开关，并将面板 y 动画偏移值`interlude_y` 设置成`-64`

```cpp
Axeuh_UI_Ebook Ebook;
void my_Callback(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
}
void setup()
{
  Ebook.set(my_Callback);
}
```

回调函数类型

```cpp
typedef void (*MenuCallback_Ebook)(Axeuh_UI_Panel *p, Axeuh_UI *m);
```

## Axeuh_UI_Keyboard（拼音键盘）

### 构建

`MenuOption::AutolenString` `*Aoutput` 菜单选项文本  
`String` `*output` 文本

`set`(`Aoutput`);  
`set`(&`output`);

`Axeuh_UI_Keyboard`(`Aoutput`);  
`Axeuh_UI_Keyboard`(&`output`);

```cpp
MenuOption myOption ={"hello",12};
String acc="hello";
Axeuh_UI_Keyboard mykeyboard;
void setup()
{
  mykeyboard.set(myOption.name);
  mykeyboard.set(&acc);
  static Axeuh_UI_Keyboard mykeyboard1(myOption.name);
  static Axeuh_UI_Keyboard mykeyboard2(&acc);
}
```

### 重置动画

`keyboard_init()`在进入拼音键盘前必须调用的函数，重置动画参数。

---

# 回调函数说明

有两种回调函数类型

```cpp
typedef void (*MenuCallback_Ebook)(Axeuh_UI_Panel *p, Axeuh_UI *m);
typedef IN_PUT_Mode (*Axeuh_UI_input_callback)();
```

`typedef IN_PUT_Mode (*Axeuh_UI_input_callback)();`  
这个回调函数用于 UI 的输入处理，函数需要返回`UP` `DOWN` `LEFT` `RIGHT` `SELECT` `STOP`。  
判断逻辑由自己实现，在无输入下默认返回`STOP`。

例如：

```cpp
// 输入处理函数 ----------------------------------------------------
IN_PUT_Mode my_ui_input()
{
  if (!digitalRead(HW_SW))
    return SELECT; // 选中
  else if (analogRead(HW_Y) >= 3995)
    return DOWN; // 向下
  else if (analogRead(HW_Y) <= 100)
    return UP; // 向上
  else if (analogRead(HW_X) >= 3995)
    return LEFT; // 向左
  else if (analogRead(HW_X) <= 100)
    return RIGHT; // 向右
  return STOP;    // 无输入
}
```

`typedef void (*MenuCallback_Ebook)(Axeuh_UI_Panel *p, Axeuh_UI *m);`  
这个函数是实现界面的响应所要执行的操作。例如菜单选中选项会执行该回调函数，我们通过判断当前的选项而执行相应的选项操作：切换菜单、进入子菜单、或者执行其他操作

其中`Axeuh_UI_Panel *p`是当前面板，使用`*p`可以直接操作和修改当前的面板，例如打开或关闭，或者添加子面板，或者替换实例。

其中`Axeuh_UI *m`是整个 UI 框架类指针，可以修改整个框架的 UI 参数，例如 fps 上限。

例如：

```cpp
void AllCallback_my_Popup_text1(Axeuh_UI_Panel *p, Axeuh_UI *m)
{
  int key = 0;
  key = p->get_textmenu_num_now();//获取菜单当前选中的选项

  if (key == 0)
  {
  }
  else if (key == 1)
  {
    // 此选项是弹出文本显示窗口
    p->set(&my_Panel_2);//添加子面板
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
```

# 许可证

[Apache License v2](./LICENSE)
