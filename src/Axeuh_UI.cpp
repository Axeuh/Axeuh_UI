#include "Axeuh_UI.h"
#include "esp_task_wdt.h"

CharLenMap Axeuh_UI::len_c;
SimpleKalmanFilter Axeuh_UI::fps_filter;

void Axeuh_UI_Ebook::drawEbook(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m)
{
    if (*if_display || !get_animation_all_isok())
    {
        xSemaphoreTake(xMutex, 100);
        if (!*lucency)
        {
            D->setDrawColor(0);
            D->drawBox(*x_now, *y_now, *w_now, *h_now);
        }
        int16_t interface_x_ = *interlude_x + *x; // 页面当前位置
        int16_t interface_y_ = *interlude_y + *y;
        int16_t interface_w_ = *interlude_w + *w;
        int16_t interface_h_ = *interlude_h + *h;

        D->setDrawColor(1);

        int clip_x = *x_now + 1;
        int clip_y = *y_now + 1;
        int clip_w = *w_now - 1;
        int clip_h = *h_now - 1;

        if (clip_x < 0)
            clip_x = 0;
        if (clip_y < 0)
            clip_y = 0;
        if (*x_now + clip_w < 0)
            clip_w = -*x_now;
        if (*y_now + clip_h < 0)
            clip_h = -*y_now;

        D->setClipWindow(clip_x, clip_y, *x_now + clip_w, *y_now + clip_h);
        int x_ = 0, y_ = 0;
        if (mode == TEXT)
        {
        }
        else if (mode == TEXT_MORE)
        {
            if (s != "")
            {
                int align_x = 0, align_y = 0;
                // if (align == LEFT_CENTER)
                // {
                //     align_x = x_now + 2;
                //     align_y = y_now + (h_now - font_offset_y) / 2;
                // }
                // else
                // {
                //     align_x = x_now + s_x;
                //     align_y = y_now + s_y;
                // }
                // D->drawUTF8(s_x + align_x, s_y + align_y + font_offset_y, s.c_str());

                int name_leng = s.length();
                if (s_len == 0)
                {
                    init_text_more();
                }
                for (int j = 0; j < name_leng; j++)
                {
                    char currentChar = s.charAt(j);
                    switch (align)
                    {
                    case LEFT_CORNER:
                        break;
                    case ALIGN_CENTER:
                    case LEFT_CENTER:
                        uint8_t string_text_line = s_len / *w_now + 1;
                        align_y = (*h_now - string_text_line * font_height) / 2;
                        if (string_text_line > *h_now / font_height)
                            align_y = 0;
                        break;
                    }

                    int16_t drawtext_y = font_offset_y + text_height_now + align_y + *y_now + 2;

                    if (currentChar == '\r' || currentChar == '\t' || currentChar == '\n')
                    {
                        y_ += font_height;
                        x_ = 0;
                    }
                    else if (isChineseChar(currentChar))
                    {
                        if (page_y_now + drawtext_y + y_ > *y_now - 12 && page_y_now + drawtext_y + y_ < *y_now + *h_now + 12)
                            D->drawUTF8(page_x_now + x_ + *x_now + 2,
                                        page_y_now + drawtext_y + y_,
                                        (s.substring(j, j + 3)).c_str());
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
                        if (page_y_now + drawtext_y + y_ > *y_now - 12 && page_y_now + drawtext_y + y_ < *y_now + *h_now + 12)
                            D->drawUTF8(page_x_now + x_ + *x_now + 2,
                                        page_y_now + drawtext_y + y_,
                                        (s.substring(j, j + 1)).c_str());

                        if (x_ + sub > *w_now - sub - 6)
                        {
                            y_ += font_height;
                            x_ = 0;
                        }
                        else
                            x_ += sub;
                    }
                    if (y_ > *h_now - page_y_now)
                    {
                        // break;
                    }
                }

                D->setDrawColor(0);
                D->drawBox(progress_bar_black_x_now + *x_now + *w_now - 7,
                           *y_now + 1,
                           6,
                           *h_now - 2);
                D->setDrawColor(1);
                int line_1 = map(-page_y_now, 0, y_, 0, *h_now - 4);
                int line_2 = map(-page_y_now + *h_now, 0, y_, 0, *h_now - 4) + 1;

                // Serial.println((String)(y_now + line_1 + 3) + "  " + (String)(y_now + h_now + line_2 - 4));

                D->drawRFrame(progress_bar_x2_now + *x_now + *w_now - 7,
                              *y_now + 1,
                              6,
                              *h_now - 2,
                              2);

                if (progress_bar_isok)
                {
                    int box_y = *y_now + line_2 - (*y_now + line_1 + 3);
                    if (box_y < 1)
                        box_y = 1;

                    D->drawBox(progress_bar_x1_now + *x_now + *w_now - 5,
                               *y_now + line_1 + 3,
                               2,
                               box_y);
                }
                else
                {
                    int box_y = *y_now + line_2 - (*y_now + line_1 + 3) + 4;
                    if (box_y < 1)
                        box_y = 1;
                    D->drawBox(progress_bar_x1_now + *x_now + *w_now - 5,
                               *y_now + line_1 + 1,
                               2,
                               *y_now + line_2 - (*y_now + line_1 + 3) + 4);
                }
            }

            D->setMaxClipWindow();
            D->drawFrame(*x_now, *y_now, *w_now, *h_now);
        }

        if ((int16_t)page_x_now == (int16_t)page_x &&
            (int16_t)page_y_now == (int16_t)page_y)
        {
            progress_bar_x1 = 2;
            progress_bar_x2 = 6;
            progress_bar_black_x = 2;
            progress_bar_isok = 0;
        }
        else
        {
            progress_bar_x1 = 0;
            progress_bar_x2 = 0;
            progress_bar_black_x = 0;
            progress_bar_isok = 1;
        }
        // Serial.print((String)page_y + (String) "\t");
        // Serial.print((String)h + (String) "\t");
        // Serial.print((String)y_ + (String) "\t");

        // Serial.println();
        if (*if_Input)
        {
            if (IN == UP)
            {
                if (page_y + 12 <= 0)
                    page_y += 12;
                else
                    page_y = 0;
            }
            else if (IN == DOWN)
            {
                if (page_y - interface_h_ > -y_)
                    page_y -= 12;
                else
                    page_y = -y_ + interface_h_ - 3;
            }
            else if (IN == SELECT)
            {
                handleInput = 1;
            }
            else
            {
                if (handleInput)
                {
                    handleInput = 0;

                    xSemaphoreGive(xMutex);
                    staticCallback(this, P, m);
                    xSemaphoreTake(xMutex, 100);
                }
            }
        }
        animation(x_now, interface_x_, m->fps / 3 * 5);
        animation(y_now, interface_y_, m->fps / 3 * 5);
        animation(w_now, interface_w_, m->fps / 3 * 5);
        animation(h_now, interface_h_, m->fps / 3 * 5);
        animation(&page_x_now, page_x, m->fps / 3 * 5);
        animation(&page_y_now, page_y, m->fps / 3 * 5);

        animation(&progress_bar_x1_now, progress_bar_x1, m->fps / 3 * 2);
        animation(&progress_bar_x2_now, progress_bar_x2, m->fps / 3 * 2);
        animation(&progress_bar_black_x_now, progress_bar_black_x, m->fps / 3 * 2);
        xSemaphoreGive(xMutex);
    }
}

