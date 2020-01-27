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
#include "config.h"
#include "buffer.h"

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void buffer_decimate(uint32_t dst, uint32_t src, uint32_t count, uint32_t offset)
{
  asm volatile (R"asm(
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    cmp        %[offset], #1
    beq        8f
    cmp        %[offset], #2
    beq        16f
    cmp        %[offset], #3
    beq        24f

0:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    ubfx       b1, b1, #0, #8
    bfi        b0, b1, #8, #8
    ubfx       b2, b2, #0, #8
    bfi        b0, b2, #16, #8
    ubfx       b3, b3, #0, #8
    bfi        b0, b3, #24, #8
    ubfx       b5, b5, #0, #8
    bfi        b4, b5, #8, #8
    ubfx       b6, b6, #0, #8
    bfi        b4, b6, #16, #8
    ubfx       b7, b7, #0, #8
    bfi        b4, b7, #24, #8
    stm        %[dst]!, { b0, b4 }
    subs       %[count], #32
    bne        0b
    b          99f

8:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    ubfx       b0, b0, #8, #8
    bfi        b1, b0, #0, #8
    ubfx       b2, b2, #8, #8
    bfi        b1, b2, #16, #8
    ubfx       b3, b3, #8, #8
    bfi        b1, b3, #24, #8
    ubfx       b4, b4, #8, #8
    bfi        b5, b4, #0, #8
    ubfx       b6, b6, #8, #8
    bfi        b5, b6, #16, #8
    ubfx       b7, b7, #8, #8
    bfi        b5, b7, #24, #8
    stm        %[dst]!, { b1, b5 }
    subs       %[count], #32
    bne        8b
    b          99f

16:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    ubfx       b0, b0, #16, #8
    bfi        b2, b0, #0, #8
    ubfx       b1, b1, #16, #8
    bfi        b2, b1, #8, #8
    ubfx       b3, b3, #16, #8
    bfi        b2, b3, #24, #8
    ubfx       b4, b4, #16, #8
    bfi        b6, b4, #0, #8
    ubfx       b5, b5, #16, #8
    bfi        b6, b5, #8, #8
    ubfx       b7, b7, #16, #8
    bfi        b6, b7, #24, #8
    stm        %[dst]!, { b2, b6 }
    subs       %[count], #32
    bne        16b
    b          99f

24:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    ubfx       b0, b0, #24, #8
    bfi        b3, b0, #0, #8
    ubfx       b1, b1, #24, #8
    bfi        b3, b1, #8, #8
    ubfx       b2, b2, #24, #8
    bfi        b3, b2, #16, #8
    ubfx       b4, b4, #24, #8
    bfi        b7, b4, #0, #8
    ubfx       b5, b5, #24, #8
    bfi        b7, b5, #8, #8
    ubfx       b6, b6, #24, #8
    bfi        b7, b6, #16, #8
    stm        %[dst]!, { b3, b7 }
    subs       %[count], #32
    bne        24b

99:
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count)
    : [offset] "r" (offset)
    : "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );
}

//-----------------------------------------------------------------------------
static void buffer_reverse_add(uint32_t buf, uint32_t count, uint32_t delta)
{
  asm volatile (R"asm(
    t          .req r4
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    // Generate APSR.GE bits for select operation
    mov        b0, #0
    mov        b1, #0xff00ff00
    sadd8      b0, b0, b1

0:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    rbit       t, b0
    ror        t, #8
    sel        b0, b0, t
    uqadd8     b0, b0, %[delta]

    rbit       t, b1
    ror        t, #8
    sel        b1, b1, t
    uqadd8     b1, b1, %[delta]

    rbit       t, b2
    ror        t, #8
    sel        b2, b2, t
    uqadd8     b2, b2, %[delta]

    rbit       t, b3
    ror        t, #8
    sel        b3, b3, t
    uqadd8     b3, b3, %[delta]

    rbit       t, b4
    ror        t, #8
    sel        b4, b4, t
    uqadd8     b4, b4, %[delta]

    rbit       t, b5
    ror        t, #8
    sel        b5, b5, t
    uqadd8     b5, b5, %[delta]

    rbit       t, b6
    ror        t, #8
    sel        b6, b6, t
    uqadd8     b6, b6, %[delta]

    rbit       t, b7
    ror        t, #8
    sel        b7, b7, t
    uqadd8     b7, b7, %[delta]

    stm        %[dst]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    subs       %[count], #32
    bne        0b

    .unreq     t
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [src] "+r" (buf), [dst] "+r" (buf), [count] "+r" (count)
    : [delta] "r" (delta)
    : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );
}

