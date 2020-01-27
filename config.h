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

#ifndef _CONFIG_H_
#define _CONFIG_H_

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "common.h"

/*- Definitions -------------------------------------------------------------*/
#define CALIB_MULTIPLIER   1024

/*- Types -------------------------------------------------------------------*/
typedef struct
{
  uint32_t magic;
  int      size;
  int      version;
  int      count;

  int      power_cycles;
  int      charge_cycles;

  int      lcd_bl_level;

  bool     ac_coupling;
  bool     x10;

  int      trigger_mode;
  int      trigger_edge;
  int      trigger_level;
  int      trigger_level_mv;

  int      horizontal_scale;
  int64_t  horizontal_position;
  int      horizontal_position_px;
  int      horizontal_period;

  int      vertical_scale;
  int      vertical_position;
  int      vertical_position_mv;
  int      vertical_mult;

  int      sample_rate_limit;

  uint32_t padding[32];

  int      calib_channel_delta;
  int      calib_dac_zero;
  int      calib_dac_mult[VS_COUNT];
  int      calib_vs_mult[VS_COUNT];

  uint32_t crc;
} Config;

/*- Variables ---------------------------------------------------------------*/
extern Config config;

/*- Prototypes --------------------------------------------------------------*/
void config_init(void);
void config_reset(void);
void config_task(void);

#endif // _CONFIG_H_

