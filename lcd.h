/*
 * Copyright (c) 2019-2020, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LCD_H_
#define _LCD_H_

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "fonts.h"
#include "images.h"

/*- Definitions -------------------------------------------------------------*/
#define LCD_WIDTH      320
#define LCD_HEIGHT     240

#define LCD_COLOR(r, g, b) \
    ((((r) << 8) & 0xf800) | (((g) << 3) & 0x07e0) | (((b) >> 3) & 0x001f))

#define LCD_BLACK_COLOR        LCD_COLOR(0, 0, 0)
#define LCD_WHITE_COLOR        LCD_COLOR(255, 255, 255)
#define LCD_RED_COLOR          LCD_COLOR(255, 0, 0)
#define LCD_GREEN_COLOR        LCD_COLOR(0, 255, 0)
#define LCD_BLUE_COLOR         LCD_COLOR(0, 0, 255)

/*- Prototypes --------------------------------------------------------------*/
void lcd_init(void);
void lcd_set_backlight_level(int level);
void lcd_draw_pixel(int x, int y, int color);
void lcd_draw_buf(int x, int y, int w, int h, const uint16_t *buf);
void lcd_draw_image(int x, int y, const Image *image);
void lcd_draw_rect(int x, int y, int w, int h, int color);
void lcd_fill_rect(int x, int y, int w, int h, int color);
void lcd_hline(int x0, int x1, int y, int color);
void lcd_vline(int x, int y0, int y1, int color);
void lcd_set_font(const Font *font);
void lcd_set_color(int bg, int fg);
void lcd_putc(int x, int y, char ch);
void lcd_puts(int x, int y, const char *str);

#endif // _LCD_H_

