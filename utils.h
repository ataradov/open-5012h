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

#ifndef _UTILS_H_
#define _UTILS_H_

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/*- Definitions -------------------------------------------------------------*/
#define ARRAY_SIZE(x)        ((int)(sizeof(x) / sizeof(0[x])))

/*- Prototypes --------------------------------------------------------------*/
void crc32_init(void);
uint32_t crc32_calc(uint32_t *data, int size);
char *format_time(int64_t value, bool show_plus_sign);
char *format_voltage(int value, bool show_plus_sign);
char *format_divisions(int value, bool show_plus_sign);
char *format_frequency(int value);
char *format_raw_data(int *data, int size);
char *format_sps(int value);

uint64_t round_divide(int64_t dividend, int64_t divisor);

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void delay_ms(int ms)
{
  uint32_t cycles = ms * F_CPU / 4 / 1000;

  asm volatile (R"asm(
    1: subs %[cycles], %[cycles], #1
       nop
       bne 1b
    )asm"
    : [cycles] "+r"(cycles)
  );
}

//-----------------------------------------------------------------------------
static inline void delay_cycles(int cycles)
{
  cycles /= 4;

  asm volatile (R"asm(
    1: subs %[cycles], %[cycles], #1
       nop
       bne 1b
    )asm"
    : [cycles] "+r"(cycles)
  );
}

//-----------------------------------------------------------------------------
static inline int rbit8(int value)
{
  return __RBIT(value) >> 24;
}

#endif // _UTILS_H_

