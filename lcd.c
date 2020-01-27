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

/*- Includes ----------------------------------------------------------------*/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx.h"
#include "hal_gpio.h"
#include "utils.h"
#include "fonts.h"
#include "lcd.h"

/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(LCD_RST,  C, 6)
HAL_GPIO_PIN(LCD_RD,   B, 3)
HAL_GPIO_PIN(LCD_WR,   B, 4)
HAL_GPIO_PIN(LCD_RS,   B, 5)
HAL_GPIO_PIN(LCD_CS,   B, 6)
HAL_GPIO_PIN(LCD_BL,   B, 0)
// LCD_D[7:0] - PE[7:0]

// Note: Not all commands are enumerated. here. Read the display datasheet
// for a full list of supported commands.
enum
{
  ST7789_NOP       = 0x00,
  ST7789_SWRESET   = 0x01,
  ST7789_RDDID     = 0x04,
  ST7789_RDDST     = 0x09,
  ST7789_SLPIN     = 0x10,
  ST7789_SLPOUT    = 0x11,
  ST7789_DISPOFF   = 0x28,
  ST7789_DISPON    = 0x29,
  ST7789_CASET     = 0x2a,
  ST7789_RASET     = 0x2b,
  ST7789_RAMWR     = 0x2c,
  ST7789_RAMRD     = 0x2e,
  ST7789_MADCTL    = 0x36,
  ST7789_COLMOD    = 0x3a,
  ST7789_PORCTRL   = 0xb2,
  ST7789_GCTRL     = 0xb7,
  ST7789_VCOMS     = 0xbb,
  ST7789_PWCTR1    = 0xc0,
  ST7789_PWCTR2    = 0xc1,
  ST7789_PWCTR3    = 0xc2,
  ST7789_PWCTR4    = 0xc3,
  ST7789_PWCTR5    = 0xc4,
  ST7789_VMCTR1    = 0xc5,
  ST7789_FRCTR2    = 0xc6,
  ST7789_PWCTRL1   = 0xd0,
};

/*- Variables ---------------------------------------------------------------*/
static const Font *lcd_font = NULL;
static int bg_color[2];
static int fg_color[2];

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void lcd_data_set_in(void)
{
  GPIOE->CTL &= ~0xffff;
}

//-----------------------------------------------------------------------------
static inline void lcd_data_set_out(void)
{
  GPIOE->CTL |= 0x5555;
}

//-----------------------------------------------------------------------------
static inline void lcd_data_write(uint8_t value)
{
  GPIOE->BOP = (~value << 16) | value;
  asm("nop");
  asm("nop");
  asm("nop");
  HAL_GPIO_LCD_WR_clr();
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  HAL_GPIO_LCD_WR_set();
  asm("nop");
  asm("nop");
  asm("nop");
}

//-----------------------------------------------------------------------------
static inline void lcd_command_write(uint8_t cmd)
{
  HAL_GPIO_LCD_RS_clr();
  lcd_data_write(cmd);
  HAL_GPIO_LCD_RS_set();
}

//-----------------------------------------------------------------------------
static void lcd_cmd(int cmd, int size, uint8_t *data)
{
  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(cmd);

  for (int i = 0; i < size; i++)
    lcd_data_write(data[i]);

  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
static void lcd_set_rect(int x, int y, int w, int h)
{
  int xe = x + w - 1;
  int ye = y + h - 1;
  uint8_t buf[4];

  buf[0] = x >> 8;
  buf[1] = x;
  buf[2] = xe >> 8;
  buf[3] = xe;
  lcd_cmd(ST7789_CASET, 4, buf);

  buf[0] = y >> 8;
  buf[1] = y;
  buf[2] = ye >> 8;
  buf[3] = ye;
  lcd_cmd(ST7789_RASET, 4, buf);
}

//-----------------------------------------------------------------------------
void lcd_init(void)
{
  HAL_GPIO_LCD_RST_out();
  HAL_GPIO_LCD_RST_set();

  HAL_GPIO_LCD_CS_out();
  HAL_GPIO_LCD_CS_set();

  HAL_GPIO_LCD_RD_out();
  HAL_GPIO_LCD_RD_set();

  HAL_GPIO_LCD_WR_out();
  HAL_GPIO_LCD_WR_set();

  HAL_GPIO_LCD_RS_out();
  HAL_GPIO_LCD_RS_set();

  lcd_data_set_in();
  lcd_data_set_out();

  HAL_GPIO_LCD_RST_clr();
  delay_ms(10);
  HAL_GPIO_LCD_RST_set();
  delay_ms(5);

  lcd_cmd(ST7789_DISPOFF, 0, NULL);
  lcd_cmd(ST7789_SLPOUT,  0, NULL);
  lcd_cmd(ST7789_MADCTL,  1, (uint8_t[]){ 0x60 });
  lcd_cmd(ST7789_COLMOD,  1, (uint8_t[]){ 0x55 });
  lcd_cmd(ST7789_PORCTRL, 5, (uint8_t[]){ 0x08, 0x08, 0x00, 0x22, 0x22 });
  lcd_cmd(ST7789_GCTRL,   1, (uint8_t[]){ 0x35 });
  lcd_cmd(ST7789_VCOMS,   1, (uint8_t[]){ 0x2b });
  lcd_cmd(ST7789_PWCTR1,  1, (uint8_t[]){ 0x2c });
  lcd_cmd(ST7789_PWCTR3,  2, (uint8_t[]){ 0x01, 0xff });
  lcd_cmd(ST7789_PWCTR4,  1, (uint8_t[]){ 0x11 });
  lcd_cmd(ST7789_PWCTR5,  1, (uint8_t[]){ 0x20 });
  lcd_cmd(ST7789_FRCTR2,  1, (uint8_t[]){ 0x0f });
  lcd_cmd(ST7789_PWCTRL1, 2, (uint8_t[]){ 0xa4, 0xa1 });
  lcd_cmd(ST7789_DISPON,  0, NULL);

  lcd_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, 0);

  lcd_set_backlight_level(100);
}

