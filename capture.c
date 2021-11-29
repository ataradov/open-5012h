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
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include "gd32f4xx.h"
#include "hal_gpio.h"
#include "utils.h"
#include "buffer.h"
#include "common.h"
#include "config.h"
#include "trigger.h"
#include "capture.h"

/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(CLK_A,    A, 8)
HAL_GPIO_PIN(CLK_B,    A, 9)

HAL_GPIO_PIN(DAC,      A, 4)

HAL_GPIO_PIN(Q0,       B, 9)
HAL_GPIO_PIN(Q1,       B, 8)
HAL_GPIO_PIN(Q2,       B, 7)
HAL_GPIO_PIN(Q3,       C, 12)
HAL_GPIO_PIN(Q4,       C, 11)
HAL_GPIO_PIN(Q5,       C, 10)
HAL_GPIO_PIN(Q6,       A, 15)
HAL_GPIO_PIN(AC_DC,    C, 15)

#define AUTO_MODE_COUNT_DIV    10

#define DMA_MAX_BUFFER_SIZE    (16 * 1024)

#define STORAGE_BUFFER_SIZE    (32 * 1024)
#define STORAGE_BUFFER_RATIO   (CAPTURE_BUFFER_SIZE / STORAGE_BUFFER_SIZE)

#define ZERO_POINT             0x80

#define MEASURE_HYSTERESIS     3

/*- Types -------------------------------------------------------------------*/
typedef struct
{
  bool     valid;
  int      period;
  int      offset;
  int      trigger;
  int      vpos;
  int      vs_mult;
  int      size;
  uint8_t  *data;
  int      min_index;
  int      max_index;
} BufferInfo;

/*- Variables ---------------------------------------------------------------*/
// NOTE: Some variables here do not need to be volatile, but I'm keeping them
//       like this, since everything here is either interrupt driven or not
//       critical for performance.
static volatile uint8_t *g_capture_buffer = (uint8_t *)0x20000000;
static volatile int g_dma_buffer_size;
static volatile int g_trigger_mode;
static volatile int g_trigger_edge;
static volatile int g_trigger_level;
static volatile int g_trigger_offset;
static volatile int g_sample_period;
static volatile bool g_dual_channel;
static volatile int g_last_sample = 0;
static int (*g_trigger_find)(uint32_t, uint32_t) = NULL;
static volatile int g_active_buf_ptr;
static volatile int g_next_buf_ptr;
static volatile int g_trigger_ptr;
static volatile int g_count;
static volatile int g_remaining;
static volatile int g_auto_mode_count;
static volatile bool g_auto_mode_stop;
static volatile bool g_triggered;
static volatile bool g_stopped;
static volatile alignas(32) uint8_t g_storage_buffer[STORAGE_BUFFER_SIZE];
static volatile BufferInfo g_capture_buffer_info;
static volatile BufferInfo g_storage_buffer_info;

/*- Prototypes --------------------------------------------------------------*/
static inline int dma_get_count(void);
static inline void dma_wait_count(uint32_t count);

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static void adc_init(void)
{
  HAL_GPIO_CLK_A_alt(1);
  HAL_GPIO_CLK_B_alt(1);

  RCU->AHB1EN_b.DMA1EN = 1;
  RCU->APB2EN_b.TIMER0EN = 1;
  RCU->APB2EN_b.TIMER7EN = 1;

  TIMER0->PSC    = 1;
  TIMER0->CAR    = 1;
  TIMER0->CNT    = 0;
  TIMER0->CH0CV  = 1;
  TIMER0->CH1CV  = 1;
  TIMER0->CHCTL2 = TIMER0_CHCTL2_CH0EN_Msk | TIMER0_CHCTL2_CH1EN_Msk;
  TIMER0->CTL1   = TIMER0_CTL1_ISO0_Msk | TIMER0_CTL1_ISO1_Msk;
  TIMER0->CCHP   = TIMER0_CCHP_POEN_Msk;
  TIMER0->CHCTL0_Output =
      (6 << TIMER0_CHCTL0_Output_CH0COMCTL_Pos) |
      (6 << TIMER0_CHCTL0_Output_CH1COMCTL_Pos);

  TIMER7->PSC      = 1;
  TIMER7->CAR      = 1;
  TIMER7->CNT      = 0;
  TIMER7->CH0CV    = 1;
  TIMER7->CHCTL2   = TIMER7_CHCTL2_CH0EN_Msk;
  TIMER7->DMAINTEN = TIMER7_DMAINTEN_CH0DEN_Msk;

  DMA1->CH2PADDR = (uint32_t)&GPIOD->ISTAT;
}