void Axeuh_UI_Panel::drawPanel(U8G2 *D, Axeuh_UI *m, IN_PUT_Mode IN)
{
    if (if_display || !get_animation_all_isok())
    {
        xSemaphoreTake(xMutex, 100);
        unsigned long currentMillis = millis();
        int text_height_now = 0;

        // Serial.print((String)x_now+"\t");
        // Serial.print((String)y_now+"\t");
        // Serial.print((String)w_now+"\t");
        // Serial.print((int)this);
        // Serial.println();

        if (gif != nullptr)
        {
            int16_t interface_x_ = interlude_x + x; // 页面当前位置
            int16_t interface_y_ = interlude_y + y;
            int16_t interface_w_ = interlude_w + w;
            int16_t interface_h_ = interlude_h + h;
            if (!lucency)
            {
                D->setDrawColor(0);
                D->drawRBox(x_now, y_now, w_now, h_now, r_now);
            }
            D->setDrawColor(1);
            if (y_now + gif->y + gif->y_now + text_height_now > y_now - gif->h &&
                y_now + gif->y + gif->y_now + text_height_now < y_now + h_now + gif->h)
            {
                if (currentMillis - gif->previousMillis >= gif->frameCount_speed)
                {
                    int frameCount_add = (currentMillis - gif->previousMillis) / gif->frameCount_speed;
                    int gif_newframeCount = (gif->frameCount_now + frameCount_add) % gif->frameCount;
                    gif->frameCount_now = gif_newframeCount;
                    gif->previousMillis = currentMillis;
                }
                D->setBitmapMode(1);
                D->drawXBMP(x_now + gif->x + gif->x_now,
                            y_now + gif->y + gif->y_now + text_height_now,
                            gif->w,
                            gif->h,
                            gif->jpg[gif->frameCount_now]);
                D->setBitmapMode(0);
            }
            else
            {
                gif->previousMillis = currentMillis;
            }
            if (if_Input)
            {
                if (IN == SELECT)
                {
                    handleInput = 1;
                    interlude_y = -64;
                }
                else
                {
                    if (handleInput)
                    {
                        handleInput = 0;
                        if_display = 0;
                        if_Input = 0;
                        *return_num_pointer = 1;
                        // y = -64;
                    }
                }
            }
            animation(&x_now, interface_x_, m->fps / 3 * 5);
            animation(&y_now, interface_y_, m->fps / 3 * 5);
            animation(&w_now, interface_w_, m->fps / 3 * 5);
            animation(&h_now, interface_h_, m->fps / 3 * 5);
            animation(&r_now, r, m->fps / 3 * 5);
        }
        else if (text != nullptr)
            text->draw_textmenu(D, IN, this, m);
        else if (slider_ != nullptr)
            slider_->drawSlider(D, IN, this, m);
        else if (Ebook != nullptr)
            Ebook->drawEbook(D, IN, this, m);
#ifdef CHINESE_KEYBOARD
        else if (keyboard != nullptr)
            keyboard->drawKeyboard(D, IN, this, m);
#endif

        if (Panel_ != nullptr)
        {
            xSemaphoreGive(xMutex);
            Panel_->Parent_Panel = this;
            Panel_->drawPanel(D, m, IN);
        }
        xSemaphoreGive(xMutex);
    }
}
void Axeuh_UI_TextMenu::draw_MenuOption(U8G2 *D, unsigned long currentMillis, int &text_height_now)
{
    for (uint8_t i = 0; i < menuOptions_index; i++)
    {
        // 获取当前选项
        MenuOption &opt = menuOptions[i];

        int align_x = 0, align_y = 0;

        if (opt.mode == TEXT)
        {
            switch (opt.align)
            {
            case LEFT_CORNER:
                break;
            case LEFT_CENTER:
                align_y = (opt.height - font_height) / 2;
                break;
            case ALIGN_CENTER:
                align_x = (opt.height - font_height) / 2;
                align_y = align_x;
                break;
            default:
                break;
            }
            int16_t drawtext_y = font_offset_y + text_height_now + align_y + *y_now + 2;
            if (interface_text_y_now + drawtext_y + opt.y > *y_now &&
                interface_text_y_now + drawtext_y + opt.y < *y_now + *h_now + 12)
                D->drawUTF8(interface_text_x_now + opt.x + *x_now + align_x + 2,
                            interface_text_y_now + drawtext_y + opt.y,
                            opt.name.c_str());
        }
        else if (opt.mode == TEXT_MORE)
        {
            int name_leng = opt.name.length();
            if (opt.all_len == 0)
                init_text_more();

            if (opt.all_len > *w_now)
            {
                int x_ = 0, y_ = 0;
                int k_ = 0;
                int16_t drawtext_y = font_offset_y + text_height_now + align_y + *y_now + 2;

                for (int j = 0; j < name_leng; j++)
                {
                    char currentChar = opt.name.charAt(j);
                    switch (opt.align)
                    {
                    case LEFT_CORNER:
                        break;
                    case ALIGN_CENTER:
                    case LEFT_CENTER:
                        uint8_t string_text_line = len_c.Get(opt.name.c_str()) / *w_now;
                        align_y = (opt.height - string_text_line * font_height) / 2;
                        if (string_text_line > opt.height / font_height)
                            align_y = 0;
                        break;
                    }

                    if (currentChar == '\r' || currentChar == '\t' || currentChar == '\n')
                    {
                        y_ += font_height;
                        x_ = 0;
                    }
                    else if (isChineseChar(currentChar))
                    {
                        if (interface_text_y_now + drawtext_y + y_ > *y_now &&
                            interface_text_y_now + drawtext_y + y_ < *y_now + *h_now + 12)
                            D->drawUTF8(interface_text_x_now + x_ + *x_now + 2,
                                        interface_text_y_now + drawtext_y + y_,
                                        (opt.name.substring(j, j + 3)).c_str());
                        if (x_ + font_height > *w_now - font_height)
                        {
                            y_ += font_height;
                            x_ = 0;
                        }
                        else
                        {
                            x_ += len_c.Get((opt.name.substring(j, j + 3)).c_str());
                        }

                        j += 2;
                    }
                    else
                    {
                        int sub = len_c.Get((opt.name.substring(j, j + 1)).c_str());
                        if (interface_text_y_now + drawtext_y + y_ > *y_now &&
                            interface_text_y_now + drawtext_y + y_ < *y_now + *h_now + 12)
                            D->drawUTF8(interface_text_x_now + x_ + *x_now + 2,
                                        interface_text_y_now + drawtext_y + y_,
                                        (opt.name.substring(j, j + 1)).c_str());
                        if (x_ + sub > *w_now - sub - 5)
                        {
                            y_ += font_height;
                            x_ = 0;
                        }
                        else
                            x_ += sub;
                    }
                    if (y_ > opt.height - font_height)
                        break;
                }
            }
            else
            {
                switch (opt.align)
                {
                case LEFT_CORNER:
                    break;
                case LEFT_CENTER:
                    align_y = (opt.height - font_height) / 2;
                    break;
                case ALIGN_CENTER:
                    align_x = (opt.height - font_height) / 2;
                    align_y = align_x;
                    break;
                default:
                    break;
                }
                int16_t drawtext_y = font_offset_y + text_height_now + align_y + *y_now + 2;
                if (interface_text_y_now + drawtext_y + opt.y > *y_now &&
                    interface_text_y_now + drawtext_y + opt.y < *y_now + *h_now + 12)
                    D->drawUTF8(interface_text_x_now + opt.x + *x_now + align_x + 2,
                                interface_text_y_now + drawtext_y + opt.y,
                                opt.name.c_str());
            }
        }
        else if (opt.mode == PICTURE)
        {
            if (interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now > *y_now - opt.gif->h &&
                interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now < *y_now + *h_now + opt.gif->h)
            {
                if (opt.gif->Play_hide == Hide)
                {
                    if (i != meun_number_now)
                        opt.gif->x_ = -opt.gif->w - opt.gif->x;
                    else
                        opt.gif->x_ = 0;
                }
                if (currentMillis - opt.gif->previousMillis >= opt.gif->frameCount_speed)
                {
                    int8_t frameCount_add = (currentMillis - opt.gif->previousMillis) / opt.gif->frameCount_speed;
                    int8_t gif_newframeCount = (opt.gif->frameCount_now + frameCount_add) % opt.gif->frameCount;
                    if (opt.gif->Autoplay == ManualPlay)
                    {
                        if (meun_number_now == i ||
                            opt.gif->frameCount_now != opt.gif->frameCount_start)
                            opt.gif->frameCount_now = gif_newframeCount;
                    }
                    else
                    {
                        opt.gif->frameCount_now = gif_newframeCount;
                    }
                    opt.gif->previousMillis = currentMillis;
                }
                D->setBitmapMode(1);
                D->drawXBMP(interface_text_x_now + *x_now + opt.gif->x + opt.gif->x_now,
                            interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now,
                            opt.gif->w,
                            opt.gif->h,
                            opt.gif->jpg[opt.gif->frameCount_now]);
                D->setBitmapMode(0);
            }
            else
            {
                opt.gif->previousMillis = currentMillis;
            }
        }
        else if (opt.mode == PICTURE_TEXT)
        {
            if (interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now > *y_now - opt.gif->h &&
                interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now < *y_now + *h_now + opt.gif->h)
            {
                if (opt.gif->Play_hide == Hide)
                {
                    if (i != meun_number_now)
                        opt.gif->x_ = -opt.gif->w - opt.gif->x;
                    else
                        opt.gif->x_ = 0;
                }
                if (currentMillis - opt.gif->previousMillis >= opt.gif->frameCount_speed)
                {
                    int frameCount_add = (currentMillis - opt.gif->previousMillis) / opt.gif->frameCount_speed;
                    int gif_newframeCount = (opt.gif->frameCount_now + frameCount_add) % opt.gif->frameCount;
                    if (opt.gif->Autoplay == ManualPlay)
                    {
                        if (meun_number_now == i ||
                            opt.gif->frameCount_now != opt.gif->frameCount_start)
                            opt.gif->frameCount_now = gif_newframeCount;
                    }
                    else
                    {
                        opt.gif->frameCount_now = gif_newframeCount;
                    }
                    opt.gif->previousMillis = currentMillis;
                }
                D->setBitmapMode(1);
                D->drawXBMP(interface_text_x_now + *x_now + opt.gif->x + opt.gif->x_now,
                            interface_text_y_now + *y_now + opt.gif->y + opt.gif->y_now + text_height_now,
                            opt.gif->w,
                            opt.gif->h,
                            opt.gif->jpg[opt.gif->frameCount_now]);
                D->setBitmapMode(0);
            }
            else
            {
                opt.gif->previousMillis = currentMillis;
            }

            switch (opt.align)
            {
            case LEFT_CORNER:
                break;
            case LEFT_CENTER:
                align_y = (opt.height - font_height) / 2;
                break;
            case ALIGN_CENTER:
                align_x = (opt.height - font_height) / 2;
                align_y = align_x;
                break;
            default:
                break;
            }
            int16_t drawtext_x = *x_now + align_x + opt.gif->w + opt.gif->x + 2;
            int16_t drawtext_y = font_offset_y + text_height_now + align_y + *y_now + 2;
            if (interface_text_y_now + drawtext_y + opt.y > *y_now &&
                interface_text_y_now + drawtext_y + opt.y < *y_now + *h_now + 12)
                D->drawUTF8(interface_text_x_now + drawtext_x + opt.x,
                            interface_text_y_now + drawtext_y + opt.y,
                            opt.name.c_str());
        }

        text_height_now += opt.height;
    }
}

