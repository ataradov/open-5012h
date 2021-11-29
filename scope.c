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
#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx.h"
#include "hal_gpio.h"
#include "utils.h"
#include "lcd.h"
#include "timer.h"
#include "images.h"
#include "common.h"
#include "config.h"
#include "buttons.h"
#include "capture.h"
#include "scope.h"

/*- Definitions -------------------------------------------------------------*/
#define ZERO_POINT             0x80

#define GRID_CENTER_X          160
#define GRID_CENTER_Y          120
#define GRID_LEFT              10
#define GRID_RIGHT             310
#define GRID_TOP               20
#define GRID_BOTTOM            220
#define GRID_WIDTH             300
#define GRID_HEIGHT            200
#define GRID_DIV_PX            25
#define GRID_DIVS_H            12
#define GRID_DIVS_V            10

#define STATUS_LINE_Y          223
#define STATUS_LINE_HEIGHT     16

#define MINIVIEW_WIDTH         160

#define CALIB_AREA_LEFT        140
#define CALIB_AREA_WIDTH       (LCD_WIDTH - CALIB_AREA_LEFT)

#define MAX_SAMPLE_RATE_LIMIT  13

#define TOAST_TIMEOUT          1500
#define TOAST_COLOR            LCD_COLOR(255, 255, 0)

#define WAIT_STATE_HOLDOFF     100

#define BG_COLOR               LCD_COLOR(0, 0, 0)
#define TRACE_COLOR            LCD_COLOR(255, 255, 0)
#define TRACE_FILLED_COLOR     LCD_COLOR(0, 255, 0)
#define TRACE_CLIP_COLOR       LCD_COLOR(255, 0, 0)
#define TRACE_INVALID_COLOR    LCD_COLOR(255, 0, 0)
#define GRID_BG_COLOR          LCD_COLOR(0, 0, 0)
#define GRID_FG_COLOR          LCD_COLOR(200, 200, 200)
#define MV_FRAME_COLOR         LCD_COLOR(230, 230, 230)

#define HSCALE_COLOR           LCD_COLOR(255, 255, 255)
#define HPOS_COLOR             LCD_COLOR(255, 180, 50)
#define VSCALE_COLOR           LCD_COLOR(255, 255, 0)

#define TRIGGER_LEVEL_COLOR    LCD_COLOR(50, 255, 255)
#define TRIGGER_MODE_COLOR     LCD_COLOR(0, 255, 0)

#define MEASURE_MODE_COLOR     LCD_COLOR(50, 255, 255)
#define MEASURE_VOLTAGE_COLOR  LCD_COLOR(255, 255, 0)
#define MEASURE_FREQ_COLOR     LCD_COLOR(255, 255, 255)

#define CAPTURE_STOP_COLOR     LCD_COLOR(255, 0, 0)
#define CAPTURE_WAIT_COLOR     LCD_COLOR(255, 180, 50)
#define CAPTURE_TRIG_COLOR     LCD_COLOR(0, 255, 0)

#define SR_LIMIT_COLOR         LCD_COLOR(250, 50, 50)
#define SR_COLOR               LCD_COLOR(0, 230, 0)

#define MIN_TRIGGER_LEVEL      -100 // px
#define MAX_TRIGGER_LEVEL       100 // px

#define MIN_HORIZONTAL_POSITION -2000000000 // 2 seconds
#define MAX_HORIZONTAL_POSITION  2000000000 // 2 seconds

#define MIN_VERTICAL_POSITION  (-10 * GRID_DIV_PX)
#define MAX_VERTICAL_POSITION  ( 10 * GRID_DIV_PX)

#define MEASURE_UPDATE_TIMEOUT 100

enum
{
  CALIB_ZERO,
  CALIB_DELTA,
  CALIB_SCALE,
  CALIB_OFFSET,
};

/*- Types -------------------------------------------------------------------*/
typedef struct
{
  uint8_t  min[GRID_WIDTH];
  uint8_t  max[GRID_WIDTH];
  uint8_t  flags[GRID_WIDTH];
} DisplayBuffer;

/*- Constants ---------------------------------------------------------------*/
static const char *hs_str[HS_COUNT] =
{
                            " 50\x01ns", // ns
  "100\x01ns", "200\x01ns", "500\x01ns",
  "  1\x01us", "  2\x01us", "  5\x01us", // us
  " 10\x01us", " 20\x01us", " 50\x01us",
  "100\x01us", "200\x01us", "500\x01us",
  "  1\x01ms", "  2\x01ms", "  5\x01ms", // ms
  " 10\x01ms", " 20\x01ms", " 50\x01ms",
  "100\x01ms", "200\x01ms", "500\x01ms",
};

static const int hs_div_value[HS_COUNT] =
{
  50, 100, 200, 500, // ns
  1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, // us
  1000000, 2000000, 5000000, 10000000, 20000000, 50000000, 100000000, 200000000, 500000000, // ms
};