//-----------------------------------------------------------------------------
static void dac_init(void)
{
  HAL_GPIO_DAC_analog();
  RCU->APB1EN_b.DACEN = 1;
  DAC->CTL = DAC_CTL_DEN0_Msk;
  DAC->DAC0_R12DH = 2048;
}

//-----------------------------------------------------------------------------
static void dac_write(int value)
{
  if (value < 0)
    value = 0;
  else if (value > 4095)
    value = 4095;

  DAC->DAC0_R12DH = value;
}

//-----------------------------------------------------------------------------
static void vertical_scale_init(void)
{
  HAL_GPIO_Q0_out();
  HAL_GPIO_Q0_set();

  HAL_GPIO_Q1_out();
  HAL_GPIO_Q1_set();

  HAL_GPIO_Q2_out();
  HAL_GPIO_Q2_set();

  HAL_GPIO_Q3_out();
  HAL_GPIO_Q3_set();

  HAL_GPIO_Q4_out();
  HAL_GPIO_Q4_set();

  HAL_GPIO_Q5_out();
  HAL_GPIO_Q5_set();

  HAL_GPIO_Q6_out();
  HAL_GPIO_Q6_set();

  HAL_GPIO_AC_DC_out();
  HAL_GPIO_AC_DC_set();
}

//-----------------------------------------------------------------------------
static void set_vertical_scale(void)
{
  static int set_scale = -1;
  int scale = config.vertical_scale;

  if (VS_50_mV == scale)
    scale = VS_100_mV;

  if (scale == set_scale)
    return;

  set_scale = scale;

  HAL_GPIO_Q0_set();
  HAL_GPIO_Q1_set();
  HAL_GPIO_Q2_set();
  HAL_GPIO_Q3_set();
  HAL_GPIO_Q4_set();
  HAL_GPIO_Q5_set();
  HAL_GPIO_Q6_set();

  if (VS_100_mV == scale)
    HAL_GPIO_Q3_clr();
  else if (VS_200_mV == scale)
    HAL_GPIO_Q2_clr();
  else if (VS_500_mV == scale)
    HAL_GPIO_Q0_clr();
  else if (VS_1_V == scale)
    HAL_GPIO_Q1_clr();
  else if (VS_2_V == scale)
    HAL_GPIO_Q6_clr();
  else if (VS_5_V == scale)
    HAL_GPIO_Q5_clr();
  else if (VS_10_V ==scale)
    HAL_GPIO_Q4_clr();
  else
    while (1);
}

//-----------------------------------------------------------------------------
static void set_ac_coupling(void)
{
  HAL_GPIO_AC_DC_write(!config.ac_coupling);
}

//-----------------------------------------------------------------------------
void capture_init(void)
{
  adc_init();
  dac_init();
  vertical_scale_init();

  g_triggered      = false;
  g_stopped        = true;
  g_dual_channel   = false;
  g_trigger_offset = CAPTURE_BUFFER_SIZE / 2;

  capture_set_trigger_edge(TRIGGER_EDGE_RISE);
  capture_set_trigger_mode(TRIGGER_MODE_AUTO);

  g_capture_buffer_info.valid = false;
  g_capture_buffer_info.size  = CAPTURE_BUFFER_SIZE;
  g_capture_buffer_info.data  = (uint8_t *)g_capture_buffer;

  g_storage_buffer_info.valid = false;
  g_storage_buffer_info.size  = STORAGE_BUFFER_SIZE;
  g_storage_buffer_info.data  = (uint8_t *)g_storage_buffer;
}

//-----------------------------------------------------------------------------
void capture_disable_clock(void)
{
  TIMER0->CTL0 = 0;
  TIMER7->CTL0 = 0;
}

//-----------------------------------------------------------------------------
static bool check_trigger_condition(int prev, int new)
{
  if (TRIGGER_EDGE_RISE == g_trigger_edge)
  {
    return (prev < g_trigger_level && new > g_trigger_level);
  }
  else if (TRIGGER_EDGE_FALL == g_trigger_edge)
  {
    return (prev > g_trigger_level && new < g_trigger_level);
  }
  else
  {
    return (prev < g_trigger_level && new > g_trigger_level) ||
           (prev > g_trigger_level && new < g_trigger_level);
  }
}

