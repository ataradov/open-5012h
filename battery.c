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
#include "config.h"
#include "utils.h"
#include "timer.h"
#include "lcd.h"
#include "battery.h"

/*- Definitions -------------------------------------------------------------*/
#define BATTERY_X                289
#define BATTERY_Y                4
#define BATTERY_THRESHOLDS_SIZE  8
#define BATTERY_REF_VOLTAGE      6600
#define BATTERY_LOW_VOLTAGE      3500
#define BATTERY_FULL_VOLTAGE     4150
#define BATTERY_BLINK_INTERVAL   500
#define BATTERY_UPDATE_INTERVAL  5000

HAL_GPIO_PIN(VBAT,     B, 1)
HAL_GPIO_PIN(CHARGING, B, 15)

/*- Constants ---------------------------------------------------------------*/
static const int battery_thresholds[BATTERY_THRESHOLDS_SIZE] =
{
  BATTERY_LOW_VOLTAGE, 3600, 3650, 3700, 3750, 3800, 3850, 3900,
};

static const int battery_thresholds_charging[BATTERY_THRESHOLDS_SIZE] =
{
  3800, 3850, 3900, 3920, 3960, 4000, 4040, 4070,
};

/*- Variables ---------------------------------------------------------------*/
static int g_battery_blink_timer = TIMER_DISABLE;
static int g_battery_update_timer = TIMER_DISABLE;
static int g_battery_voltage = 0;
static bool g_battery_blink_state = true;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static bool battery_charging(void)
{
  if (HAL_GPIO_CHARGING_read() || (g_battery_voltage > BATTERY_FULL_VOLTAGE))
    return false;
  else
    return true;
}

//-----------------------------------------------------------------------------
static int read_battery_voltage(void)
{
  return (ADC0->RDATA * BATTERY_REF_VOLTAGE) / 4096;
}

//-----------------------------------------------------------------------------
static void battery_draw_frame(void)
{
  lcd_fill_rect(BATTERY_X,    BATTERY_Y,   20, 10, LCD_WHITE_COLOR);
  lcd_fill_rect(BATTERY_X+1,  BATTERY_Y+1, 18, 8,  LCD_BLACK_COLOR);
  lcd_fill_rect(BATTERY_X+20, BATTERY_Y+3, 2,  4,  LCD_WHITE_COLOR);
}

//-----------------------------------------------------------------------------
static void battery_draw_level(void)
{
  const int *thresholds = HAL_GPIO_CHARGING_read() ? battery_thresholds : battery_thresholds_charging;
  int bars, color;

  for (bars = 0; bars < BATTERY_THRESHOLDS_SIZE; bars++)
  {
    if (g_battery_voltage < thresholds[bars])
      break;
  }

  if (bars < 2)
    color = LCD_RED_COLOR;
  else if (bars < 4)
    color = LCD_COLOR(255, 200, 0);
  else
    color = LCD_GREEN_COLOR;

  if (bars > 0 && !g_battery_blink_state)
    bars--;

  if (bars > 0)
    lcd_fill_rect(BATTERY_X+2, BATTERY_Y+2, bars * 2, 6, color);

  if (bars < BATTERY_THRESHOLDS_SIZE)
  {
    lcd_fill_rect(BATTERY_X+2 + bars * 2, BATTERY_Y+2,
        (BATTERY_THRESHOLDS_SIZE-bars) * 2, 6, LCD_BLACK_COLOR);
  }
}

//-----------------------------------------------------------------------------
void battery_init(void)
{
  HAL_GPIO_CHARGING_in();
  HAL_GPIO_CHARGING_pullup();

  HAL_GPIO_VBAT_analog();

  RCU->APB2EN_b.ADC0EN = 1;

  ADC_Common->SYNCCTL_b.ADCCK = 7; // HCLK / 20

  ADC0->OVSAMPCTL = ADC0_OVSAMPCTL_OVSEN_Msk | (7/*256x*/ << ADC0_OVSAMPCTL_OVSR_Pos) |
      (8/*Shift 8-bits*/ << ADC0_OVSAMPCTL_OVSS_Pos);

  ADC0->CTL1_b.ADCON = 1;
  delay_ms(1);

  ADC0->CTL1_b.RSTCLB = 1;
  while (ADC0->CTL1_b.RSTCLB);

  ADC0->CTL1_b.CLB = 1;
  while (ADC0->CTL1_b.CLB);

  ADC0->RSQ2_b.RSQ0 = 9;
  ADC0->SAMPT1_b.SPT0 = 7; // 480 cycles

  ADC0->CTL1_b.CTN = 1;

  ADC0->CTL1_b.SWRCST = 1;
  while (0 == ADC0->STAT_b.EOC);

  g_battery_voltage = read_battery_voltage();

  timer_add(&g_battery_update_timer);
  timer_add(&g_battery_blink_timer);

  g_battery_update_timer = 0;

  battery_draw_frame();
  battery_draw_level();
}

//-----------------------------------------------------------------------------
int battery_voltage(void)
{
  return g_battery_voltage;
}

//-----------------------------------------------------------------------------
void battery_task(void)
{
  if (0 == g_battery_update_timer)
  {
    int voltage = read_battery_voltage();

    if (voltage < BATTERY_LOW_VOLTAGE)
      battery_low_handler();

    if (voltage != g_battery_voltage)
    {
      g_battery_voltage = voltage;
      battery_draw_level();
    }

    g_battery_update_timer = BATTERY_UPDATE_INTERVAL;
  }

  if (0 == g_battery_blink_timer)
  {
    g_battery_voltage = read_battery_voltage();

    if (battery_charging())
    {
      g_battery_blink_timer = BATTERY_BLINK_INTERVAL;
      g_battery_blink_state = !g_battery_blink_state;
    }
    else
    {
      g_battery_blink_timer = TIMER_DISABLE;
      g_battery_blink_state = true;
    }

    battery_draw_level();
  }

  if (TIMER_DISABLE == g_battery_blink_timer && battery_charging())
  {
    config.charge_cycles++;
    g_battery_blink_timer = BATTERY_BLINK_INTERVAL;
  }
}