static const int hs_px_value[HS_COUNT] = // in ns
{
  2, 4, 8, 20, // ns
  40, 80, 200, 400, 800, 2000, 4000, 8000, 20000, // us
  40000, 80000, 200000, 400000, 800000, 2000000, 4000000, 8000000, 20000000, // ms
};

static const char *vs_str[VS_COUNT] =
{
  " 50\x01mV", "100\x01mV", "200\x01mV", "500\x01mV", "  1\x01V ", "  2\x01V ", "  5\x01V ", " 10\x01V ",
};

static const int vs_px_value[VS_COUNT] =
{
  2, 4, 8, 20, 40, 80, 200, 400,
};

/*- Variables ---------------------------------------------------------------*/
static uint16_t *g_grid_data[GRID_WIDTH];
static uint16_t g_grid_column_0[240];
static uint16_t g_grid_column_1[240];
static uint16_t g_grid_column_2[240];
static uint16_t g_grid_column_3[240];
static uint16_t g_grid_column_4[240];

static DataBuffer g_data_buffer;
static DisplayBuffer g_display_buffer;

static int g_trace_column = (GRID_WIDTH-1);

static bool g_toast_active = false;
static int g_toast_timer = TIMER_DISABLE;

static int g_state = -1;
static int g_state_timer = TIMER_DISABLE;

static bool g_calibration_mode = false;
static bool g_calibration_dual_channel = false;
static int g_calibration_parameter = CALIB_ZERO;

static int g_measure_timer = TIMER_DISABLE;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static void grid_init(void)
{
  // Setup column data
  for (int y = 0; y < 199; y++)
    g_grid_column_0[y] = GRID_BG_COLOR;

  for (int y = 0; y < 199; y++)
    g_grid_column_1[y] = (4 == (y % 5)) ? GRID_FG_COLOR : GRID_BG_COLOR;

  for (int y = 0; y < 199; y++)
    g_grid_column_2[y] = (24 == (y % 25)) ? GRID_FG_COLOR : GRID_BG_COLOR;
  g_grid_column_2[0]   = GRID_FG_COLOR;
  g_grid_column_2[1]   = GRID_FG_COLOR;
  g_grid_column_2[98]  = GRID_FG_COLOR;
  g_grid_column_2[100] = GRID_FG_COLOR;
  g_grid_column_2[197] = GRID_FG_COLOR;
  g_grid_column_2[198] = GRID_FG_COLOR;

  for (int y = 0; y < 199; y++)
    g_grid_column_3[y] = (4 == (y % 5)) ? GRID_FG_COLOR : GRID_BG_COLOR;
  g_grid_column_3[0]   = GRID_FG_COLOR;
  g_grid_column_3[1]   = GRID_FG_COLOR;
  g_grid_column_3[2]   = GRID_FG_COLOR;
  g_grid_column_3[98]  = GRID_FG_COLOR;
  g_grid_column_3[100] = GRID_FG_COLOR;
  g_grid_column_3[196] = GRID_FG_COLOR;
  g_grid_column_3[197] = GRID_FG_COLOR;
  g_grid_column_3[198] = GRID_FG_COLOR;

  for (int y = 0; y < 199; y++)
    g_grid_column_4[y] = (24 == (y % 25)) ? GRID_FG_COLOR : GRID_BG_COLOR;

  // Assign columns
  for (int x = 0; x < 299; x++)
    g_grid_data[x] = g_grid_column_0;

  for (int x = 4; x < 299; x += 5)
    g_grid_data[x] = g_grid_column_2;

  for (int x = 24; x < 299; x += 25)
    g_grid_data[x] = g_grid_column_3;

  g_grid_data[0]   = g_grid_column_1;
  g_grid_data[1]   = g_grid_column_1;
  g_grid_data[2]   = g_grid_column_4;
  g_grid_data[148] = g_grid_column_1;
  g_grid_data[150] = g_grid_column_1;
  g_grid_data[296] = g_grid_column_4;
  g_grid_data[297] = g_grid_column_1;
  g_grid_data[298] = g_grid_column_1;
}

//-----------------------------------------------------------------------------
static void toast_show(void)
{
  if (!g_toast_active)
    lcd_fill_rect(GRID_LEFT, GRID_BOTTOM+1, GRID_WIDTH+1, STATUS_LINE_HEIGHT, BG_COLOR);

  lcd_set_color(BG_COLOR, TOAST_COLOR);
  g_toast_active = true;
  g_toast_timer = TOAST_TIMEOUT;
}

//-----------------------------------------------------------------------------
static bool trace_ready(void)
{
  return (g_trace_column == (GRID_WIDTH-1));
}

//-----------------------------------------------------------------------------
static void redraw_trace(void)
{
  if (trace_ready())
    g_trace_column = 0;
}

//-----------------------------------------------------------------------------
static void update_column_from_image(int index, uint16_t *column, int x, int y, const Image *image)
{
  int image_col = index - x + image->ox + 1;

  if (image_col < 0 || image_col > (image->width-1))
    return;

  for (int i = 0; i < image->height; i++)
  {
    int color = image->data[i * image->width + image_col];

    if (color)
      column[y + i] = color;
  }
}

