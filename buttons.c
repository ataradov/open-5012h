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
#include "timer.h"
#include "buttons.h"

/*
  Button mapping:
    PE13 - PE0.A0
    PB14 - PE0.A1
    PB13 - PE0.A2
    PE14 - PE0.GS
    PE15 - PE1.A0
    PB12 - PE1.A1
    PB11 - PE1.A2
    PB10 - PE1.GS
    PE12 - BTN_1X10X
    PE11 - BTN_F2
*/

/*- Definitions -------------------------------------------------------------*/
#define DEBOUNCE_TIMEOUT       20
#define REPEAT_DELAY           250
#define REPEAT_INTERVAL_MAX    50
#define REPEAT_INTERVAL_MIN    10

/*- Variables ---------------------------------------------------------------*/
static int g_next_state = 0;
static int g_buttons_state = 0;
static int g_debounce_timer = TIMER_DISABLE;
static int g_repeat_timer = TIMER_DISABLE;
static int g_repeat_interval = 0;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
int buttons_read(void)
{
  int pb = ~GPIOB->ISTAT;
  int pe = ~GPIOE->ISTAT;
  int state = 0;

  if (pe & (1 << 11))
    state |= BTN_F2;

  if (pe & (1 << 12))
    state |= BTN_1X_10X;

  if (pe & (1 << 14))
  {
    int idx = ((pb >> 13) & 0x3) | ((pe >> 11) & 0x4);
    state |= (1 << idx);
  }
  
  if (pb & (1 << 10))
  {
    int idx = ((pb >> 11) & 0x3) | ((pe >> 13) & 0x4);
    state |= (0x100 << idx);
  }

  return state;
}

//-----------------------------------------------------------------------------
void buttons_init(void)
{
  g_next_state = buttons_read();
  g_buttons_state = g_next_state;

  timer_add(&g_debounce_timer);
  timer_add(&g_repeat_timer);
}

//-----------------------------------------------------------------------------
int buttons_state(void)
{
  return g_buttons_state;
}

//-----------------------------------------------------------------------------
void buttons_task(void)
{
  int state = buttons_read();

  if (state != g_next_state)
  {
    g_next_state = state;
    g_debounce_timer = DEBOUNCE_TIMEOUT;
  }

  if (0 == g_repeat_timer && g_buttons_state)
  {
    g_repeat_timer = g_repeat_interval;

    if (g_repeat_interval > REPEAT_INTERVAL_MIN)
      g_repeat_interval--;

    buttons_handler(g_buttons_state | BTN_REPEAT);
  }

  if (0 == g_debounce_timer && g_next_state != g_buttons_state)
  {
    g_repeat_timer    = g_next_state ? REPEAT_DELAY : TIMER_DISABLE;
    g_repeat_interval = REPEAT_INTERVAL_MAX;
    g_debounce_timer  = TIMER_DISABLE;
    g_buttons_state   = g_next_state;
    buttons_handler(g_buttons_state);
  }
}