//-----------------------------------------------------------------------------
static void buffer_reverse_sub(uint32_t buf, uint32_t count, uint32_t delta)
{
  asm volatile (R"asm(
    t          .req r4
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    // Generate APSR.GE bits for select operation
    mov        b0, #0
    mov        b1, #0xff00ff00
    sadd8      b0, b0, b1

0:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    rbit       t, b0
    ror        t, #8
    sel        b0, b0, t
    uqsub8     b0, b0, %[delta]

    rbit       t, b1
    ror        t, #8
    sel        b1, b1, t
    uqsub8     b1, b1, %[delta]

    rbit       t, b2
    ror        t, #8
    sel        b2, b2, t
    uqsub8     b2, b2, %[delta]

    rbit       t, b3
    ror        t, #8
    sel        b3, b3, t
    uqsub8     b3, b3, %[delta]

    rbit       t, b4
    ror        t, #8
    sel        b4, b4, t
    uqsub8     b4, b4, %[delta]

    rbit       t, b5
    ror        t, #8
    sel        b5, b5, t
    uqsub8     b5, b5, %[delta]

    rbit       t, b6
    ror        t, #8
    sel        b6, b6, t
    uqsub8     b6, b6, %[delta]

    rbit       t, b7
    ror        t, #8
    sel        b7, b7, t
    uqsub8     b7, b7, %[delta]

    stm        %[dst]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    subs       %[count], #32
    bne        0b

    .unreq     t
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [src] "+r" (buf), [dst] "+r" (buf), [count] "+r" (count)
    : [delta] "r" (delta)
    : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );
}


//-----------------------------------------------------------------------------
void buffer_reverse(uint32_t buf, uint32_t count)
{
  uint32_t delta;

  if (config.calib_channel_delta < 0)
  {
    delta = -config.calib_channel_delta;
    delta = (delta << 24) | (delta << 8);
    buffer_reverse_sub(buf, count, delta);
  }
  else
  {
    delta = config.calib_channel_delta;
    delta = (delta << 24) | (delta << 8);
    buffer_reverse_add(buf, count, delta);
  }
}

//-----------------------------------------------------------------------------
static void buffer_decimate_reverse_add(uint32_t dst, uint32_t src, uint32_t count, uint32_t offset, uint32_t delta)
{
  asm volatile (R"asm(
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    cmp        %[offset], #3
    beq        16f

8:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    rbit       b0, b0
    rbit       b1, b1
    rbit       b2, b2
    rbit       b3, b3
    rbit       b4, b4
    rbit       b5, b5
    rbit       b6, b6
    rbit       b7, b7
    ubfx       b0, b0, #16, #8
    bfi        b2, b0, #0, #8
    ubfx       b1, b1, #16, #8
    bfi        b2, b1, #8, #8
    ubfx       b3, b3, #16, #8
    bfi        b2, b3, #24, #8
    ubfx       b4, b4, #16, #8
    bfi        b6, b4, #0, #8
    ubfx       b5, b5, #16, #8
    bfi        b6, b5, #8, #8
    ubfx       b7, b7, #16, #8
    bfi        b6, b7, #24, #8
    uqadd8     b2, b2, %[delta]
    uqadd8     b6, b6, %[delta]
    stm        %[dst]!, { b2, b6 }
    subs       %[count], #32
    bne        8b
    b          99f

16:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    rbit       b0, b0
    rbit       b1, b1
    rbit       b2, b2
    rbit       b3, b3
    rbit       b4, b4
    rbit       b5, b5
    rbit       b6, b6
    rbit       b7, b7
    ubfx       b1, b1, #0, #8
    bfi        b0, b1, #8, #8
    ubfx       b2, b2, #0, #8
    bfi        b0, b2, #16, #8
    ubfx       b3, b3, #0, #8
    bfi        b0, b3, #24, #8
    ubfx       b5, b5, #0, #8
    bfi        b4, b5, #8, #8
    ubfx       b6, b6, #0, #8
    bfi        b4, b6, #16, #8
    ubfx       b7, b7, #0, #8
    bfi        b4, b7, #24, #8
    uqadd8     b0, b0, %[delta]
    uqadd8     b4, b4, %[delta]
    stm        %[dst]!, { b0, b4 }
    subs       %[count], #32
    bne        16b

99:
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count)
    : [offset] "r" (offset), [delta] "r" (delta)
    : "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );
}