//-----------------------------------------------------------------------------
static void update_from_display_buffer(uint16_t *column, DisplayBuffer *db)
{
  bool clip_h = db->flags[g_trace_column] & SAMPLE_FLAG_CLIP_H;
  bool clip_l = db->flags[g_trace_column] & SAMPLE_FLAG_CLIP_L;
  int color;

  if (db->flags[g_trace_column] & SAMPLE_FLAG_VALID)
  {
    if (clip_h || clip_l)
      color = TRACE_CLIP_COLOR;
    else if (db->flags[g_trace_column] & SAMPLE_FLAG_FILLED)
      color = TRACE_FILLED_COLOR;
    else
      color =  TRACE_COLOR;

    for (int y = db->min[g_trace_column]; y <= db->max[g_trace_column]; y++)
      column[y] = color;
  }
  else
  {
    column[GRID_HEIGHT/2-1] = TRACE_INVALID_COLOR;
  }

  if (clip_h)
  {
    for (int i = 0; i < 3; i++)
      column[i] = TRACE_CLIP_COLOR;
  }

  if (clip_l)
  {
    for (int i = 0; i < 3; i++)
      column[GRID_HEIGHT-2 - i] = TRACE_CLIP_COLOR;
  }
}

//-----------------------------------------------------------------------------
static void draw_trace(void)
{
  uint16_t column[GRID_HEIGHT];

  if (trace_ready())
    return;

  for (int i = 0; i < GRID_HEIGHT; i++)
    column[i] = g_grid_data[g_trace_column][i];

  update_from_display_buffer(column, &g_display_buffer);

  if (config.horizontal_position_px < -(GRID_WIDTH/2-1))
  {
    update_column_from_image(g_trace_column, column, GRID_WIDTH-1, 1, &image_trigger_offset_right);
  }
  else if (config.horizontal_position_px > (GRID_WIDTH/2-1))
  {
    update_column_from_image(g_trace_column, column, 1, 1, &image_trigger_offset_left);
  }
  else
  {
    int pos = GRID_WIDTH/2 - config.horizontal_position_px;
    update_column_from_image(g_trace_column, column, pos, 0, &image_trigger_offset);
  }

  lcd_draw_buf(GRID_LEFT+1 + g_trace_column, GRID_TOP+1, 1, GRID_HEIGHT-1, column);

  g_trace_column++;
}

//-----------------------------------------------------------------------------
static void draw_grid_frame(void)
{
  lcd_vline(10, 20, 220, GRID_FG_COLOR);
  lcd_vline(310, 20, 220, GRID_FG_COLOR);
  lcd_hline(10, 310, 20, GRID_FG_COLOR);
  lcd_hline(10, 310, 220, GRID_FG_COLOR);
}

//-----------------------------------------------------------------------------
static void draw_ac_dc(void)
{
  if (g_toast_active)
    return;

  lcd_draw_image(54, STATUS_LINE_Y, config.ac_coupling ? &image_ac : &image_dc);
}

//-----------------------------------------------------------------------------
static void draw_horizontal_scale(void)
{
  if (g_toast_active)
    return;

  lcd_set_color(BG_COLOR, HSCALE_COLOR);
  lcd_puts(82, STATUS_LINE_Y, hs_str[config.horizontal_scale]);
}

//-----------------------------------------------------------------------------
static void draw_horizontal_position(void)
{
  char *str;

  if (g_toast_active || g_calibration_mode || config.measure_display)
    return;

  str = format_time(config.horizontal_position, true);
  lcd_set_color(BG_COLOR, HPOS_COLOR);
  lcd_puts(236, STATUS_LINE_Y, str);
}

//-----------------------------------------------------------------------------
static void draw_vertical_position(bool toast)
{
  int y = GRID_CENTER_Y - config.vertical_position;

  lcd_fill_rect(0, GRID_TOP-4, 9, GRID_HEIGHT+9, BG_COLOR);

  if (y < GRID_TOP)
    lcd_draw_image(GRID_LEFT-5, GRID_TOP, &image_reference_level_up);
  else if (y > GRID_BOTTOM)
    lcd_draw_image(GRID_LEFT-5, GRID_BOTTOM, &image_reference_level_down);
  else
    lcd_draw_image(GRID_LEFT-2, y, &image_reference_level);

  if (toast)
  {
    char *str;

    toast_show();

    lcd_puts(GRID_LEFT, STATUS_LINE_Y, "Vertical position");

    str = format_divisions((config.vertical_position * 100) / GRID_DIV_PX, true);
    lcd_puts(GRID_LEFT + 140, STATUS_LINE_Y, str);

    str = format_voltage(config.vertical_position_mv, true);
    lcd_puts(GRID_LEFT + 226, STATUS_LINE_Y, str);
  }
}

