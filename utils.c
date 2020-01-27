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
#include "utils.h"

/*- Definitions -------------------------------------------------------------*/
#define SPACE      "\x01"

/*- Variables ---------------------------------------------------------------*/
static char g_buf[64];

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void crc32_init(void)
{
  RCU->AHB1EN_b.CRCEN = 1;
}

//-----------------------------------------------------------------------------
uint32_t crc32_calc(uint32_t *data, int size)
{
  size /= sizeof(uint32_t);

  CRC->CTL = CRC_CTL_RST_Msk;
  while (CRC->CTL);

  for (int i = 0; i < size; i++)
    CRC->DATA = data[i];

  return CRC->DATA;
}

//-----------------------------------------------------------------------------
static char *format_number(int64_t value, int sign, int prec, int width, char *unit)
{
  char *start = g_buf + sizeof(g_buf) - 8; // Maximum length of a unit string is 7
  char *ptr = start;

  while (*unit)
    *ptr++ = *unit++;

  *ptr = 0;
  ptr = start;

  do
  {
    *(--ptr) = '0' + (value % 10);
    value /= 10;
    if (--prec == 0)
      *(--ptr) = '.';
  } while (value > 0 || prec >= 0);

  if (sign < 0)
    *(--ptr) = '-';
  else if (sign > 0)
    *(--ptr) = '+';

  while ((start - ptr) < width)
    *(--ptr) = ' ';

  return ptr;
}

//-----------------------------------------------------------------------------
char *format_time(int64_t value, bool show_plus_sign)
{
  int sign = (value < 0) ? -1 : (show_plus_sign ? 1 : 0);

  if (value < 0)
    value = -value;

  if (value < 1000)
    return format_number(value * 100, sign, 2, 7, SPACE"ns");
  else if (value < 1000000)
    return format_number(value / 10, sign, 2, 7, SPACE"us");
  else if (value < 1000000000)
    return format_number(value / 10000, sign, 2, 7, SPACE"ms");
  else
    return format_number(value / 10000000, sign, 2, 7, SPACE"s ");
}

//-----------------------------------------------------------------------------
char *format_voltage(int value, bool show_plus_sign)
{
  int sign = (value < 0) ? -1 : (show_plus_sign ? 1 : 0);

  if (value < 0)
    value = -value;

  if (value < 1000)
    return format_number(value * 100, sign, 2, 7, SPACE"mV");
  else
    return format_number(value / 10, sign, 2, 7, SPACE"V ");
}

//-----------------------------------------------------------------------------
char *format_divisions(int value, bool show_plus_sign)
{
  int sign = (value < 0) ? -1 : (show_plus_sign ? 1 : 0);

  if (value < 0)
    value = -value;

  return format_number(value, sign, 2, 6, SPACE"div");
}

//-----------------------------------------------------------------------------
char *format_frequency(int value)
{
  if (value < 1000)
    return format_number(value * 100, 0, 2, 6, SPACE"Hz ");
  else if (value < 1000000)
    return format_number(value / 10, 0, 2, 6, SPACE"kHz");
  else
    return format_number(value / 10000, 0, 2, 6, SPACE"MHz");
}

//-----------------------------------------------------------------------------
char *format_sps(int value)
{
  if (value < 1000)
    return format_number(value, 0, 0, 3, SPACE" ");
  else if (value < 1000000)
    return format_number(value / 1000, 0, 0, 3, SPACE"K");
  else
    return format_number(value / 1000000, 0, 0, 3, SPACE"M");
}

//-----------------------------------------------------------------------------
char *format_raw_data(int *data, int size)
{
  static const char hex[] = "0123456789ABCDEF";
  char *ptr = g_buf;;

  for (int i = 0; i < size; i++)
  {
    ptr[0] = hex[(data[i] >> 4) & 0xf];
    ptr[1] = hex[data[i] & 0xf];
    ptr[2] = ' ';
    ptr += 3;
  }

  ptr[size ? -1 : 0] = 0;

  return g_buf;
}

//-----------------------------------------------------------------------------
uint64_t round_divide(int64_t dividend, int64_t divisor)
{
  return (dividend + (divisor / 2)) / divisor;
}