//-----------------------------------------------------------------------------
static void update_capture_buffer(void)
{
  g_capture_buffer_info.offset = g_next_buf_ptr - dma_get_count();

  if (g_capture_buffer_info.offset < 0)
    g_capture_buffer_info.offset += CAPTURE_BUFFER_SIZE;
  else if (g_capture_buffer_info.offset >= CAPTURE_BUFFER_SIZE)
    g_capture_buffer_info.offset -= CAPTURE_BUFFER_SIZE;

  if (g_auto_mode_stop)
    g_trigger_ptr = (g_capture_buffer_info.offset + g_trigger_offset) % CAPTURE_BUFFER_SIZE;

  g_capture_buffer_info.trigger = g_trigger_ptr;
  g_capture_buffer_info.valid   = true;
}

//-----------------------------------------------------------------------------
static void update_storage_buffer(void)
{
  int offset = g_capture_buffer_info.trigger % STORAGE_BUFFER_RATIO;

  if (g_storage_buffer_info.valid)
    return;

  // NOTE: Given the way dual channel trigger event search is implemented,
  //       the reverse version will never be called. I'm still keeping it
  //       here just in case things change in the future.
  if (g_dual_channel && (offset & 1) == 1)
    buffer_decimate_reverse((uint32_t)g_storage_buffer, (uint32_t)g_capture_buffer, CAPTURE_BUFFER_SIZE, offset);
  else
    buffer_decimate((uint32_t)g_storage_buffer, (uint32_t)g_capture_buffer, CAPTURE_BUFFER_SIZE, offset);

  g_storage_buffer_info.offset = g_capture_buffer_info.offset / STORAGE_BUFFER_RATIO;

  if (offset < (g_capture_buffer_info.offset % STORAGE_BUFFER_RATIO))
    g_storage_buffer_info.offset++;

  g_storage_buffer_info.period  = g_sample_period * STORAGE_BUFFER_RATIO;
  g_storage_buffer_info.trigger = g_capture_buffer_info.trigger / STORAGE_BUFFER_RATIO;
  g_storage_buffer_info.vpos    = g_capture_buffer_info.vpos;
  g_storage_buffer_info.vs_mult = g_capture_buffer_info.vs_mult;
  g_storage_buffer_info.valid   = true;
}

//-----------------------------------------------------------------------------
static inline void dma_start(void)
{
  DMA1->CH2CTL_b.MBS = 0;

  DMA1->CH2M0ADDR = (uint32_t)g_capture_buffer;
  DMA1->CH2M1ADDR = (uint32_t)g_capture_buffer + g_dma_buffer_size;

  if (g_dual_channel)
  {
    DMA1->CH2CTL = (0/*P2M*/ << DMA1_CH2CTL_TM_Pos) | DMA1_CH2CTL_SBMEN_Msk |
        (1/*16-bit*/ << DMA1_CH2CTL_PWIDTH_Pos) | (1/*16-bit*/ << DMA1_CH2CTL_MWIDTH_Pos) |
        DMA1_CH2CTL_MNAGA_Msk | (3/*UltraHigh*/ << DMA1_CH2CTL_PRIO_Pos) |
        (7/*TIMER7_CH0*/ << DMA1_CH2CTL_PERIEN_Pos) | DMA1_CH2CTL_FTFIE_Msk;

    DMA1->CH2CNT = g_dma_buffer_size / 2;
  }
  else
  {
    DMA1->CH2CTL = (0/*P2M*/ << DMA1_CH2CTL_TM_Pos) | DMA1_CH2CTL_SBMEN_Msk |
        (0/*8-bit*/ << DMA1_CH2CTL_PWIDTH_Pos) | (0/*8-bit*/ << DMA1_CH2CTL_MWIDTH_Pos) |
        DMA1_CH2CTL_MNAGA_Msk | (3/*UltraHigh*/ << DMA1_CH2CTL_PRIO_Pos) |
        (7/*TIMER7_CH0*/ << DMA1_CH2CTL_PERIEN_Pos) | DMA1_CH2CTL_FTFIE_Msk;

    DMA1->CH2CNT = g_dma_buffer_size;
  }

  g_active_buf_ptr = 0;
  g_next_buf_ptr   = g_dma_buffer_size * 2;
  g_trigger_ptr    = 0;
  g_count          = 0;
  g_remaining      = 0;
  g_triggered      = false;
  g_auto_mode_stop = false;

  g_capture_buffer_info.period  = g_sample_period;
  g_capture_buffer_info.vpos    = config.vertical_position_mv;
  g_capture_buffer_info.vs_mult = config.calib_vs_mult[config.vertical_scale];
  g_capture_buffer_info.valid   = false;

  DMA1->INTC0 = DMA1_INTC0_FTFIFC2_Msk;
  NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
  NVIC_EnableIRQ(DMA1_Channel2_IRQn);

  DMA1->CH2CTL_b.CHEN = 1;
}