//-----------------------------------------------------------------------------
static void draw_vertical_scale(void)
{
  if (g_toast_active)
    return;

  lcd_set_color(BG_COLOR, VSCALE_COLOR);
  lcd_puts(10, STATUS_LINE_Y, vs_str[config.vertical_scale]);
}

//-----------------------------------------------------------------------------
static void draw_trigger_level(void)
{
  char *str;

  lcd_fill_rect(GRID_RIGHT+2, GRID_TOP-4, 8, GRID_HEIGHT+9, BG_COLOR);
  lcd_draw_image(GRID_RIGHT+2, GRID_CENTER_Y - config.trigger_level,
      &image_trigger_level);

  if (g_toast_active || g_calibration_mode || config.measure_display)
    return;

  str = format_voltage(config.trigger_level_mv - config.vertical_position_mv, true);
  lcd_set_color(BG_COLOR, TRIGGER_LEVEL_COLOR);
  lcd_puts(148, STATUS_LINE_Y, str);
}

//-----------------------------------------------------------------------------
static void draw_trigger_edge(void)
{
  if (g_toast_active || g_calibration_mode || config.measure_display)
    return;

  if (TRIGGER_EDGE_RISE == config.trigger_edge)
    lcd_draw_image(140, STATUS_LINE_Y, &image_trigger_edge_rise);
  else if (TRIGGER_EDGE_FALL == config.trigger_edge)
    lcd_draw_image(140, STATUS_LINE_Y, &image_trigger_edge_fall);
  else
    lcd_draw_image(140, STATUS_LINE_Y, &image_trigger_edge_both);
}

//-----------------------------------------------------------------------------
static void draw_trigger_mode(void)
{
  char *str;

  if (TRIGGER_MODE_AUTO == config.trigger_mode)
    str = "AUTO";
  else if (TRIGGER_MODE_NORMAL == config.trigger_mode)
    str = "NORM";
  else
    str = "SNGL";

  lcd_set_color(BG_COLOR, TRIGGER_MODE_COLOR);
  lcd_puts(10, 4, str);
}

//-----------------------------------------------------------------------------
static void draw_measure(void)
{
  int vmin, vmax, vpp;
  char *str;

  if (g_toast_active || g_calibration_mode || !config.measure_display)
    return;

  vmin = g_data_buffer.min_value;
  vmax = g_data_buffer.max_value;

  if (vmax > 0 && vmin < 0)
    vpp = vmax - vmin;
  else
    vpp = vmax - vmin;

  lcd_set_color(BG_COLOR, MEASURE_MODE_COLOR);
  lcd_putc(140, STATUS_LINE_Y, 'M');

  str = format_voltage(vpp, false);
  lcd_set_color(BG_COLOR, MEASURE_VOLTAGE_COLOR);
  lcd_puts(148, STATUS_LINE_Y, str);

  str = format_frequency(g_data_buffer.frequency);
  lcd_set_color(BG_COLOR, MEASURE_FREQ_COLOR);
  lcd_puts(236, STATUS_LINE_Y, str);
}

//-----------------------------------------------------------------------------
static void draw_capture_state(void)
{
  int state = capture_get_state();
  int color = BG_COLOR;
  char *str = "";

  if (g_state == state)
    return;

  if (CAPTURE_STATE_STOP == state)
  {
    color = CAPTURE_STOP_COLOR;
    str = "STOP";
  }
  else if (CAPTURE_STATE_WAIT == state)
  {
    color = CAPTURE_WAIT_COLOR;
    str = "WAIT";
  }
  else if (CAPTURE_STATE_TRIG == state)
  {
    color = CAPTURE_TRIG_COLOR;
    str = "TRIG";
  }

  lcd_set_color(BG_COLOR, color);
  lcd_puts(46, 4, str);

  g_state = state;
}

//-----------------------------------------------------------------------------
static void draw_miniview(int trigger_offset, int window_offset, int window_width)
{
  static const uint8_t wave_pattern[8] = { 1, 0, 0, 1, 2, 3, 3, 2 };
  uint16_t buf[8];

  for (int x = -MINIVIEW_WIDTH/2+1; x < MINIVIEW_WIDTH/2; x++)
  {
    bool inside = ((x > window_offset) && (x < (window_offset + window_width)));
    bool edge = ((x == window_offset) || (x == (window_offset + window_width - 1)));

    if (edge)
    {
      for (int i = 0; i < 8; i++)
        buf[i] = MV_FRAME_COLOR;
    }
    else
    {
      for (int i = 0; i < 8; i++)
        buf[i] = BG_COLOR;

      buf[2 + wave_pattern[x % sizeof(wave_pattern)]] = inside ? TRACE_COLOR : MV_FRAME_COLOR;

      if (inside)
      {
        buf[0] = MV_FRAME_COLOR;
        buf[7] = MV_FRAME_COLOR;
      }
    }

    lcd_draw_buf(GRID_CENTER_X + x, 7, 1, 8, buf);
  }

#define LEFT   (GRID_CENTER_X - MINIVIEW_WIDTH/2)
#define RIGHT  (GRID_CENTER_X + MINIVIEW_WIDTH/2)

  lcd_fill_rect(LEFT - image_trigger_mv.width/2, 1, MINIVIEW_WIDTH + image_trigger_mv.width,
      image_trigger_mv.height, BG_COLOR);
  lcd_draw_image(GRID_CENTER_X + trigger_offset, 5, &image_trigger_mv);

  lcd_vline(LEFT, 6, 15, MV_FRAME_COLOR);
  lcd_hline(LEFT, LEFT+2, 6, MV_FRAME_COLOR);
  lcd_hline(LEFT, LEFT+2, 15, MV_FRAME_COLOR);

  lcd_vline(RIGHT, 6, 15, MV_FRAME_COLOR);
  lcd_hline(RIGHT-2, RIGHT, 6, MV_FRAME_COLOR);
  lcd_hline(RIGHT-2, RIGHT, 15, MV_FRAME_COLOR);

#undef LEFT
#undef RIGHT
}

