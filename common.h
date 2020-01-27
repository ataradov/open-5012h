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

#ifndef _COMMON_H_
#define _COMMON_H_

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*- Definitions -------------------------------------------------------------*/
enum // Horizontal Scale
{
  HS_50_ns,
  HS_100_ns,
  HS_200_ns,
  HS_500_ns,

  HS_1_us,
  HS_2_us,
  HS_5_us,
  HS_10_us,
  HS_20_us,
  HS_50_us,
  HS_100_us,
  HS_200_us,
  HS_500_us,

  HS_1_ms,
  HS_2_ms,
  HS_5_ms,
  HS_10_ms,
  HS_20_ms,
  HS_50_ms,
  HS_100_ms,
  HS_200_ms,
  HS_500_ms,

  HS_LAST = HS_500_ms,
  HS_COUNT,
};

enum // Vertical Scale
{
  VS_50_mV,
  VS_100_mV,
  VS_200_mV,
  VS_500_mV,
  VS_1_V,
  VS_2_V,
  VS_5_V,
  VS_10_V,

  VS_LAST = VS_10_V,
  VS_COUNT,
};

enum
{
  TRIGGER_EDGE_RISE,
  TRIGGER_EDGE_FALL,
  TRIGGER_EDGE_BOTH,
};

enum
{
  TRIGGER_MODE_AUTO,
  TRIGGER_MODE_NORMAL,
  TRIGGER_MODE_SINGLE,
};

enum
{
  CAPTURE_STATE_STOP,
  CAPTURE_STATE_WAIT,
  CAPTURE_STATE_TRIG,
};

enum
{
  SAMPLE_FLAG_NONE   = 0,
  SAMPLE_FLAG_VALID  = (1 << 0),
  SAMPLE_FLAG_FILLED = (1 << 1),
  SAMPLE_FLAG_CLIP_L = (1 << 2),
  SAMPLE_FLAG_CLIP_H = (1 << 3),
};

/*- Prototypes  -------------------------------------------------------------*/
void error(char *text);

#endif // _COMMON_H_