//-----------------------------------------------------------------------------
static inline void dma_stop(void)
{
  NVIC_DisableIRQ(DMA1_Channel2_IRQn);

  DMA1->CH2CTL_b.CHEN = 0;
  while (DMA1->CH2CTL_b.CHEN);

  DMA1->INTC0 = DMA1_INTC0_FTFIFC2_Msk;
  NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
}

//-----------------------------------------------------------------------------
static inline int dma_get_count(void)
{
  if (g_dual_channel)
    return DMA1->CH2CNT * 2;
  else
    return DMA1->CH2CNT;
}

//-----------------------------------------------------------------------------
static inline void dma_wait_count(uint32_t count)
{
  if (g_dual_channel)
    count /= 2;

  while (DMA1->CH2CNT > count);
}

//-----------------------------------------------------------------------------
static inline void dma_finish(void)
{
  dma_stop();
  update_capture_buffer();
  update_storage_buffer();

  if (TRIGGER_MODE_SINGLE == g_trigger_mode)
  {
    if (g_dual_channel)
      buffer_reverse((uint32_t)g_capture_buffer, CAPTURE_BUFFER_SIZE);

    g_stopped = true;
  }
  else
  {
    dma_start();
  }
}

//-----------------------------------------------------------------------------
void irq_handler_dma1_channel2(void)
{
  uint8_t *active_buffer = (uint8_t *)g_capture_buffer + g_active_buf_ptr;

  DMA1->INTC0 = DMA1_INTC0_FTFIFC2_Msk;

  if (DMA1->CH2CTL_b.MBS)
    DMA1->CH2M0ADDR = (uint32_t)g_capture_buffer + g_next_buf_ptr;
  else
    DMA1->CH2M1ADDR = (uint32_t)g_capture_buffer + g_next_buf_ptr;

  if (g_triggered)
  {
    if (g_remaining >= g_dma_buffer_size)
    {
      g_remaining -= g_dma_buffer_size;
    }
    else
    {
      dma_wait_count(g_dma_buffer_size - g_remaining);
      dma_finish();
      return;
    }
  }

  else if (g_count < g_trigger_offset)
  {
    g_count += g_dma_buffer_size;
  }

  else
  {
    int trigger;

    if (check_trigger_condition(g_last_sample, active_buffer[0]))
      trigger = g_dma_buffer_size;
    else
      trigger = g_trigger_find((uint32_t)active_buffer, g_dma_buffer_size);

    if (trigger > 0)
    {
      g_triggered = true;
      g_trigger_ptr = g_active_buf_ptr + (g_dma_buffer_size - trigger);
      g_remaining = (CAPTURE_BUFFER_SIZE - g_trigger_offset) - trigger;

      if (g_remaining < 0)
      {
        dma_finish();
        return;
      }
      else if (g_remaining < g_dma_buffer_size)
      {
        dma_wait_count(g_dma_buffer_size - g_remaining);
        dma_finish();
        return;
      }
      else
      {
        g_remaining -= g_dma_buffer_size;
      }
    }
    else if (TRIGGER_MODE_AUTO == g_trigger_mode)
    {
      g_count += g_dma_buffer_size;

      if (g_count > g_auto_mode_count)
      {
        g_auto_mode_stop = true;
        dma_finish();
        return;
      }
    }
  }

  g_last_sample    = active_buffer[g_dma_buffer_size - (g_dual_channel ? 2 : 1)];
  g_next_buf_ptr   = (g_next_buf_ptr + g_dma_buffer_size) % CAPTURE_BUFFER_SIZE;
  g_active_buf_ptr = (g_active_buf_ptr + g_dma_buffer_size) % CAPTURE_BUFFER_SIZE;
}

