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

#ifndef _CAPTURE_H_
#define _CAPTURE_H_

/*- Definitions -------------------------------------------------------------*/
#define BASE_SAMPLE_RATE       125e6
#define BASE_SAMPLE_PERIOD     (1e9 / BASE_SAMPLE_RATE)
#define CAPTURE_BUFFER_SIZE    (128 * 1024)
#define TRIGGER_MARGIN_SAMPLES 1024
#define DATA_BUFFER_SIZE       300

/*- Types -------------------------------------------------------------------*/
typedef struct
{
  int      size;
  int      min_value;
  int      max_value;
  int      vertical_position;
  int      frequency;
  int      min[DATA_BUFFER_SIZE];
  int      max[DATA_BUFFER_SIZE];
  uint8_t  flags[DATA_BUFFER_SIZE];
} DataBuffer;

/*- Prototypes --------------------------------------------------------------*/
void capture_init(void);
void capture_disable_clock(void);
void capture_start(void);
void capture_stop(void);
void capture_set_vertical_parameters(void);
void capture_set_horizontal_parameters(int sr_divider, int trigger_offset);
void capture_set_trigger_level(int level);
void capture_set_trigger_edge(int edge);
void capture_set_trigger_mode(int mode);
int capture_get_state(void);
bool capture_buffer_updated(void);
void capture_get_data(DataBuffer *db);
void capture_get_raw_data(int *raw, int size);

#endif // _CAPTURE_H_


