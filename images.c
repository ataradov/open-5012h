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
#include "images.h"
#include "lcd.h"

/*- Constants ---------------------------------------------------------------*/
#define __     LCD_COLOR(0, 0, 0)        // Background
#define WT     LCD_COLOR(255, 255, 255)  // White
#define OR     LCD_COLOR(255, 180, 50)   // Orange
#define YL     LCD_COLOR(255, 255, 0)    // Yellow
#define CY     LCD_COLOR(50, 255, 255)   // Cyan-ish

const Image image_ac =
{
  .width  = 8,
  .height = 16,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, YL, YL, __, __, __, __,
    __, YL, __, __, YL, __, __, YL,
    __, __, __, __, __, YL, YL, __,
    __, __, __, __, __, __, __, __,
    __, __, YL, __, __, __, YL, __,
    __, YL, __, YL, __, YL, __, YL,
    __, YL, __, YL, __, YL, __, __,
    __, YL, YL, YL, __, YL, __, __,
    __, YL, __, YL, __, YL, __, YL,
    __, YL, __, YL, __, __, YL, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
  }
};

const Image image_dc =
{
  .width  = 8,
  .height = 16,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, YL, __, YL, __, YL, __, YL,
    __, __, __, __, __, __, __, __,
    __, YL, YL, YL, YL, YL, YL, YL,
    __, __, __, __, __, __, __, __,
    __, YL, YL, __, __, __, YL, __,
    __, YL, __, YL, __, YL, __, YL,
    __, YL, __, YL, __, YL, __, __,
    __, YL, __, YL, __, YL, __, __,
    __, YL, __, YL, __, YL, __, YL,
    __, YL, YL, __, __, __, YL, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
  }
};

const Image image_trigger_edge_rise =
{
  .width  = 8,
  .height = 16,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, CY, CY, CY,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, CY, CY, CY, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
  }
};

const Image image_trigger_edge_fall =
{
  .width  = 8,
  .height = 16,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, CY, CY, CY, __, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, __, __, CY, __, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, __, CY, __, __,
    __, __, __, __, __, CY, CY, CY,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
  }
};

const Image image_trigger_edge_both =
{
  .width  = 8,
  .height = 16,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, CY, CY, CY, __, CY, CY, CY,
    __, __, __, CY, __, CY, __, __,
    __, __, __, CY, __, CY, __, __,
    __, __, __, CY, __, CY, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, __, CY, __, __, __,
    __, __, __, CY, __, CY, __, __,
    __, __, __, CY, __, CY, __, __,
    __, __, __, CY, __, CY, __, __,
    __, CY, CY, CY, __, CY, CY, CY,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
    __, __, __, __, __, __, __, __,
  }
};

const Image image_trigger_level =
{
  .width  = 8,
  .height = 7,
  .ox     = 0,
  .oy     = 3,
  .data = {
    __, __, __, OR, OR, OR, OR, OR,
    __, __, OR, OR, OR, OR, OR, OR,
    __, OR, OR, OR, OR, OR, OR, OR,
    OR, OR, OR, OR, OR, OR, OR, OR,
    __, OR, OR, OR, OR, OR, OR, OR,
    __, __, OR, OR, OR, OR, OR, OR,
    __, __, __, OR, OR, OR, OR, OR,
  }
};

const Image image_trigger_offset =
{
  .width  = 7,
  .height = 7,
  .ox     = 3,
  .oy     = 0,
  .data = {
    __, __, __, WT, __, __, __,
    __, __, __, WT, __, __, __,
    __, __, __, WT, __, __, __,
    WT, WT, WT, WT, WT, WT, WT,
    __, WT, WT, WT, WT, WT, __,
    __, __, WT, WT, WT, __, __,
    __, __, __, WT, __, __, __,
  }
};

const Image image_trigger_offset_left =
{
  .width  = 7,
  .height = 7,
  .ox     = 0,
  .oy     = 0,
  .data = {
    __, __, __, WT, __, __, __,
    __, __, WT, WT, __, __, __,
    __, WT, WT, WT, __, __, __,
    WT, WT, WT, WT, WT, WT, WT,
    __, WT, WT, WT, __, __, __,
    __, __, WT, WT, __, __, __,
    __, __, __, WT, __, __, __,
  }
};

const Image image_trigger_offset_right =
{
  .width  = 7,
  .height = 7,
  .ox     = 6,
  .oy     = 0,
  .data = {
    __, __, __, WT, __, __, __,
    __, __, __, WT, WT, __, __,
    __, __, __, WT, WT, WT, __,
    WT, WT, WT, WT, WT, WT, WT,
    __, __, __, WT, WT, WT, __,
    __, __, __, WT, WT, __, __,
    __, __, __, WT, __, __, __,
  }
};

const Image image_trigger_mv =
{
  .width  = 5,
  .height = 5,
  .ox     = 2,
  .oy     = 4,
  .data = {
    OR, OR, OR, OR, OR,
    OR, OR, OR, OR, OR,
    OR, OR, OR, OR, OR,
    __, OR, OR, OR, __,
    __, __, OR, __, __,
  }
};

const Image image_reference_level =
{
  .width  = 8,
  .height = 7,
  .ox     = 7,
  .oy     = 3,
  .data = {
    YL, YL, YL, YL, YL, __, __, __,
    YL, YL, YL, YL, YL, YL, __, __,
    YL, YL, YL, YL, YL, YL, YL, __,
    YL, YL, YL, YL, YL, YL, YL, YL,
    YL, YL, YL, YL, YL, YL, YL, __,
    YL, YL, YL, YL, YL, YL, __, __,
    YL, YL, YL, YL, YL, __, __, __,
  }
};

const Image image_reference_level_up =
{
  .width  = 7,
  .height = 4,
  .ox     = 3,
  .oy     = 0,
  .data = {
    __, __, __, YL, __, __, __,
    __, __, YL, YL, YL, __, __,
    __, YL, YL, YL, YL, YL, __,
    YL, YL, YL, YL, YL, YL, YL,
  }
};

const Image image_reference_level_down =
{
  .width  = 7,
  .height = 4,
  .ox     = 3,
  .oy     = 3,
  .data = {
    YL, YL, YL, YL, YL, YL, YL,
    __, YL, YL, YL, YL, YL, __,
    __, __, YL, YL, YL, __, __,
    __, __, __, YL, __, __, __,
  }
};