//-----------------------------------------------------------------------------
void capture_start(void)
{
  if (!g_stopped)
    return;

  g_stopped = false;

  dma_start();
}

//-----------------------------------------------------------------------------
void capture_stop(void)
{
  if (g_stopped)
    return;

  dma_stop();

  g_stopped = true;
}

//-----------------------------------------------------------------------------
static void update_trigger_handler(void)
{
  if (g_dual_channel)
  {
    if (TRIGGER_EDGE_RISE == g_trigger_edge)
      g_trigger_find = trigger_find_rise_dual;
    else if (TRIGGER_EDGE_FALL == g_trigger_edge)
      g_trigger_find = trigger_find_fall_dual;
    else
      g_trigger_find = trigger_find_both_dual;
  }
  else
  {
    if (TRIGGER_EDGE_RISE == g_trigger_edge)
      g_trigger_find = trigger_find_rise_single;
    else if (TRIGGER_EDGE_FALL == g_trigger_edge)
      g_trigger_find = trigger_find_fall_single;
    else
      g_trigger_find = trigger_find_both_single;
  }
}

//-----------------------------------------------------------------------------
void capture_set_vertical_parameters(void)
{
  int offset = (config.vertical_position * config.calib_dac_mult[config.vertical_scale]) / 1024;

  dma_stop();

  set_ac_coupling();
  dac_write(config.calib_dac_zero + offset);
  set_vertical_scale();

  if (!g_stopped)
    dma_start();
}

//-----------------------------------------------------------------------------
void capture_set_horizontal_parameters(int sr_divider, int trigger_offset)
{
  int divider, dma_divider;

  dma_stop();

  if (sr_divider < 1)
  {
    divider = 1;
    g_dual_channel = true;
  }
  else
  {
    divider = (1 << sr_divider) - 1;
    g_dual_channel = false;
  }

  dma_divider = (sr_divider < 6) ? 1 : (1 << (sr_divider - 6));

  g_dma_buffer_size = DMA_MAX_BUFFER_SIZE / dma_divider;

  TIMER0->CTL0 = 0;
  TIMER7->CTL0 = 0;

  TIMER0->PSC = (divider > 63) ? 63 : divider;
  TIMER7->PSC = divider;

  TIMER0->CNT = 0;
  TIMER7->CNT = 0;

  TIMER0->CTL0 = TIMER0_CTL0_CEN_Msk;
  TIMER7->CTL0 = TIMER7_CTL0_CEN_Msk;

  g_trigger_offset  = trigger_offset;
  g_sample_period   = BASE_SAMPLE_PERIOD * (1 << sr_divider);
  g_auto_mode_count = (BASE_SAMPLE_RATE / (1 << sr_divider)) / AUTO_MODE_COUNT_DIV;

  if (g_auto_mode_count < CAPTURE_BUFFER_SIZE)
    g_auto_mode_count = CAPTURE_BUFFER_SIZE;

  update_trigger_handler();

  if (!g_stopped)
    dma_start();
}

//-----------------------------------------------------------------------------
void capture_set_trigger_level(int level)
{
  g_trigger_level = (level * CALIB_MULTIPLIER) / config.calib_vs_mult[config.vertical_scale] + ZERO_POINT;

  if (g_trigger_level < 20)
    g_trigger_level = 20;
  else if (g_trigger_level > 235)
    g_trigger_level = 235;

  trigger_set_levels(g_trigger_level);
}

//-----------------------------------------------------------------------------
void capture_set_trigger_edge(int edge)
{
  dma_stop();

  g_trigger_edge = edge;

  update_trigger_handler();

  if (!g_stopped)
    dma_start();
}

//-----------------------------------------------------------------------------
void capture_set_trigger_mode(int mode)
{
  dma_stop();

  g_trigger_mode = mode;

  if (!g_stopped)
    dma_start();
}

//-----------------------------------------------------------------------------
int capture_get_state(void)
{
  if (g_stopped)
    return CAPTURE_STATE_STOP;
  else if (g_triggered)
    return CAPTURE_STATE_TRIG;
  else
   return CAPTURE_STATE_WAIT;
}