void Axeuh_UI_TextMenu::draw_textmenu(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m)
{
    xSemaphoreTake(xMutex, 100);
    if (*if_display || !get_animation_interface_isok())
    {
        unsigned long currentMillis = millis();

        int text_height_now = 0;

        MenuOption &Box_opt = menuOptions[meun_number_now];

        int16_t interface_x_ = *interlude_x + *x; // 页面当前位置
        int16_t interface_y_ = *interlude_y + *y;
        int16_t interface_w_ = *interlude_w + *w;
        int16_t interface_h_ = *interlude_h + *h;

        if (Box_opt.mode == TEXT || Box_opt.mode == TEXT_MORE)
            pointer_x = 0;

        else if (Box_opt.mode == PICTURE || Box_opt.mode == PICTURE_TEXT)
            pointer_x = Box_opt.gif->x - 2;

        int pointer_y_all = 0;
        for (int i = 0; i < meun_number_now; i++)
            pointer_y_all += menuOptions[i].height;

        pointer_y = pointer_y_all + interface_text_y;

        /*Serial.print("pointer_y_now:" + (String)pointer_y_now);
        Serial.print("\t,y_now:" + (String)*y_now);
        Serial.print("\t,interface_h_:" + (String)interface_h_);
        Serial.print("\t,interface_text_y_now:" + (String)interface_text_y_now);

        Serial.println();*/

        if (in_put_now == DOWN)
        {
            if (Box_opt.isSelectable == No_Focusing)
            {
                if (meun_number_now < menuOptions_index - isSelectable_end - 1)
                {
                    meun_number_now++;
                }
                else
                {
                    int16_t pointer_no_able = 0;
                    for (int i = 0; i < isSelectable_end - 1; i++)
                        pointer_no_able += menuOptions[menuOptions_index - 1 - i].height;

                    interface_text_y = -(pointer_y_all - *h_now) - Box_opt.height - pointer_no_able - 3;
                    meun_number_now--;
                }
                if (menuOptions[meun_number_now].mode == TEXT || menuOptions[meun_number_now].mode == TEXT_MORE)
                    pointer_x = 0;

                else if (menuOptions[meun_number_now].mode == PICTURE || menuOptions[meun_number_now].mode == PICTURE_TEXT)
                    pointer_x = menuOptions[meun_number_now].gif->x - 2;

                pointer_y_all = 0;
                for (int i = 0; i < meun_number_now; i++)
                    pointer_y_all += menuOptions[i].height;

                pointer_y = pointer_y_all + interface_text_y;
            }
            else
            {
                if (pointer_y > interface_h_ - Box_opt.height - 3)
                {
                    pointer_y = interface_h_ - Box_opt.height - 3;
                    interface_text_y = -(pointer_y_all - interface_h_) - Box_opt.height - 3;
                }
            }
        }
        else if (in_put_now == UP)
        {
            if (Box_opt.isSelectable == No_Focusing)
            {
                if (meun_number_now > isSelectable_start - 1)
                {
                    meun_number_now--;
                }
                else
                {
                    int16_t pointer_no_able = 0;
                    for (int i = 0; i < isSelectable_start - 1; i++)
                        pointer_no_able += menuOptions[i].height;
                    interface_text_y = -pointer_y_all + pointer_no_able;
                    meun_number_now++;
                }

                if (menuOptions[meun_number_now].mode == TEXT || menuOptions[meun_number_now].mode == TEXT_MORE)
                    pointer_x = 0;

                else if (menuOptions[meun_number_now].mode == PICTURE || menuOptions[meun_number_now].mode == PICTURE_TEXT)
                    pointer_x = menuOptions[meun_number_now].gif->x - 2;

                pointer_y_all = 0;
                for (int i = 0; i < meun_number_now; i++)
                    pointer_y_all += menuOptions[i].height;
                pointer_y = pointer_y_all + interface_text_y;
            }
            else
            {
                if (pointer_y < 0)
                {
                    pointer_y = 0;
                    interface_text_y = -pointer_y_all;
                }
            }
        }
        else if (in_put_now == STOP)
        {
            if (Box_opt.isSelectable == No_Focusing)
            {
                meun_number_now++;
                if (meun_number_now > menuOptions_index)
                    meun_number_now = 0;
                xSemaphoreGive(xMutex);
                return;
            }
        }
        pointer_h = menuOptions[meun_number_now].height;
        int gif_get_x = 0;
        if (menuOptions[meun_number_now].gif != nullptr)
            gif_get_x = menuOptions[meun_number_now].gif->w + 1;
        if (menuOptions[meun_number_now].all_len == 0)
            init_text_more();

        uint16_t name_len = menuOptions[meun_number_now].all_len;

        if (menuOptions[meun_number_now].align == ALIGN_CENTER)
        {
            int mmmmm = menuOptions[meun_number_now].height - font_height;
            pointer_w = menuOptions[meun_number_now].x + gif_get_x + mmmmm + name_len + 4;
        }
        else
        {
            if (menuOptions[meun_number_now].x < 0)
                pointer_w = gif_get_x + 4;
            else
                pointer_w = menuOptions[meun_number_now].x + gif_get_x + name_len + 4;
        }

        if (pointer_w > *x_now + *w_now)
            pointer_w = *x_now + *w_now - 5;
        if (pointer_w_now > *x_now + *w_now)
            pointer_w_now = *x_now + *w_now - 5;

        if (pointer_w_now < 9)
        {
            pointer_w = 9;
            pointer_w_now = 9;
        }
        if (pointer_h_now < 9)
        {
            pointer_h = 9;
            pointer_h_now = 9;
        }

        if (!*lucency)
        {
            D->setDrawColor(0);
            D->drawBox(*x_now, *y_now, *w_now, *h_now);
        }
        D->setDrawColor(1);

        int clip_x = *x_now + 1;
        int clip_y = *y_now + 1;
        int clip_w = *w_now - 1;
        int clip_h = *h_now - 1;

        if (clip_x < 0)
            clip_x = 0;
        if (clip_y < 0)
            clip_y = 0;
        if (*x_now + clip_w < 0)
            clip_w = -*x_now;
        if (*y_now + clip_h < 0)
            clip_h = -*y_now;

        if (clip_x >= width)
            clip_x = width;
        if (clip_y >= height)
            clip_y = height;
        if (*x_now + clip_w >= width)
            clip_w = -*x_now + height;
        if (*y_now + clip_h >= height)
            clip_h = -*y_now + height;

        D->setClipWindow(clip_x, clip_y, *x_now + clip_w, *y_now + clip_h);
        D->setDrawColor(0);
        D->drawRBox(*x_now + pointer_x_now + 1, *y_now + pointer_y_now + 1, pointer_w_now - 1, pointer_h_now + 1, 0);
        D->setDrawColor(1);
        draw_MenuOption(D, currentMillis, text_height_now);

        D->setDrawColor(2);
        D->drawRBox(*x_now + pointer_x_now + 1, *y_now + pointer_y_now + 1, pointer_w_now - 1, pointer_h_now + 1, 0);

        D->setDrawColor(0);

        D->drawBox(progress_bar_black_x_now + *x_now + *w_now - 7,
                   *y_now + 1,
                   6,
                   *h_now - 2);
        D->setDrawColor(1);
        int line_1 = map(-interface_text_y_now, 0, text_height_now, 0, *h_now - 4);
        int line_2 = map(-interface_text_y_now + *h_now, 0, text_height_now, 0, *h_now - 4) + 1;

        D->drawRFrame(progress_bar_x2_now + *x_now + *w_now - 7,
                      *y_now + 1,
                      6,
                      *h_now - 2,
                      2);

        if (progress_bar_isok)
        {
            int box_h = line_2 - line_1 - 3;
            if (box_h < 1)
                box_h = 1;
            D->drawBox(progress_bar_x1_now + *x_now + *w_now - 5,
                       *y_now + line_1 + 3,
                       2,
                       box_h);
        }
        else
        {
            int box_h = line_2 - line_1 - 3 + 4;
            if (box_h < 1)
                box_h = 1;
            D->drawBox(progress_bar_x1_now + *x_now + *w_now - 5,
                       *y_now + line_1 + 1,
                       2,
                       box_h);
        }

        D->setMaxClipWindow();
        D->drawFrame(*x_now, *y_now, *w_now, *h_now);

        animation(x_now, interface_x_, m->fps / 3 * 4);
        animation(y_now, interface_y_, m->fps / 3 * 4);
        animation(w_now, interface_w_, m->fps / 3 * 4);
        animation(h_now, interface_h_, m->fps / 3 * 4);

        animation(&pointer_x_now, pointer_x, m->fps);
        animation(&pointer_y_now, pointer_y, m->fps);
        animation(&pointer_w_now, pointer_w, m->fps);
        animation(&pointer_h_now, pointer_h, m->fps);

        animation(&interface_text_x_now, interface_text_x, m->fps);
        animation(&interface_text_y_now, interface_text_y, m->fps);

        animation(&progress_bar_x1_now, progress_bar_x1, m->fps / 3 * 2);
        animation(&progress_bar_x2_now, progress_bar_x2, m->fps / 3 * 2);
        animation(&progress_bar_black_x_now, progress_bar_black_x, m->fps / 3 * 2);

        for (int i = 0; i < menuOptions_index; i++)
        {
            MenuOption &Box_opt_jpg = menuOptions[i];
            if (Box_opt_jpg.mode == PICTURE || Box_opt_jpg.mode == PICTURE_TEXT)
            {
                animation(&Box_opt_jpg.gif->x_now, Box_opt_jpg.gif->x_, m->fps / 3 * 5);
                animation(&Box_opt_jpg.gif->y_now, Box_opt_jpg.gif->y_, m->fps / 3 * 5);
            }
        }

        if ((int16_t)interface_text_x_now == (int16_t)interface_text_x &&
            (int16_t)interface_text_y_now == (int16_t)interface_text_y)
        {
            progress_bar_x1 = 2;
            progress_bar_x2 = 6;
            progress_bar_black_x = 2;
            progress_bar_isok = 0;
        }
        else
        {
            progress_bar_x1 = 0;
            progress_bar_x2 = 0;
            progress_bar_black_x = 0;
            progress_bar_isok = 1;
        }

        if (*if_Input)
        {
            if (IN == UP || IN == DOWN)
            {
                if (IN == UP && !handleInput && time_ == 0)
                {
                    in_put_now = UP;
                    if (meun_number_now > 0)
                        meun_number_now--;
                }
                else if (IN == DOWN && !handleInput && time_ == 0)
                {
                    in_put_now = DOWN;
                    if (meun_number_now < menuOptions_index - 1)
                        meun_number_now++;
                }
                if (keyboard_isok == 1 && keyboard_isok_number < 3)
                {
                    keyboard_isok_number += 1;
                }
                if (keyboard_isok == 0)
                {
                    keyboard_isok_number = 0;
                    keyboard_isok = 1;
                }

                if (time_ == 0)
                    time_ = 6 - keyboard_isok_number;
            }
            else
            {
                if (keyboard_isok_number != 0)
                    keyboard_isok_number--;
                keyboard_isok = 0;
            }

            if (IN == SELECT && Box_opt.isSelectable == Focused)
            {
                handleInput = 1;

                if (Box_opt.triggersAnimation == Trigger)
                    *interlude_w = -(*w - 3);
            }
            else
            {
                if (handleInput && get_animation_interface_isok())
                {
                    handleInput = 0;
                    xSemaphoreGive(xMutex);
                    staticCallback(this, P, m);
                }
            }

            if (time_ != 0)
            {
                if (millis() - lastMillis > 25)
                {
                    lastMillis = millis();
                    time_--;
                }
            }
        }
    }
    xSemaphoreGive(xMutex);
}
void Axeuh_UI_slider::drawSlider(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m)
{
    if (*if_display || !get_animation_all_isok())
    {
        float num = 0;
        if (s.num_ != nullptr)
            num = *s.num_;
        else if (s.num_uint16 != nullptr)
            num = *s.num_uint16;
        else
            num = *s.num;

        int16_t x_ = *interlude_x + *x; // 页面位置
        int16_t y_ = *interlude_y + *y;
        int16_t w_ = *interlude_w + *w;
        int16_t h_ = *interlude_h + *h;
        int16_t r_ = *interlude_r + *r;

        int h_now_ = *h_now;
        if (*h_now < 22)
        {
            h_now_ = 22;
        }
        if (!*lucency)
        {
            D->setDrawColor(0);
            D->drawRBox(*x_now, *y_now, *w_now, h_now_, *r_now);
        }
        if (*y_now + h_now_ > 0)
        {

            progress_w = map(num * 100, s.min * 100, s.max * 100, 0, *w_now - 16);

            D->setDrawColor(1);
            D->drawRFrame(*x_now, *y_now, *w_now, h_now_, *r_now);

            D->drawFrame(*x_now + 8, *y_now + h_now_ - 16, *w_now - 16, 10);

            D->drawBox(*x_now + 8, *y_now + h_now_ - 16, progress_w_now, 10);

            D->setDrawColor(2);
            if (h_now_ >= 34)
            {
                D->drawUTF8(*x_now + 8, *y_now + 15, (s.name + ":").c_str());
            }
            String num_str;
            if ((int)unit == unit)
                num_str = String((int)num);
            else
                num_str = String(num);
            if (h_now_ < 46)
            {
                D->drawUTF8(*x_now + 8 + 20, *y_now + h_now_ - 16 + 9, (num_str + "|" + String(s.max)).c_str());
            }
            else
            {
                D->drawUTF8(*x_now + 8, *y_now + 27, (num_str + "|" + String(s.max)).c_str());
            }

            if (*if_Input)
            {
                if (IN == LEFT || IN == RIGHT)
                {
                    if (holdStartTime == 0)
                    {
                        // 单次增减1
                        if (IN == LEFT)
                            num -= 1 * unit;
                        else
                            num += 1 * unit;
                        num = constrain(num, s.min, s.max);

                        holdStartTime = millis();
                        initialDelayPassed = false;
                        time_ = 20; // 20 * 25ms=500ms延迟
                        keyboard_isok_number = 0;
                        keyboard_isok = 1;
                    }
                    else
                    {
                        if (millis() - holdStartTime >= 500)
                        {
                            initialDelayPassed = true;
                        }
                    }

                    // 加速阶段处理
                    if (initialDelayPassed)
                    {
                        if (time_ == 0)
                        {
                            // 每次仅增减1
                            if (IN == LEFT)
                                num -= (1 + num_speed_up) * unit;
                            else
                                num += (1 + num_speed_up) * unit;
                            num = constrain(num, s.min, s.max);

                            // 动态计算间隔时间（6-keyboard_isok_number）
                            time_ = 6 - keyboard_isok_number;

                            // 渐进加速
                            if (keyboard_isok && keyboard_isok_number < 5)
                            {
                                keyboard_isok_number++;
                            }
                            else
                            {
                                if (num_speed_up < s.max / 10)
                                    num_speed_up++;
                            }
                        }
                    }
                }
                else if (IN == SELECT)
                {
                    handleInput = 1;
                }
                else
                {
                    // 释放按键时重置状态
                    holdStartTime = 0;
                    initialDelayPassed = false;
                    keyboard_isok = 0;
                    keyboard_isok_number = 0;
                    time_ = 0;
                    num_speed_up = 0;

                    if (handleInput)
                    {
                        **return_num_pointer = 1;
                        handleInput = 0;
                        *if_display = 0;
                        *if_Input = 0;
                        *interlude_y = -64;
                        //*y = -64;
                    }
                }

                // 更新时间计数器
                if (time_ != 0)
                {
                    if (millis() - lastMillis > 25)
                    {
                        lastMillis = millis();
                        time_--;
                    }
                }
            }
        }
        if (s.num_ != nullptr)
            *s.num_ = (int)num;
        else if (s.num_uint16 != nullptr)
            *s.num_uint16 = (uint16_t)num;
        else
            *s.num = num;

        animation(x_now, x_, m->fps / 3 * 4);
        animation(y_now, y_, m->fps / 3 * 4);
        animation(w_now, w_, m->fps / 3 * 4);
        animation(h_now, h_, m->fps / 3 * 4);
        animation(r_now, r_, m->fps / 3 * 4);
        animation(&progress_w_now, progress_w, m->fps / 3 * 4);
    }
}
void Axeuh_UI_Cube::drawCube(U8G2 *D, Axeuh_UI *m)
{
    xSemaphoreTake(xMutex, 100);
    unsigned long currentTime = millis();

    D->setDrawColor(1);
    // 更新旋转角度（形成动画效果）
    if (currentTime - lastTime > 16)
    {
        lastTime = currentTime;
        angleX += angleX_speed; // X轴旋转速度
        angleY += angleY_speed; // Y轴旋转速度
        angleZ += angleZ_speed; // Z轴旋转速度
    }

    // 预计算所有顶点的旋转坐标
    for (int i = 0; i < 8; i++)
    {
        rotatePoint(cube[i][0], cube[i][1], cube[i][2],
                    rotatedCube[i][0], rotatedCube[i][1], rotatedCube[i][2]);
    }

    // 遍历所有6个面
    for (int f = 0; f < 6; f++)
    {
        Face face = faces[f]; // 获取当前面

        // 计算面法线旋转后的方向
        float fnx, fny, fnz;
        rotatePoint(face.normal[0], face.normal[1], face.normal[2], fnx, fny, fnz);

        // 背面剔除算法
        // 当旋转后的法线Z分量>0时，表示面朝后（观察方向为-Z）
        if (fnz > 0)
            continue; // 跳过背面绘制

        // 绘制面的四条边
        for (int e = 0; e < 4; e++)
        {
            int from = face.vertices[e];         // 边的起点索引
            int to = face.vertices[(e + 1) % 4]; // 边的终点索引（循环连接）

            // 将3D坐标转换为屏幕坐标
            // 64和32是屏幕中心坐标（128x64分辨率）
            int x1 = rotatedCube[from][0] * cube_scale_now + cube_x_now;
            int y1 = rotatedCube[from][1] * cube_scale_now + cube_y_now;
            int x2 = rotatedCube[to][0] * cube_scale_now + cube_x_now;
            int y2 = rotatedCube[to][1] * cube_scale_now + cube_y_now;

            // 绘制线段
            D->drawLine(x1, y1, x2, y2);
        }
    }
    animation(&cube_x_now, cube_x, m->fps / 3 * 5);
    animation(&cube_y_now, cube_y, m->fps / 3 * 5);
    animation(&angleX_now, angleX, m->fps / 3 * 5, 0.1);
    animation(&angleY_now, angleY, m->fps / 3 * 5, 0.1);
    animation(&angleZ_now, angleZ, m->fps / 3 * 5, 0.1);
    animation(&cube_scale_now, cube_scale, m->fps / 3 * 5, 0.1);
    xSemaphoreGive(xMutex);
}
#ifdef CHINESE_KEYBOARD
void Axeuh_UI_Keyboard::drawKeyboard(U8G2 *D, IN_PUT_Mode IN, Axeuh_UI_Panel *P, Axeuh_UI *m) // 史山
{
    if (*if_display || !get_animation_interface_isok())
    {
        String output_str = "";
        if (Aoutput != nullptr)
        {
            output_str = *Aoutput;
        }
        else
        {
            if (output != nullptr)
                output_str = *output;
        }

        for (int i = 0; i < 41; i++)
        {
            if (pinyin_MEUN[i].this_c == key.text_now)
            {
                keyboard_now = i;
                break;
            }
        }

        if (!*lucency)
        {
            D->setDrawColor(0);
            D->drawBox(0, 0 + key.input_box_now, 128, 12);
            D->drawBox(0 + key.key_boa_ui_now, 12, 114, 52);
            D->drawBox(114 + key.cn_box_ui_now, 12, 14, 52);
        }

        D->setDrawColor(1);

        D->drawFrame(0, 0 + key.input_box_now, 128, 12);

        D->drawRFrame(0 + key.key_boa_ui_now, 13, 11, 13, 3);
        D->drawRFrame(12 + key.key_boa_ui_now, 13, 13, 12, 3);
        D->drawRFrame(26 + key.key_boa_ui_now, 13, 9, 12, 3);
        D->drawRFrame(36 + key.key_boa_ui_now, 13, 9, 12, 3);
        D->drawRFrame(46 + key.key_boa_ui_now, 13, 11, 12, 3);
        D->drawRFrame(58 + key.key_boa_ui_now, 13, 11, 12, 3);
        D->drawRFrame(70 + key.key_boa_ui_now, 13, 10, 12, 3);
        D->drawRFrame(81 + key.key_boa_ui_now, 13, 7, 12, 3);
        D->drawRFrame(89 + key.key_boa_ui_now, 13, 11, 12, 3);
        D->drawRFrame(101 + key.key_boa_ui_now, 13, 9, 12, 3);
        D->drawRFrame(5 + key.key_boa_ui_now, 26, 11, 12, 3);
        D->drawRFrame(17 + key.key_boa_ui_now, 26, 9, 12, 3);
        D->drawRFrame(27 + key.key_boa_ui_now, 26, 11, 12, 3);
        D->drawRFrame(39 + key.key_boa_ui_now, 26, 9, 12, 3);
        D->drawRFrame(49 + key.key_boa_ui_now, 26, 10, 12, 3);
        D->drawRFrame(60 + key.key_boa_ui_now, 26, 10, 12, 3);
        D->drawRFrame(71 + key.key_boa_ui_now, 26, 8, 14, 3);
        D->drawRFrame(80 + key.key_boa_ui_now, 26, 10, 12, 3);
        D->drawRFrame(91 + key.key_boa_ui_now, 26, 10, 12, 3);

        D->drawRFrame(0 + key.key_boa_ui_now, 39, 13, 12, 3);
        D->drawRFrame(14 + key.key_boa_ui_now, 39, 11, 12, 3);
        D->drawRFrame(26 + key.key_boa_ui_now, 39, 10, 12, 3);
        D->drawRFrame(37 + key.key_boa_ui_now, 39, 10, 12, 3);
        D->drawRFrame(48 + key.key_boa_ui_now, 39, 11, 12, 3);
        D->drawRFrame(60 + key.key_boa_ui_now, 39, 10, 12, 3);
        D->drawRFrame(71 + key.key_boa_ui_now, 39, 10, 12, 3);
        D->drawRFrame(82 + key.key_boa_ui_now, 39, 11, 12, 3);
        D->drawRFrame(94 + key.key_boa_ui_now, 39, 16, 12, 3);
        D->drawRFrame(0 + key.key_boa_ui_now, 52, 15, 12, 3);
        D->drawRFrame(16 + key.key_boa_ui_now, 52, 15, 12, 3);
        D->drawRFrame(32 + key.key_boa_ui_now, 52, 9, 12, 3);
        D->drawRFrame(42 + key.key_boa_ui_now, 52, 21, 12, 3);
        D->drawRFrame(64 + key.key_boa_ui_now, 52, 9, 12, 3);
        D->drawRFrame(74 + key.key_boa_ui_now, 52, 17, 12, 3);
        D->drawRFrame(92 + key.key_boa_ui_now, 52, 18, 12, 3);

        D->drawRBox(key.x_now + key.key_boa_ui_now, key.y_now, key.w_now, key.h_now, 3);

        if (key.capslk)
            D->drawRBox(0 + key.key_boa_ui_now, 39, 13, 12, 3);

        D->setDrawColor(2);
        D->drawUTF8(2, 10 + key.input_box_now, (output_str + CE_out).c_str());

        const char **key_arr;
        int key_int = 0;

        if (CE_of)
            key_arr = PINYIN_MEUN_ARR_CN;
        else if (num_of)
            key_arr = PINYIN_MEUN_arr_num;
        else if (key.capslk)
            key_arr = PINYIN_MEUN_ARR;
        else
            key_arr = PINYIN_MEUN_arr;

        String mystring = "";
        if (!CE_of)
        {
            D->drawUTF8(114 + key.cn_box_ui_now, 31, "贺");
            D->drawUTF8(114 + key.cn_box_ui_now, 43, "吸");
            D->drawUTF8(114 + key.cn_box_ui_now, 55, "呼");
        }
        else
        {
            char arr[CE_out.length() + 1];
            CE_out.toCharArray(arr, CE_out.length() + 1);
            char *CE_out_ = PYSearch(arr, &CE_num_max);

            // Serial.print("num:");
            // Serial.println(CE_out_);

            if (CE_out_ == NULL)
            {
                CE_out_ = "";
            }
            mystring = CE_out_;
            char firstChineseChar1[4] = {CE_out_[0], CE_out_[1], CE_out_[2]};
            int i = CE_num;
            D->drawUTF8(114 + key.cn_box_ui_now, 31, (mystring.substring(i * 3, i++ * 3 + 3)).c_str());
            D->drawUTF8(114 + key.cn_box_ui_now, 43, (mystring.substring(i * 3, i++ * 3 + 3)).c_str());
            D->drawUTF8(114 + key.cn_box_ui_now, 55, (mystring.substring(i * 3, i++ * 3 + 3)).c_str());
        }

        if (CE_num > 0)
            D->drawTriangle(114, 19, 120, 13, 126, 19);
        if (CE_num + 3 < CE_num_max)
            D->drawTriangle(115, 57, 120, 63, 125, 57);

        D->drawUTF8(2 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(14 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(28 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(38 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(48 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(60 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(72 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(83 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(91 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(103 + key.key_boa_ui_now, 23, key_arr[key_int++]);
        D->drawUTF8(7 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(19 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(29 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(41 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(51 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(62 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(73 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(82 + key.key_boa_ui_now, 36, key_arr[key_int++]);
        D->drawUTF8(93 + key.key_boa_ui_now, 36, key_arr[key_int++]);

        if (3 + key.key_boa_ui_now >= 0)
        {
            D->drawLine(3 + key.key_boa_ui_now, 44, 6 + key.key_boa_ui_now, 41);
            D->drawLine(7 + key.key_boa_ui_now, 42, 9 + key.key_boa_ui_now, 44);
            D->drawLine(6 + key.key_boa_ui_now, 42, 6 + key.key_boa_ui_now, 47);
        }

        D->drawUTF8(16 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(28 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(39 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(50 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(62 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(73 + key.key_boa_ui_now, 49, key_arr[key_int++]);
        D->drawUTF8(84 + key.key_boa_ui_now, 49, key_arr[key_int++]);

        if (97 + key.key_boa_ui_now >= 0)
        {
            D->drawLine(97 + key.key_boa_ui_now, 44, 100 + key.key_boa_ui_now, 41);
            D->drawLine(98 + key.key_boa_ui_now, 45, 100 + key.key_boa_ui_now, 47);
            D->drawLine(98 + key.key_boa_ui_now, 44, 106 + key.key_boa_ui_now, 44);
        }

        D->drawUTF8(2 + key.key_boa_ui_now, 62, "符");
        D->drawUTF8(18 + key.key_boa_ui_now, 62, "数");

        if (46 + key.key_boa_ui_now >= 0)
        {
            D->drawLine(46 + key.key_boa_ui_now, 60, 57 + key.key_boa_ui_now, 60);
            D->drawLine(46 + key.key_boa_ui_now, 60, 46 + key.key_boa_ui_now, 57);
            D->drawLine(57 + key.key_boa_ui_now, 57, 57 + key.key_boa_ui_now, 60);
        }

        D->drawUTF8(32 + key.key_boa_ui_now, 60, key_arr[key_int++]);
        D->drawUTF8(64 + key.key_boa_ui_now, 60, key_arr[key_int++]);

        D->drawUTF8(76 + key.key_boa_ui_now, 62, "CE");

        if (95 + key.key_boa_ui_now > 0)
        {
            D->drawLine(95 + key.key_boa_ui_now, 60, 100 + key.key_boa_ui_now, 55);
            D->drawLine(96 + key.key_boa_ui_now, 60, 105 + key.key_boa_ui_now, 60);
        }

        D->setDrawColor(1);

        animation(&key.x_now, pinyin_MEUN[keyboard_now].x, m->fps / 3 * 5);
        animation(&key.y_now, pinyin_MEUN[keyboard_now].y, m->fps / 3 * 5);
        animation(&key.h_now, pinyin_MEUN[keyboard_now].h, m->fps / 3 * 5);
        animation(&key.w_now, pinyin_MEUN[keyboard_now].w, m->fps / 3 * 5);
        animation(&key.input_box_now, key.input_box, m->fps / 3 * 5);
        animation(&key.key_boa_ui_now, key.key_boa_ui, m->fps / 3 * 5);
        animation(&key.cn_box_ui_now, key.cn_box_ui, m->fps / 3 * 5);

        if (*if_Input)
        {
            if (time_ == 0)
            {
                if (IN != STOP)
                {
                    time_ = 5;
                    if (IN == UP) // 向上
                    {
                        key.text_now = pinyin_MEUN[keyboard_now].up;
                    }
                    else if (IN == DOWN) // 向下
                    {
                        key.text_now = pinyin_MEUN[keyboard_now].down;
                    }
                    else if (IN == LEFT) // 左
                    {
                        key.text_now = pinyin_MEUN[keyboard_now].left;
                    }
                    else if (IN == RIGHT) // 右
                    {
                        key.text_now = pinyin_MEUN[keyboard_now].right;
                    }
                    else if (IN == SELECT) // 确认
                    {
                        if (key.text_now == "input")
                        {
                        }
                        else if (key.text_now == "daxie")
                        {
                            key.capslk = !key.capslk;
                        }
                        else if (key.text_now == "fu")
                        {
                        }
                        else if (key.text_now == "shu")
                        {
                            num_of = !num_of;
                        }
                        else if (key.text_now == "up")
                        {
                            if (CE_of)
                                if (CE_num > 0)
                                    CE_num--;
                        }
                        else if (key.text_now == "text1")
                        {
                            output_str += mystring.substring(CE_num * 3, CE_num * 3 + 3);
                            CE_num = 0;
                            CE_num_max = 0;
                            CE_out = "";
                        }
                        else if (key.text_now == "text2")
                        {
                            output_str += mystring.substring((CE_num + 1) * 3, (CE_num + 1) * 3 + 3);
                            CE_num = 0;
                            CE_num_max = 0;
                            CE_out = "";
                        }
                        else if (key.text_now == "text3")
                        {
                            output_str += mystring.substring((CE_num + 2) * 3, (CE_num + 2) * 3 + 3);
                            CE_num = 0;
                            CE_num_max = 0;
                            CE_out = "";
                        }
                        else if (key.text_now == "down")
                        {
                            if (CE_of)
                                if (CE_num + 3 < CE_num_max)
                                    CE_num++;
                        }
                        else if (key.text_now == "CE")
                        {
                            CE_of = !CE_of;
                            output_str += CE_out;
                            CE_out = "";
                        }
                        else if (key.text_now == "Enter")
                        {
                            if (CE_out == "")
                            {
                                handleInput = 1;
                            }
                            else
                            {
                                output_str += CE_out;
                                CE_out = "";
                            }
                        }
                        else if (key.text_now == "tuige")
                        {
                            // 检查字符串是否非空
                            // 删除最后一个字符
                            if (CE_out == "")
                            {
                                if ((output_str).length() > 0)
                                    if (isChineseChar((output_str).charAt((output_str).length() - 3)))
                                        (output_str).remove((output_str).length() - 3);
                                    else
                                        (output_str).remove((output_str).length() - 1);
                            }
                            else if (CE_out.length() > 0)
                                CE_out.remove(CE_out.length() - 1);
                        }
                        else if (key.text_now == " ")
                        {
                            if (!CE_of)
                                output_str += " ";
                            else
                                CE_out += " ";
                        }
                        else if (key.text_now == ",")
                        {
                            if (!CE_of)
                                output_str += ",";
                            else
                                CE_out += "，";
                        }
                        else if (key.text_now == ".")
                        {
                            if (!CE_of)
                                output_str += ".";
                            else
                                CE_out += "。";
                        }
                        else
                        {
                            if (key.capslk)
                            {
                                key.capslk = !key.capslk;
                                output_str += key_arr[keyboard_now];
                            }
                            else
                            {
                                if (!CE_of)
                                    output_str += key_arr[keyboard_now];
                                else
                                    CE_out += PINYIN_MEUN_arr[keyboard_now];
                            }
                        }
                    }
                }
                else if (handleInput)
                {
                    handleInput = 0;
                    *if_display = 0;
                    *if_Input = 0;
                    key.input_box = -12;
                    key.key_boa_ui = -128;
                    key.cn_box_ui = 24;

                    **return_num_pointer = 1;
                }
            }
            else
            {
                if (millis() - lastMillis > 35)
                {
                    lastMillis = millis();
                    time_--;
                }
            }
        }

        if (Aoutput != nullptr)
        {
            *Aoutput = output_str;
        }
        else
        {
            if (output != nullptr)
                *output = output_str;
        }
    }
}
#endif
void Axeuh_UI::menu_display() // 绘制函数
{
    esp_task_wdt_init(30, false); // 设置看门狗为30s，且不重启
    while (1)
    {
        const uint32_t frameStart = micros(); // 本帧开始时间

        frameCount++;
        unsigned long currentTime = millis();
        unsigned long timeElapsed = currentTime - lastTime;

        if (timeElapsed >= 100) // 每0.1秒更新一次
        {
            float fps_ = frameCount * 10 / (timeElapsed / 100.0);
            fps = fps_filter.update(fps_); // 卡尔曼滤波计算当前fps
            frameCount = 0;
            lastTime = currentTime;
        }

        if (input_callback != nullptr) // 检查输入状态
            IN_now = handleInput();

        D->clearBuffer(); // 清理缓冲区

        xSemaphoreTake(xMutex_, 100);
        if (StatusBar != nullptr) // 绘制状态栏
            StatusBar->drawStatusBar(D, this);
        if (cube != nullptr) // 绘制立方体
            cube->drawCube(D, this);
        if (Panel != nullptr) // 绘制首级菜单
            Panel->drawPanel(D, this, IN_now);
        xSemaphoreGive(xMutex_);

        D->sendBuffer(); // 发生缓冲区

        const uint32_t elapsed = micros() - frameStart; // 本帧已耗时
        int32_t remaining = 1000000 / fps_max - elapsed;
        remaining += fps_speed_; // 误差补偿
        if (remaining > 0)       // 限制最大帧率，减少CPU占用
        {
            if (fps > fps_max)
                fps_speed_++;
            else if (fps < fps_max)
                fps_speed_--;
            if (remaining > 1000)
            {
                delay(remaining / 1000);
                delayMicroseconds(remaining % 1000);
            }
            else
            {
                delay(1); // 最低1ms防止触发看门狗
            }
        }
        else
        {
            delay(1);
        }
        // esp_task_wdt_reset();
        // delay(1);

        // uint32_t totalHeap = ESP.getHeapSize();
        // uint32_t freeHeap = ESP.getFreeHeap();
        // uint32_t usedHeap = totalHeap - freeHeap;

        // Serial.printf("已用堆内存: %u 字节 (约 %.1f KB)\t", usedHeap, usedHeap / 1024.0);
        // Serial.printf("可用堆内存: %u 字节 (约 %.1f KB)\n", freeHeap, freeHeap / 1024.0);
    }
}