//-----------------------------------------------------------------------------
static void pwm_start(int level)
{
  RCU->APB1EN_b.TIMER2EN = 1;

  TIMER2->CTL0  = 0;
  TIMER2->CNT   = 0;
  TIMER2->PSC   = 20;
  TIMER2->CAR   = 51200;
  TIMER2->CH2CV = level * 512;
  TIMER2->CHCTL1_Output = (6 << TIMER2_CHCTL1_Output_CH2COMCTL_Pos);
  TIMER2->CHCTL2 = TIMER2_CHCTL2_CH2EN_Msk;
  TIMER2->CTL0  = TIMER2_CTL0_CEN_Msk;
}

//-----------------------------------------------------------------------------
static void pwm_stop(void)
{
  TIMER2->CTL0 = 0;
}

//-----------------------------------------------------------------------------
void lcd_set_backlight_level(int level)
{
  if (level < 0)
    level = 0;
  else if (level > 100)
    level = 100;

  if (level == 0)
  {
    HAL_GPIO_LCD_BL_out();
    HAL_GPIO_LCD_BL_clr();
    pwm_stop();
  }
  else if (level == 100)
  {
    HAL_GPIO_LCD_BL_out();
    HAL_GPIO_LCD_BL_set();
    pwm_stop();
  }
  else
  {
    HAL_GPIO_LCD_BL_alt(2);
    pwm_start(level);
  }
}

//-----------------------------------------------------------------------------
void lcd_draw_pixel(int x, int y, int color)
{
  lcd_set_rect(x, y, 1, 1);

  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(ST7789_RAMWR);
  lcd_data_write((color >> 8) & 0xff);
  lcd_data_write(color & 0xff);
  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
void lcd_draw_buf(int x, int y, int w, int h, const uint16_t *buf)
{
  int size = w * h;

  lcd_set_rect(x, y, w, h);

  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(ST7789_RAMWR);

  for (int i = 0; i < size; i++)
  {
    lcd_data_write(buf[i] >> 8);
    lcd_data_write(buf[i]);
  }

  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
void lcd_draw_image(int x, int y, const Image *image)
{
  int size = image->width * image->height;

  lcd_set_rect(x - image->ox, y - image->oy, image->width, image->height);

  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(ST7789_RAMWR);

  for (int i = 0; i < size; i++)
  {
    lcd_data_write(image->data[i] >> 8);
    lcd_data_write(image->data[i]);
  }

  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
void lcd_draw_rect(int x, int y, int w, int h, int color)
{
  w -= 1;
  h -= 1;
  lcd_vline(x,   y,   y+h, color);
  lcd_vline(x+w, y,   y+h, color);
  lcd_hline(x,   x+w, y,   color);
  lcd_hline(x,   x+w, y+h, color);
}

//-----------------------------------------------------------------------------
void lcd_fill_rect(int x, int y, int w, int h, int color)
{
  int size = w * h;
  int c0 = (color >> 8) & 0xff;
  int c1 = color & 0xff;

  lcd_set_rect(x, y, w, h);

  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(ST7789_RAMWR);

  for (int i = 0; i < size; i++)
  {
    lcd_data_write(c0);
    lcd_data_write(c1);
  }

  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
void lcd_hline(int x0, int x1, int y, int color)
{
  lcd_fill_rect(x0, y, x1 - x0 + 1, 1, color);
}

//-----------------------------------------------------------------------------
void lcd_vline(int x, int y0, int y1, int color)
{
  lcd_fill_rect(x, y0, 1, y1 - y0 + 1, color);
}

//-----------------------------------------------------------------------------
void lcd_set_font(const Font *font)
{
  lcd_font = font;
}

//-----------------------------------------------------------------------------
void lcd_set_color(int bg, int fg)
{
  bg_color[0] = (bg >> 8) & 0xff;
  bg_color[1] = bg & 0xff;

  fg_color[0] = (fg >> 8) & 0xff;
  fg_color[1] = fg & 0xff;
}

//-----------------------------------------------------------------------------
void lcd_putc(int x, int y, char ch)
{
  int size = lcd_font->width * lcd_font->height;
  const uint8_t *bitmap;

  lcd_set_rect(x, y, lcd_font->width, lcd_font->height);

  HAL_GPIO_LCD_CS_clr();
  lcd_command_write(ST7789_RAMWR);

  if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR)
    ch = '?';

  bitmap = lcd_font->data + (ch - FONT_FIRST_CHAR) * lcd_font->pitch;

  for (int i = 0; i < size; i++)
  {
    int byte = bitmap[i / 8];
    int pixel = (byte >> (i % 8)) & 1;

    if (pixel)
    {
      lcd_data_write(fg_color[0]);
      lcd_data_write(fg_color[1]);
    }
    else
    {
      lcd_data_write(bg_color[0]);
      lcd_data_write(bg_color[1]);
    }
  }

  HAL_GPIO_LCD_CS_set();
}

//-----------------------------------------------------------------------------
void lcd_puts(int x, int y, const char *str)
{
  while (*str)
  {
    if (FONT_HALF_SPACE == *str)
    {
      int color = (bg_color[0] << 8) | bg_color[1];
      lcd_fill_rect(x, y, lcd_font->width / 2, lcd_font->height, color);
      x += lcd_font->width / 2;
    }
    else
    {
      lcd_putc(x, y, *str);
      x += lcd_font->width;
    }

    str++;
  }
}