//-----------------------------------------------------------------------------
bool capture_buffer_updated(void)
{
  return g_storage_buffer_info.valid;
}

//---------------------------------------------------------------------
static void find_min_max_buf_aligned(uint32_t buf, int size, int *vmin, int *vmax)
{
  uint32_t min = *vmin;
  uint32_t max = *vmax;

  asm volatile (R"asm(
    t          .req r3
    b0         .req r5
    b1         .req r6

    bfi        %[min], %[min], #8, #8
    bfi        %[min], %[min], #16, #8
    bfi        %[min], %[min], #24, #8

    bfi        %[max], %[max], #8, #8
    bfi        %[max], %[max], #16, #8
    bfi        %[max], %[max], #24, #8

0:
    ldm        %[buf]!, { b0, b1 }

    usub8      t, %[min], b0
    sel        %[min], b0, %[min]
    usub8      t, %[min], b1
    sel        %[min], b1, %[min]

    usub8      t, %[max], b0
    sel        %[max], %[max], b0
    usub8      t, %[max], b1
    sel        %[max], %[max], b1

    subs       %[size], #8
    bne        0b

    // Finalize min value
    ubfx       b0, %[min], #16, #16
    usub8      t, %[min], b0
    sel        %[min], b0, %[min]

    ubfx       b0, %[min], #8, #8
    usub8      t, %[min], b0
    sel        %[min], b0, %[min]

    and        %[min], #0xff

    // Finalize max value
    ubfx       b0, %[max], #16, #16
    usub8      t, %[max], b0
    sel        %[max], %[max], b0

    ubfx       b0, %[max], #8, #8
    usub8      t, %[max], b0
    sel        %[max], %[max], b0

    and        %[max], #0xff
99:
    .unreq     t
    .unreq     b0
    .unreq     b1
    )asm"
    : [buf] "+r" (buf), [size] "+r" (size), [min] "+r" (min), [max] "+r" (max)
    : /* none */
    : "r3", "r5", "r6"
  );

  *vmin = min;
  *vmax = max;
}

//---------------------------------------------------------------------
static void find_min_max_buf(uint8_t *data, int size, int *vmin, int *vmax)
{
  int max = *vmax;
  int min = *vmin;

  while (size > 0 && ((uint32_t)data & 3))
  {
    int v = *data;

    if (v > max)
      max = v;

    if (v < min)
      min = v;

    size--;
    data++;
  }

  if (size >= 8)
  {
    int sz = size & ~7;

    find_min_max_buf_aligned((uint32_t)data, sz, &min, &max);

    data += sz;
    size -= sz;
  }

  while (size > 0)
  {
    int v = *data;

    if (v > max)
      max = v;

    if (v < min)
      min = v;

    size--;
    data++;
  }

  *vmax = max;
  *vmin = min;
}

//---------------------------------------------------------------------
static int clamp_index(BufferInfo *info, int index)
{
  if (index < info->min_index)
    index = info->min_index;

  if (index > info->max_index)
    index = info->max_index;

  index = info->trigger + index;

  if (index < 0)
    index += info->size;
  else if (index >= info->size)
    index -= info->size;

  return index;
}

//---------------------------------------------------------------------
static bool find_min_max(BufferInfo *info, int index0, int index1, int *vmin, int *vmax)
{
  if (index0 > info->max_index || index1 < info->min_index)
    return false;

  index0 = clamp_index(info, index0);
  index1 = clamp_index(info, index1);

  if (index0 < index1)
  {
    find_min_max_buf(&info->data[index0], index1 - index0, vmin, vmax);
  }
  else
  {
    find_min_max_buf(&info->data[index0], info->size-1 - index0, vmin, vmax);
    find_min_max_buf(&info->data[0], index1, vmin, vmax);
  }

  return true;
}