//-----------------------------------------------------------------------------
static void draw_sample_rates(int sample_rate_limit, int sample_rate)
{
  char *str;

  lcd_set_font(FONT_SMALL);

  str = format_sps(sample_rate_limit);
  lcd_set_color(BG_COLOR, SR_LIMIT_COLOR);
  lcd_puts(252, 2, str);

  str = format_sps(sample_rate);
  lcd_set_color(BG_COLOR, SR_COLOR);
  lcd_puts(252, 10, str);

  lcd_set_font(FONT_LARGE);
}

//-----------------------------------------------------------------------------
static void update_sample_rate(void)
{
  int64_t hp_abs = (config.horizontal_position < 0) ? -config.horizontal_position : config.horizontal_position;
  int64_t window_time = (int64_t)hs_div_value[config.horizontal_scale] * GRID_DIVS_H;
  int64_t period = BASE_SAMPLE_PERIOD;
  int64_t trigger_margin, trigger_offset;
  int64_t buffer_time, required_time;
  int64_t window_offset;
  int64_t denom;
  int sample_rate = BASE_SAMPLE_RATE;
  int sample_rate_limit;
  int trigger_offset_px, window_offset_px, window_width_px;
  int sr_divider = config.sample_rate_limit;

  for (int i = 0; i < config.sample_rate_limit; i++)
  {
    period *= 2;
    sample_rate /= 2;
  }

  sample_rate_limit = sample_rate;

  while (1)
  {
    trigger_margin = period * TRIGGER_MARGIN_SAMPLES;
    required_time = trigger_margin + hp_abs + window_time/2;
    buffer_time = (int64_t)CAPTURE_BUFFER_SIZE * period;

    if (required_time < window_time)
      required_time = window_time;

    if (required_time < buffer_time)
      break;

    sr_divider++;
    period *= 2;
    sample_rate /= 2;
  }

  g_calibration_dual_channel = (sr_divider == 0);

  denom = buffer_time - window_time/2 - trigger_margin;
  trigger_offset = -config.horizontal_position * (buffer_time/2 - trigger_margin) / denom;
  window_offset = trigger_offset + config.horizontal_position;

  capture_set_horizontal_parameters(sr_divider, CAPTURE_BUFFER_SIZE/2 + trigger_offset / period);

  denom = period * CAPTURE_BUFFER_SIZE;

  trigger_offset_px = (trigger_offset * MINIVIEW_WIDTH) / denom;
  window_offset_px  = ((window_offset - window_time/2) * MINIVIEW_WIDTH) / denom;
  window_width_px   = (window_time * MINIVIEW_WIDTH) / denom;

  if (window_offset_px == 0)
    window_offset_px = -1;

  if (window_width_px < 3)
    window_width_px = 3;

  draw_miniview(trigger_offset_px, window_offset_px, window_width_px);
  draw_sample_rates(sample_rate_limit, sample_rate);
}

//-----------------------------------------------------------------------------
static int clip_for_display(int value)
{
  value = GRID_HEIGHT/2-1 - value;

  if (value > (GRID_HEIGHT-2))
    value = (GRID_HEIGHT-2);
  else if (value < 0)
    value = 0;

  return value;
}

//---------------------------------------------------------------------
static void close_gaps(DisplayBuffer *db)
{
  for (int i = 0; i < GRID_WIDTH-1; i++)
  {
    if ((db->flags[i] & SAMPLE_FLAG_VALID) && (db->flags[i+1] & SAMPLE_FLAG_VALID))
    {
      if (db->max[i] < (db->min[i+1]-1))
      {
        int avg = (db->max[i] + db->min[i+1]) / 2;
        db->max[i] = avg;
        db->min[i+1] = avg+1;
      }
      else if (db->min[i] > (db->max[i+1] + 1))
      {
        int avg = (db->min[i] + db->max[i+1]) / 2;
        db->min[i] = avg;
        db->max[i+1] = avg-1;
      }
    }
  }
}

