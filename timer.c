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
#include "common.h"
#include "timer.h"

/*- Definitions -------------------------------------------------------------*/
#define MAX_TIMERS     16
#define MS             (F_CPU / 8 / 1000)

/*- Variables ---------------------------------------------------------------*/
static int *g_timer_list[MAX_TIMERS];
static int g_timer_count = 0;
static int g_timer_max_delta = 0;
static int g_timer_prev_value = 0;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static void timer_restart(void)
{
  SysTick->CTRL = 0;
  SysTick->VAL  = 0;
  SysTick->LOAD = 0xffffff;
  SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
}

//-----------------------------------------------------------------------------
void timer_init(void)
{
  g_timer_count      = 0;
  g_timer_max_delta  = 0;
  g_timer_prev_value = 0xffffff;
  timer_restart();
}

//-----------------------------------------------------------------------------
void timer_add(int *timer)
{
  for (int i = 0; i < g_timer_count; i++)
  {
    if (g_timer_list[i] == timer)
      return;
  }

  if (g_timer_count == MAX_TIMERS)
    while (1);

  g_timer_list[g_timer_count++] = timer;
}

//-----------------------------------------------------------------------------
void timer_remove(int *timer)
{
  for (int i = 0; i < g_timer_count; i++)
  {
    if (g_timer_list[i] == timer)
    {
      g_timer_list[i] = g_timer_list[g_timer_count-1];
      g_timer_count--;
      return;
    }
  }
}

//-----------------------------------------------------------------------------
int timer_get_max_delta(void)
{
  int res = g_timer_max_delta;
  g_timer_max_delta = 0;
  return res;
}

//-----------------------------------------------------------------------------
void timer_task(void)
{
  int value = SysTick->VAL;
  int delta = g_timer_prev_value - value;

  if (delta > MS)
  {
    int ms  = delta / MS;
    int rem = delta % MS;

    for (int i = 0; i < g_timer_count; i++)
    {
      if (*g_timer_list[i] > ms)
        *g_timer_list[i] -= ms;
      else if (*g_timer_list[i] > 0)
        *g_timer_list[i] = 0;
    }

    g_timer_prev_value = value - rem;

    if (ms > g_timer_max_delta)
      g_timer_max_delta = ms;
  }

  if (value < 0x7fffff)
  {
    g_timer_prev_value += 0x7fffff;
    SysTick->LOAD = SysTick->VAL + 0x7fffff;
    SysTick->VAL  = 0;
  }

  if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    error("Timer overflow");
}