//---------------------------------------------------------------------
static int calc_frequency(BufferInfo *info)
{
  int pn, pa, pb, level, index;
  bool low;

  if (!config.measure_display)
    return 0;

  index = info->offset;
  low = (info->data[index] < g_trigger_level);
  level = g_trigger_level + (low ? MEASURE_HYSTERESIS : -MEASURE_HYSTERESIS);
  pn = 0;

  for (int i = 0; i < info->size; i++)
  {
    bool toggle = (low && (info->data[index] > level)) || (!low && (info->data[index] < level));

    if (toggle)
    {
      if (low)
      {
        if (pn == 0)
          pa = i;
        else
          pb = i;

        pn++;
        low = false;
        level = g_trigger_level - MEASURE_HYSTERESIS;
      }
      else
      {
        low = true;
        level = g_trigger_level + MEASURE_HYSTERESIS;
      }
    }

    index++;

    if (index == info->size)
      index = 0;
  }

  if (pn < 2)
    return 0;

  return ((uint64_t)(pn-1) * (uint64_t)1e9) / ((pb - pa) * info->period);
}

//---------------------------------------------------------------------
void capture_get_data(DataBuffer *db)
{
  BufferInfo *capture_info = (BufferInfo *)&g_capture_buffer_info;
  BufferInfo *storage_info = (BufferInfo *)&g_storage_buffer_info;
  BufferInfo *info = NULL;
  int index_inc, error_inc, index, error, next_index, next_error;
  int istart, dx, min_value, max_value, flags;
  int64_t offs;

  if (g_stopped && capture_info->valid)
    info = capture_info;
  else
    info = storage_info;

  offs = config.horizontal_position - (int64_t)config.horizontal_period * (db->size/2 - 1) -
      info->period/2 - config.horizontal_period/2;

  index_inc = config.horizontal_period / info->period;
  error_inc = config.horizontal_period % info->period;
  index     = offs / info->period;
  error     = offs % info->period;
  dx        = info->period / config.horizontal_period;
  istart    = 0;

  if (error < 0)
  {
    index -= 1;
    error += info->period;
  }

  if (info->trigger > info->offset)
    info->min_index = info->offset - info->trigger;
  else
    info->min_index = info->offset - info->trigger - info->size;

  info->max_index = info->size + info->min_index - 1;

  if (index_inc == 0)
  {
    for (istart = 0; (error + istart * error_inc) > 0; istart--);
  }

  db->min_value = INT_MAX;
  db->max_value = INT_MIN;
  db->vertical_position = info->vpos;

  for (int i = 0; i < db->size; i++)
  {
    next_index = index + index_inc;
    next_error = error + error_inc;

    if (next_error >= info->period)
    {
      next_index += 1;
      next_error -= info->period;
    }

    flags = SAMPLE_FLAG_NONE;

    if (next_index == index)
    {
      int di = i - istart;
      int v = info->data[clamp_index(info, index)];
      int nv = info->data[clamp_index(info, index + 1)];
      int value = ((dx - di) * v + di * nv + dx/2) / dx;

      min_value = value;
      max_value = value;
      flags = SAMPLE_FLAG_VALID | SAMPLE_FLAG_FILLED;
    }
    else if ((next_index - index) == 1)
    {
      int idx = clamp_index(info, next_index);

      istart = i;

      min_value = info->data[idx];
      max_value = info->data[idx];
      flags = SAMPLE_FLAG_VALID;
    }
    else
    {
      istart = i;
      min_value = 255;
      max_value = 0;

      if (find_min_max(info, index, next_index-1, &min_value, &max_value))
        flags = SAMPLE_FLAG_VALID;
    }

    if (min_value == 0)
      flags |= SAMPLE_FLAG_CLIP_L;

    if (max_value == 255)
      flags |= SAMPLE_FLAG_CLIP_H;

    if (flags & SAMPLE_FLAG_VALID)
    {
      min_value = ((min_value - ZERO_POINT) * info->vs_mult + info->vs_mult/2) / CALIB_MULTIPLIER;
      max_value = ((max_value - ZERO_POINT) * info->vs_mult + info->vs_mult/2) / CALIB_MULTIPLIER;

      if (min_value < db->min_value)
        db->min_value = min_value;

      if (max_value > db->max_value)
        db->max_value = max_value;
    }

    db->min[i] = min_value;
    db->max[i] = max_value;
    db->flags[i] = flags;

    index = next_index;
    error = next_error;
  }

  db->frequency = calc_frequency(info);

  g_storage_buffer_info.valid = false;
}

//---------------------------------------------------------------------
void capture_get_raw_data(int *raw, int size)
{
  for (int i = 0; i < size; i++)
    raw[i] = g_capture_buffer[i];
}