//-----------------------------------------------------------------------------
static void update_display(void)
{
  int scale = vs_px_value[config.vertical_scale];

  g_data_buffer.size = GRID_WIDTH;
  capture_get_data(&g_data_buffer);

  for (int i = 0; i < GRID_WIDTH; i++)
  {
    int min = (g_data_buffer.min[i] - g_data_buffer.vertical_position) / scale + config.vertical_position;
    int max = (g_data_buffer.max[i] - g_data_buffer.vertical_position) / scale + config.vertical_position;

    g_display_buffer.min[i]   = clip_for_display(max);
    g_display_buffer.max[i]   = clip_for_display(min);
    g_display_buffer.flags[i] = g_data_buffer.flags[i];
  }

  close_gaps(&g_display_buffer);
  redraw_trace();
}

//-----------------------------------------------------------------------------
static void change_horizontal_scale(int delta)
{
  if ((delta < 0 && config.horizontal_scale == 0) ||
      (delta > 0 && config.horizontal_scale == HS_LAST))
    return;

  config.horizontal_scale += delta;
  config.horizontal_position_px = config.horizontal_position / hs_px_value[config.horizontal_scale];
  config.horizontal_period = hs_px_value[config.horizontal_scale];

  draw_horizontal_scale();
  draw_horizontal_position();
  update_sample_rate();
  update_display();
}

//-----------------------------------------------------------------------------
static void change_horizontal_position(int delta)
{
  int div = hs_px_value[config.horizontal_scale];

  if ((delta < 0 && config.horizontal_position <= MIN_HORIZONTAL_POSITION) ||
      (delta > 0 && config.horizontal_position >= MAX_HORIZONTAL_POSITION))
    return;

  config.horizontal_position_px = config.horizontal_position / div + delta;
  config.horizontal_position = (int64_t)config.horizontal_position_px * div;

  draw_horizontal_position();
  update_sample_rate();
  update_display();
}

//-----------------------------------------------------------------------------
static void change_vertical_scale(int delta)
{
  if ((delta < 0 && config.vertical_scale == 0) ||
      (delta > 0 && config.vertical_scale == VS_LAST))
    return;

  config.vertical_scale += delta;
  config.vertical_mult = config.calib_vs_mult[config.vertical_scale];
  config.vertical_position_mv = config.vertical_position * vs_px_value[config.vertical_scale];

  config.trigger_level_mv = config.trigger_level * vs_px_value[config.vertical_scale];

  capture_set_vertical_parameters();
  capture_set_trigger_level(config.trigger_level_mv);
  draw_vertical_scale();
  draw_trigger_level();
  update_display();
}

//-----------------------------------------------------------------------------
static void change_vertical_position(int delta)
{
  if ((delta < 0 && config.vertical_position <= MIN_VERTICAL_POSITION) ||
      (delta > 0 && config.vertical_position >= MAX_VERTICAL_POSITION))
    return;

  config.vertical_position += delta;
  config.vertical_position_mv = config.vertical_position * vs_px_value[config.vertical_scale];

  capture_set_vertical_parameters();
  draw_vertical_position(true);
  draw_trigger_level();
  update_display();
}

//-----------------------------------------------------------------------------
static void change_trigger_level(int delta)
{
  if ((delta < 0 && config.trigger_level == MIN_TRIGGER_LEVEL) ||
      (delta > 0 && config.trigger_level == MAX_TRIGGER_LEVEL))
    return;

  config.trigger_level += delta;
  config.trigger_level_mv = config.trigger_level * vs_px_value[config.vertical_scale];

  capture_set_trigger_level(config.trigger_level_mv);
  draw_trigger_level();
}

//-----------------------------------------------------------------------------
static void change_sample_rate_limit(int delta)
{
  if ((delta < 0 && config.sample_rate_limit == 0) ||
      (delta > 0 && config.sample_rate_limit == MAX_SAMPLE_RATE_LIMIT))
    return;

  config.sample_rate_limit += delta;

  update_sample_rate();
}

//-----------------------------------------------------------------------------
static void change_calibration_value(int delta, bool shift)
{
  if (shift)
  {
    if (g_calibration_parameter == CALIB_OFFSET)
      g_calibration_parameter = CALIB_ZERO;
    else
      g_calibration_parameter++;

    lcd_fill_rect(CALIB_AREA_LEFT, GRID_BOTTOM+1, CALIB_AREA_WIDTH, STATUS_LINE_HEIGHT, BG_COLOR);

    return;
  }

  if (g_calibration_parameter == CALIB_ZERO)
  {
    config.calib_dac_zero += delta;

    if (config.calib_dac_zero < 1900)
      config.calib_dac_zero = 1900;
    else if (config.calib_dac_zero > 2200)
      config.calib_dac_zero = 2200;
  }
  else if (g_calibration_parameter == CALIB_DELTA)
  {
    config.calib_channel_delta += delta;

    if (config.calib_channel_delta < -64)
      config.calib_channel_delta = -64;
    else if (config.calib_channel_delta > 64)
      config.calib_channel_delta = 64;
  }
  else if (g_calibration_parameter == CALIB_SCALE)
  {
    config.calib_vs_mult[config.vertical_scale] += delta;

    if (config.calib_vs_mult[config.vertical_scale] < 0)
      config.calib_vs_mult[config.vertical_scale] = 0;
    else if (config.calib_vs_mult[config.vertical_scale] > 4000000)
      config.calib_vs_mult[config.vertical_scale] = 4000000;
  }
  else if (g_calibration_parameter == CALIB_OFFSET)
  {
    config.calib_dac_mult[config.vertical_scale] += delta;

    if (config.calib_dac_mult[config.vertical_scale] < 0)
      config.calib_dac_mult[config.vertical_scale] = 0;
    else if (config.calib_dac_mult[config.vertical_scale] > 100000)
      config.calib_dac_mult[config.vertical_scale] = 100000;
  }

  capture_set_vertical_parameters();
}