//-----------------------------------------------------------------------------
static void buffer_decimate_reverse_sub(uint32_t dst, uint32_t src, uint32_t count, uint32_t offset, uint32_t delta)
{
  asm volatile (R"asm(
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    cmp        %[offset], #3
    beq        16f

8:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    rbit       b0, b0
    rbit       b1, b1
    rbit       b2, b2
    rbit       b3, b3
    rbit       b4, b4
    rbit       b5, b5
    rbit       b6, b6
    rbit       b7, b7
    ubfx       b0, b0, #16, #8
    bfi        b2, b0, #0, #8
    ubfx       b1, b1, #16, #8
    bfi        b2, b1, #8, #8
    ubfx       b3, b3, #16, #8
    bfi        b2, b3, #24, #8
    ubfx       b4, b4, #16, #8
    bfi        b6, b4, #0, #8
    ubfx       b5, b5, #16, #8
    bfi        b6, b5, #8, #8
    ubfx       b7, b7, #16, #8
    bfi        b6, b7, #24, #8
    uqsub8     b2, b2, %[delta]
    uqsub8     b6, b6, %[delta]
    stm        %[dst]!, { b2, b6 }
    subs       %[count], #32
    bne        8b
    b          99f

16:
    ldm        %[src]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    rbit       b0, b0
    rbit       b1, b1
    rbit       b2, b2
    rbit       b3, b3
    rbit       b4, b4
    rbit       b5, b5
    rbit       b6, b6
    rbit       b7, b7
    ubfx       b1, b1, #0, #8
    bfi        b0, b1, #8, #8
    ubfx       b2, b2, #0, #8
    bfi        b0, b2, #16, #8
    ubfx       b3, b3, #0, #8
    bfi        b0, b3, #24, #8
    ubfx       b5, b5, #0, #8
    bfi        b4, b5, #8, #8
    ubfx       b6, b6, #0, #8
    bfi        b4, b6, #16, #8
    ubfx       b7, b7, #0, #8
    bfi        b4, b7, #24, #8
    uqsub8     b0, b0, %[delta]
    uqsub8     b4, b4, %[delta]
    stm        %[dst]!, { b0, b4 }
    subs       %[count], #32
    bne        16b

99:
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [src] "+r" (src), [dst] "+r" (dst), [count] "+r" (count)
    : [offset] "r" (offset), [delta] "r" (delta)
    : "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );
}

//-----------------------------------------------------------------------------
void buffer_decimate_reverse(uint32_t dst, uint32_t src, uint32_t count, uint32_t offset)
{
  uint32_t delta;

  if (config.calib_channel_delta < 0)
  {
    delta = -config.calib_channel_delta;
    delta = (delta << 24) | (delta << 16) | (delta << 8) | delta;
    buffer_decimate_reverse_sub(dst, src, count, offset, delta);
  }
  else
  {
    delta = config.calib_channel_delta;
    delta = (delta << 24) | (delta << 16) | (delta << 8) | delta;
    buffer_decimate_reverse_add(dst, src, count, offset, delta);
  }
}