//-----------------------------------------------------------------------------
static void draw_calibration_info(void)
{
  static const char *labels[] = { "Z", "D", "S", "O" };
  char *str;

  lcd_set_color(BG_COLOR, LCD_WHITE_COLOR);

  if (!g_toast_active)
    lcd_puts(CALIB_AREA_LEFT, STATUS_LINE_Y, labels[g_calibration_parameter]);

  g_data_buffer.size = GRID_WIDTH;

  if (g_calibration_parameter == CALIB_ZERO || g_calibration_parameter == CALIB_DELTA)
  {
    if ((g_calibration_parameter == CALIB_ZERO) == g_calibration_dual_channel)
      lcd_set_color(BG_COLOR, LCD_RED_COLOR);

    capture_get_raw_data(g_data_buffer.max, GRID_WIDTH/2);

    if (g_calibration_dual_channel)
    {
      for (int i = 1; i < GRID_WIDTH/2; i += 2)
        g_data_buffer.max[i] = rbit8(g_data_buffer.max[i]) + config.calib_channel_delta;
    }

    if (!g_toast_active)
    {
      str = format_raw_data(g_data_buffer.max, 4);
      lcd_puts(CALIB_AREA_LEFT + 24, STATUS_LINE_Y, str);
    }

    #define PIXEL_SIZE 15
    for (int i = 0; i < GRID_WIDTH/2; i++)
    {
      g_data_buffer.max[i] -= ZERO_POINT;
      g_data_buffer.min[i] = g_data_buffer.max[i];

      if (0 == i % PIXEL_SIZE)
      {
        g_data_buffer.max[GRID_WIDTH/2 + i] = 0;
        g_data_buffer.min[GRID_WIDTH/2 + i] = 0;
      }
      else
      {
        int value = g_data_buffer.max[i / PIXEL_SIZE] * PIXEL_SIZE;
        g_data_buffer.max[GRID_WIDTH/2 + i] = value + PIXEL_SIZE/2;
        g_data_buffer.min[GRID_WIDTH/2 + i] = value - PIXEL_SIZE/2;
      }
    }

    for (int i = 0; i < GRID_WIDTH; i++)
    {
      g_display_buffer.min[i]   = clip_for_display(g_data_buffer.max[i]);
      g_display_buffer.max[i]   = clip_for_display(g_data_buffer.min[i]);
      g_display_buffer.flags[i] = SAMPLE_FLAG_VALID;
    }

    redraw_trace();
  }
  else // if (g_calibration_parameter == CALIB_SCALE || g_calibration_parameter == CALIB_OFFSET)
  {
    update_display();

    int vmin = g_data_buffer.min_value;
    int vmax = g_data_buffer.max_value;
    int vpos = g_data_buffer.vertical_position;

    if (!g_toast_active)
    {
      str = format_voltage(vmin - vpos, true);
      lcd_puts(CALIB_AREA_LEFT + 12, STATUS_LINE_Y, str);

      str = format_voltage(vmax - vpos, true);
      lcd_puts(CALIB_AREA_LEFT + 96, STATUS_LINE_Y, str);
    }
  }
}

//-----------------------------------------------------------------------------
static void draw_status_line(void)
{
  lcd_fill_rect(GRID_LEFT, GRID_BOTTOM+1, GRID_WIDTH+1, STATUS_LINE_HEIGHT, BG_COLOR);

  draw_vertical_scale();
  draw_ac_dc();
  draw_horizontal_scale();
  draw_horizontal_position();
  draw_trigger_level();
  draw_trigger_edge();
  draw_measure();
}

//-----------------------------------------------------------------------------
void scope_buttons_handler(int buttons)
{
  bool shift  = (buttons & BTN_SHIFT);
  bool repeat = (buttons & BTN_REPEAT);

  if ((buttons & BTN_UP) && (buttons & BTN_DOWN))
  {
    config.vertical_position = 0;
    config.vertical_position_mv = 0;

    capture_set_vertical_parameters();
    draw_vertical_position(true);
    update_display();
  }
  else if (buttons & BTN_UP)
  {
    if (shift)
      change_vertical_scale(1);
    else
      change_vertical_position(1);
  }
  else if (buttons & BTN_DOWN)
  {
    if (shift)
      change_vertical_scale(-1);
    else
      change_vertical_position(-1);
  }

  else if ((buttons & BTN_LEFT) && (buttons & BTN_RIGHT))
  {
    config.horizontal_position = 0;
    config.horizontal_position_px = 0;

    draw_horizontal_position();
    update_sample_rate();
    update_display();
  }
  else if (buttons & BTN_LEFT)
  {
    if (shift)
      change_horizontal_scale(-1);
    else
      change_horizontal_position(1);
  }
  else if (buttons & BTN_RIGHT)
  {
    if (shift)
      change_horizontal_scale(1);
    else
      change_horizontal_position(-1);
  }

  else if (buttons & BTN_TRIG)
  {
    if (repeat || g_calibration_mode)
      return;

    if (config.trigger_mode == TRIGGER_MODE_SINGLE)
      config.trigger_mode = TRIGGER_MODE_AUTO;
    else
      config.trigger_mode++;

    capture_set_trigger_mode(config.trigger_mode);
    capture_start();
    draw_trigger_mode();
  }
  else if (buttons & BTN_EDGE)
  {
    if (repeat || g_calibration_mode)
      return;

    if (config.trigger_edge == TRIGGER_EDGE_BOTH)
      config.trigger_edge = TRIGGER_EDGE_RISE;
    else
      config.trigger_edge++;

    capture_set_trigger_edge(config.trigger_edge);
    draw_trigger_edge();
  }
  else if (buttons & BTN_TRIG_UP)
  {
    if (g_calibration_mode)
      change_calibration_value(1, shift);
    else if (shift)
      change_sample_rate_limit(1);
    else
      change_trigger_level(1);
  }
  else if (buttons & BTN_TRIG_DOWN)
  {
    if (g_calibration_mode)
      change_calibration_value(-1, shift);
    else if (shift)
      change_sample_rate_limit(-1);
    else
      change_trigger_level(-1);
  }

  else if (buttons & BTN_AC_DC)
  {
    if (repeat)
      return;

    config.ac_coupling = !config.ac_coupling;

    capture_set_vertical_parameters();
    draw_ac_dc();
  }

  else if (buttons & BTN_MODE)
  {
    config.measure_display = !config.measure_display;
    g_measure_timer = config.measure_display ? MEASURE_UPDATE_TIMEOUT : TIMER_DISABLE;
    draw_status_line();
  }

  else if (buttons & BTN_SAVE)
  {
  }

  else if (buttons & BTN_STOP)
  {
    if (capture_get_state() == CAPTURE_STATE_STOP)
      capture_start();
    else
      capture_stop();
  }
}

//-----------------------------------------------------------------------------
void scope_init(bool calibration_mode)
{
  g_calibration_mode = calibration_mode;

  config.horizontal_period = hs_px_value[config.horizontal_scale];
  config.vertical_mult = config.calib_vs_mult[config.vertical_scale];

  grid_init();
  draw_grid_frame();
  draw_vertical_position(false);
  draw_trigger_mode();
  draw_capture_state();
  draw_status_line();
  redraw_trace();

  if (g_calibration_mode)
  {
    capture_set_trigger_edge(TRIGGER_EDGE_RISE);
    capture_set_trigger_mode(TRIGGER_MODE_AUTO);
    capture_set_trigger_level(0);
  }
  else
  {
    capture_set_trigger_edge(config.trigger_edge);
    capture_set_trigger_mode(config.trigger_mode);
    capture_set_trigger_level(config.trigger_level_mv);
  }

  timer_add(&g_toast_timer);
  timer_add(&g_state_timer);
  timer_add(&g_measure_timer);

  g_measure_timer = config.measure_display ? MEASURE_UPDATE_TIMEOUT : TIMER_DISABLE;

  update_sample_rate();
  capture_set_vertical_parameters();
  capture_start();
}

//-----------------------------------------------------------------------------
void scope_task(void)
{
  if (trace_ready())
  {
    if (capture_buffer_updated())
    {
      if (g_calibration_mode)
        draw_calibration_info();
      else
        update_display();
    }
  }

  draw_trace();

  if (CAPTURE_STATE_WAIT == capture_get_state())
  {
    if (g_state_timer == TIMER_DISABLE)
    {
      g_state_timer = WAIT_STATE_HOLDOFF;
    }
    else if (g_state_timer == 0)
    {
      g_state_timer = TIMER_DISABLE;
      draw_capture_state();
    }
  }
  else
  {
    g_state_timer = TIMER_DISABLE;
    draw_capture_state();
  }

  if (g_toast_active)
  {
    if (g_toast_timer == 0)
    {
      g_toast_timer = TIMER_DISABLE;
      g_toast_active = false;
      draw_status_line();
    }
  }

  if (config.measure_display)
  {
    if (g_measure_timer == 0)
    {
      g_measure_timer = MEASURE_UPDATE_TIMEOUT;
      draw_measure();
    }
  }
}

