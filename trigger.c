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
#include "trigger.h"

/*- Definitions -------------------------------------------------------------*/
#define TRIGGER_HYSTERESIS     0x03030303

/*- Variables ---------------------------------------------------------------*/
static volatile uint32_t g_trigger_levels;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
void trigger_set_levels(int level)
{
  g_trigger_levels = (level << 24) | (level << 16) | (level << 8) | level;
}

//-----------------------------------------------------------------------------
int trigger_find_rise_single(uint32_t buf, uint32_t count)
{
  asm volatile (R"asm(
    t          .req r3
    u          .req r4
    v          .req r5 // Same as b0
    x          .req r6 // Same as b1
    y          .req r7 // Same as b2
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    ldrb       u, [%[buf]]
    ubfx       t, %[triggers], #0, #8
    sub        t, %[hysteresis] & 0xff
    cmp        u, t
    bls        30f

    mov        x, %[hysteresis]
    uqsub8     %[triggers], %[triggers], x

    // First sample is above the trigger, wait until it gets below
    // the trigger for at least one sample
0:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    uqsub8     t, %[triggers], b0
    cbnz       t, 10f
    uqsub8     t, %[triggers], b1
    cbnz       t, 11f
    uqsub8     t, %[triggers], b2
    cbnz       t, 12f
    uqsub8     t, %[triggers], b3
    cbnz       t, 13f
    uqsub8     t, %[triggers], b4
    cbnz       t, 14f
    uqsub8     t, %[triggers], b5
    cbnz       t, 15f
    uqsub8     t, %[triggers], b6
    cbnz       t, 16f
    uqsub8     t, %[triggers], b7
    cbnz       t, 17f
    subs       %[count], #32
    bne        0b
    b          99f

10:
    mov        u, #0
    mov        v, b0 // NOTE: v and b0 are the same register already
    ldr        lr, =31f+1
    b          20f
11:
    mov        u, #4
    mov        v, b1
    ldr        lr, =32f+1
    b          20f
12:
    mov        u, #8
    mov        v, b2
    ldr        lr, =33f+1
    b          20f
13:
    mov        u, #12
    mov        v, b3
    ldr        lr, =34f+1
    b          20f
14:
    mov        u, #16
    mov        v, b4
    ldr        lr, =35f+1
    b          20f
15:
    mov        u, #20
    mov        v, b5
    ldr        lr, =36f+1
    b          20f
16:
    mov        u, #24
    mov        v, b6
    ldr        lr, =37f+1
    b          20f
17:
    mov        u, #28
    mov        v, b7
    ldr        lr, =38f+1

20:
    // Check if the remaining bytes contain edge transition
    // t = trigger compare results, >0 - below trigger
    // u = offset into 32-byte block
    // v = buffer value (b0-b7)
    // lr = continuation address
    push       { x, y }

    mov        x, %[hysteresis]
    uqadd8     %[triggers], %[triggers], x

    ubfx       y, %[triggers], #0, #8 // y contains single trigger level value

    // Find the first sample below the trigger level
    ubfx       x, t, #0, #8
    cbnz       x, 21f
    ubfx       x, t, #8, #8
    cbnz       x, 22f
    ubfx       x, t, #16, #8
    cbnz       x, 23f
    b          24f

    // Check if any of the following samples are above the trigger
21:
    ubfx       x, v, #8, #8
    cmp        x, y
    bgt        29f
22:
    ubfx       x, v, #16, #8
    cmp        x, y
    bgt        28f
23:
    ubfx       x, v, #24, #8
    cmp        x, y
    bgt        27f
24:
    pop        { x, y }
    bx         lr

    // Found the trigger condition in the same word
27:
    subs       %[count], #1
28:
    subs       %[count], #1
29:
    subs       %[count], #1
    subs       %[count], u
    pop        { x, y }
    b          99f

    // Look for the rising trigger condition
30:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, b0, %[triggers]
    cbnz       t, 48f
31:
    uqsub8     t, b1, %[triggers]
    cbnz       t, 41f
32:
    uqsub8     t, b2, %[triggers]
    cbnz       t, 42f
33:
    uqsub8     t, b3, %[triggers]
    cbnz       t, 43f
34:
    uqsub8     t, b4, %[triggers]
    cbnz       t, 44f
35:
    uqsub8     t, b5, %[triggers]
    cbnz       t, 45f
36:
    uqsub8     t, b6, %[triggers]
    cbnz       t, 46f
37:
    uqsub8     t, b7, %[triggers]
    cbnz       t, 47f
38:
    subs       %[count], #32
    bne        30b
    b          99f

41:
    subs       %[count], #4
    b          48f
42:
    subs       %[count], #8
    b          48f
43:
    subs       %[count], #12
    b          48f
44:
    subs       %[count], #16
    b          48f
45:
    subs       %[count], #20
    b          48f
46:
    subs       %[count], #24
    b          48f
47:
    subs       %[count], #28
    b          48f

48:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     u
    .unreq     v
    .unreq     x
    .unreq     y
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr"
  );

  return count;
}

//-----------------------------------------------------------------------------
int trigger_find_fall_single(uint32_t buf, uint32_t count)
{
  asm volatile (R"asm(
    t          .req r3
    u          .req r4
    v          .req r5 // Same as b0
    x          .req r6 // Same as b1
    y          .req r7 // Same as b2
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    ldrb       u, [%[buf]]
    ubfx       t, %[triggers], #0, #8
    add        t, %[hysteresis] & 0xff
    cmp        u, t
    bgt        30f

    mov        x, %[hysteresis]
    uqadd8     %[triggers], %[triggers], x

    // First sample is below the trigger, wait until it gets above
    // the trigger for at least one sample
0:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    uqsub8     t, b0, %[triggers]
    cbnz       t, 10f
    uqsub8     t, b1, %[triggers]
    cbnz       t, 11f
    uqsub8     t, b2, %[triggers]
    cbnz       t, 12f
    uqsub8     t, b3, %[triggers]
    cbnz       t, 13f
    uqsub8     t, b4, %[triggers]
    cbnz       t, 14f
    uqsub8     t, b5, %[triggers]
    cbnz       t, 15f
    uqsub8     t, b6, %[triggers]
    cbnz       t, 16f
    uqsub8     t, b7, %[triggers]
    cbnz       t, 17f
    subs       %[count], #32
    bne        0b
    b          99f

10:
    mov        u, #0
    mov        v, b0 // NOTE: v and b0 are the same register already
    ldr        lr, =31f+1
    b          20f
11:
    mov        u, #4
    mov        v, b1
    ldr        lr, =32f+1
    b          20f
12:
    mov        u, #8
    mov        v, b2
    ldr        lr, =33f+1
    b          20f
13:
    mov        u, #12
    mov        v, b3
    ldr        lr, =34f+1
    b          20f
14:
    mov        u, #16
    mov        v, b4
    ldr        lr, =35f+1
    b          20f
15:
    mov        u, #20
    mov        v, b5
    ldr        lr, =36f+1
    b          20f
16:
    mov        u, #24
    mov        v, b6
    ldr        lr, =37f+1
    b          20f
17:
    mov        u, #28
    mov        v, b7
    ldr        lr, =38f+1

20:
    // Check if the remaining bytes contain edge transition
    // t = trigger compare results, >0 - above trigger
    // u = offset into 32-byte block
    // v = buffer value (b0-b7)
    // lr = continuation address
    push       { x, y }

    mov        x, %[hysteresis]
    uqsub8     %[triggers], %[triggers], x

    ubfx       y, %[triggers], #0, #8 // y contains single trigger level value

    // Find the first sample above the trigger level
    ubfx       x, t, #0, #8
    cbnz       x, 21f
    ubfx       x, t, #8, #8
    cbnz       x, 22f
    ubfx       x, t, #16, #8
    cbnz       x, 23f
    b          24f

    // Check if any of the following samples are below the trigger
21:
    ubfx       x, v, #8, #8
    cmp        x, y
    bls        29f
22:
    ubfx       x, v, #16, #8
    cmp        x, y
    bls        28f
23:
    ubfx       x, v, #24, #8
    cmp        x, y
    bls        27f
24:
    pop        { x, y }
    bx         lr

    // Found the trigger condition in the same word
27:
    subs       %[count], #1
28:
    subs       %[count], #1
29:
    subs       %[count], #1
    subs       %[count], u
    pop        { x, y }
    b          99f

    // Look for the falling trigger condition
30:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, %[triggers], b0
    cbnz       t, 48f
31:
    uqsub8     t, %[triggers], b1
    cbnz       t, 41f
32:
    uqsub8     t, %[triggers], b2
    cbnz       t, 42f
33:
    uqsub8     t, %[triggers], b3
    cbnz       t, 43f
34:
    uqsub8     t, %[triggers], b4
    cbnz       t, 44f
35:
    uqsub8     t, %[triggers], b5
    cbnz       t, 45f
36:
    uqsub8     t, %[triggers], b6
    cbnz       t, 46f
37:
    uqsub8     t, %[triggers], b7
    cbnz       t, 47f
38:
    subs       %[count], #32
    bne        30b
    b          99f

41:
    subs       %[count], #4
    b          48f
42:
    subs       %[count], #8
    b          48f
43:
    subs       %[count], #12
    b          48f
44:
    subs       %[count], #16
    b          48f
45:
    subs       %[count], #20
    b          48f
46:
    subs       %[count], #24
    b          48f
47:
    subs       %[count], #28
    b          48f

48:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     u
    .unreq     v
    .unreq     x
    .unreq     y
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr"
  );

  return count;
}

//-----------------------------------------------------------------------------
int trigger_find_both_single(uint32_t buf, uint32_t count)
{
  asm volatile (R"asm(
    t          .req r2
    h          .req r3
    l          .req r4
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    ldrb       h, [%[buf]]

    ubfx       t, %[triggers], #0, #8
    sub        t, %[hysteresis] & 0xff
    cmp        h, t
    bls        40f // Look for the rising trigger condition

    ubfx       t, %[triggers], #0, #8
    add        t, %[hysteresis] & 0xff
    cmp        h, t
    bgt        60f // Look for the falling trigger condition

    mov        t, %[hysteresis]
    uqadd8     h, %[triggers], t
    uqsub8     l, %[triggers], t

0:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, l, b0
    cbnz       t, 41f
    uqsub8     t, b0, h
    cbnz       t, 11f

    uqsub8     t, l, b1
    cbnz       t, 42f
    uqsub8     t, b1, h
    cbnz       t, 12f

    uqsub8     t, l, b2
    cbnz       t, 43f
    uqsub8     t, b2, h
    cbnz       t, 13f

    uqsub8     t, l, b3
    cbnz       t, 44f
    uqsub8     t, b3, h
    cbnz       t, 14f

    uqsub8     t, l, b4
    cbnz       t, 45f
    uqsub8     t, b4, h
    cbnz       t, 15f

    uqsub8     t, l, b5
    cbnz       t, 46f
    uqsub8     t, b5, h
    cbnz       t, 16f

    uqsub8     t, l, b6
    cbnz       t, 47f
    uqsub8     t, b6, h
    cbnz       t, 17f

    uqsub8     t, l, b7
    cbnz       t, 48f
    uqsub8     t, b7, h
    cbnz       t, 18f

    subs       %[count], #32
    bne        0b
    b          99f

11:
    b          61f
12:
    b          62f
13:
    b          63f
14:
    b          64f
15:
    b          65f
16:
    b          66f
17:
    b          67f
18:
    b          68f

    // Look for the rising trigger condition
40:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, b0, %[triggers]
    cbnz       t, 58f
41:
    uqsub8     t, b1, %[triggers]
    cbnz       t, 51f
42:
    uqsub8     t, b2, %[triggers]
    cbnz       t, 52f
43:
    uqsub8     t, b3, %[triggers]
    cbnz       t, 53f
44:
    uqsub8     t, b4, %[triggers]
    cbnz       t, 54f
45:
    uqsub8     t, b5, %[triggers]
    cbnz       t, 55f
46:
    uqsub8     t, b6, %[triggers]
    cbnz       t, 56f
47:
    uqsub8     t, b7, %[triggers]
    cbnz       t, 57f
48:
    subs       %[count], #32
    bne        40b
    b          99f

    // This code is the same as the code below. This is done to be
    // in the range of the cbnz instruction.
51:
    subs       %[count], #4
    b          78f
52:
    subs       %[count], #8
    b          78f
53:
    subs       %[count], #12
    b          78f
54:
    subs       %[count], #16
    b          78f
55:
    subs       %[count], #20
    b          78f
56:
    subs       %[count], #24
    b          78f
57:
    subs       %[count], #28
    b          78f
58:
    b          78f

    // Look for the falling trigger condition
60:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, %[triggers], b0
    cbnz       t, 78f
61:
    uqsub8     t, %[triggers], b1
    cbnz       t, 71f
62:
    uqsub8     t, %[triggers], b2
    cbnz       t, 72f
63:
    uqsub8     t, %[triggers], b3
    cbnz       t, 73f
64:
    uqsub8     t, %[triggers], b4
    cbnz       t, 74f
65:
    uqsub8     t, %[triggers], b5
    cbnz       t, 75f
66:
    uqsub8     t, %[triggers], b6
    cbnz       t, 76f
67:
    uqsub8     t, %[triggers], b7
    cbnz       t, 77f
68:
    subs       %[count], #32
    bne        60b
    b          99f

71:
    subs       %[count], #4
    b          78f
72:
    subs       %[count], #8
    b          78f
73:
    subs       %[count], #12
    b          78f
74:
    subs       %[count], #16
    b          78f
75:
    subs       %[count], #20
    b          78f
76:
    subs       %[count], #24
    b          78f
77:
    subs       %[count], #28
    b          78f

78:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     h
    .unreq     l
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12"
  );

  return count;
}

//-----------------------------------------------------------------------------
int trigger_find_rise_dual(uint32_t buf, uint32_t count)
{
  // NOTE: The code of this function is essentyally the same as trigger_find_rise_single()
  // The differences are:
  //   1. "ands t, #0x00ff00ff" on all values to ignore even samples
  //   2. The switchover code is modified to ignore even samples

  asm volatile (R"asm(
    t          .req r3
    u          .req r4
    v          .req r5 // Same as b0
    x          .req r6 // Same as b1
    y          .req r7 // Same as b2
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    ldrb       u, [%[buf]]
    ubfx       t, %[triggers], #0, #8
    sub        t, %[hysteresis] & 0xff
    cmp        u, t
    bls        30f

    mov        x, %[hysteresis]
    uqsub8     %[triggers], %[triggers], x

    // First sample is above the trigger, wait until it gets below
    // the trigger for at least one sample
0:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    uqsub8     t, %[triggers], b0
    ands       t, #0x00ff00ff
    cbnz       t, 10f

    uqsub8     t, %[triggers], b1
    ands       t, #0x00ff00ff
    cbnz       t, 11f

    uqsub8     t, %[triggers], b2
    ands       t, #0x00ff00ff
    cbnz       t, 12f

    uqsub8     t, %[triggers], b3
    ands       t, #0x00ff00ff
    cbnz       t, 13f

    uqsub8     t, %[triggers], b4
    ands       t, #0x00ff00ff
    cbnz       t, 14f

    uqsub8     t, %[triggers], b5
    ands       t, #0x00ff00ff
    cbnz       t, 15f

    uqsub8     t, %[triggers], b6
    ands       t, #0x00ff00ff
    cbnz       t, 16f

    uqsub8     t, %[triggers], b7
    ands       t, #0x00ff00ff
    cbnz       t, 17f

    subs       %[count], #32
    bne        0b
    b          99f

10:
    mov        u, #0
    mov        v, b0 // NOTE: v and b0 are the same register already
    ldr        lr, =31f+1
    b          20f
11:
    mov        u, #4
    mov        v, b1
    ldr        lr, =32f+1
    b          20f
12:
    mov        u, #8
    mov        v, b2
    ldr        lr, =33f+1
    b          20f
13:
    mov        u, #12
    mov        v, b3
    ldr        lr, =34f+1
    b          20f
14:
    mov        u, #16
    mov        v, b4
    ldr        lr, =35f+1
    b          20f
15:
    mov        u, #20
    mov        v, b5
    ldr        lr, =36f+1
    b          20f
16:
    mov        u, #24
    mov        v, b6
    ldr        lr, =37f+1
    b          20f
17:
    mov        u, #28
    mov        v, b7
    ldr        lr, =38f+1

20:
    // Check if the remaining bytes contain edge transition
    // t = trigger compare results, >0 - below trigger
    // u = offset into 32-byte block
    // v = buffer value (b0-b7)
    // lr = continuation address
    push       { x, y }

    mov        x, %[hysteresis]
    uqadd8     %[triggers], %[triggers], x

    ubfx       y, %[triggers], #0, #8 // y contains single trigger level value

    // Find the first sample below the trigger level
    ubfx       x, t, #0, #8
    cbz        x, 24f

    // Check if the following sample is above the trigger
    ubfx       x, v, #16, #8
    cmp        x, y
    bgt        28f
24:
    pop        { x, y }
    bx         lr

    // Found the trigger condition in the same word
28:
    subs       %[count], #2
    subs       %[count], u
    pop        { x, y }
    b          99f

    // Look for the rising trigger condition
30:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, b0, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 48f
31:
    uqsub8     t, b1, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 41f
32:
    uqsub8     t, b2, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 42f
33:
    uqsub8     t, b3, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 43f
34:
    uqsub8     t, b4, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 44f
35:
    uqsub8     t, b5, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 45f
36:
    uqsub8     t, b6, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 46f
37:
    uqsub8     t, b7, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 47f
38:
    subs       %[count], #32
    bne        30b
    b          99f

41:
    subs       %[count], #4
    b          48f
42:
    subs       %[count], #8
    b          48f
43:
    subs       %[count], #12
    b          48f
44:
    subs       %[count], #16
    b          48f
45:
    subs       %[count], #20
    b          48f
46:
    subs       %[count], #24
    b          48f
47:
    subs       %[count], #28
    b          48f

48:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     u
    .unreq     v
    .unreq     x
    .unreq     y
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr"
  );

  return count;
}

//-----------------------------------------------------------------------------
int trigger_find_fall_dual(uint32_t buf, uint32_t count)
{
  // NOTE: The code of this function is essentyally the same as trigger_find_fall_single()
  // The differences are:
  //   1. "ands t, #0x00ff00ff" on all values to ignore even samples
  //   2. The switchover code is modified to ignore even samples

  asm volatile (R"asm(
    t          .req r3
    u          .req r4
    v          .req r5 // Same as b0
    x          .req r6 // Same as b1
    y          .req r7 // Same as b2
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8
    b4         .req r9
    b5         .req r10
    b6         .req r11
    b7         .req r12

    ldrb       u, [%[buf]]
    ubfx       t, %[triggers], #0, #8
    add        t, %[hysteresis] & 0xff
    cmp        u, t
    bgt        30f

    mov        x, %[hysteresis]
    uqadd8     %[triggers], %[triggers], x

    // First sample is below the trigger, wait until it gets above
    // the trigger for at least one sample
0:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }
    uqsub8     t, b0, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 10f

    uqsub8     t, b1, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 11f

    uqsub8     t, b2, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 12f

    uqsub8     t, b3, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 13f

    uqsub8     t, b4, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 14f

    uqsub8     t, b5, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 15f

    uqsub8     t, b6, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 16f

    uqsub8     t, b7, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 17f

    subs       %[count], #32
    bne        0b
    b          99f

10:
    mov        u, #0
    mov        v, b0 // NOTE: v and b0 are the same register already
    ldr        lr, =31f+1
    b          20f
11:
    mov        u, #4
    mov        v, b1
    ldr        lr, =32f+1
    b          20f
12:
    mov        u, #8
    mov        v, b2
    ldr        lr, =33f+1
    b          20f
13:
    mov        u, #12
    mov        v, b3
    ldr        lr, =34f+1
    b          20f
14:
    mov        u, #16
    mov        v, b4
    ldr        lr, =35f+1
    b          20f
15:
    mov        u, #20
    mov        v, b5
    ldr        lr, =36f+1
    b          20f
16:
    mov        u, #24
    mov        v, b6
    ldr        lr, =37f+1
    b          20f
17:
    mov        u, #28
    mov        v, b7
    ldr        lr, =38f+1

20:
    // Check if the remaining bytes contain edge transition
    // t = trigger compare results, >0 - above trigger
    // u = offset into 32-byte block
    // v = buffer value (b0-b7)
    // lr = continuation address
    push       { x, y }

    mov        x, %[hysteresis]
    uqsub8     %[triggers], %[triggers], x

    ubfx       y, %[triggers], #0, #8 // y contains single trigger level value

    // Find the first sample above the trigger level
    ubfx       x, t, #0, #8
    cbnz       x, 24f

    // Check if the following sample is above the trigger
    ubfx       x, v, #16, #8
    cmp        x, y
    bls        28f
24:
    pop        { x, y }
    bx         lr

    // Found the trigger condition in the same word
28:
    subs       %[count], #2
    subs       %[count], u
    pop        { x, y }
    b          99f

    // Look for the falling trigger condition
30:
    ldm        %[buf]!, { b0, b1, b2, b3, b4, b5, b6, b7 }

    uqsub8     t, %[triggers], b0
    ands       t, #0x00ff00ff
    cbnz       t, 48f
31:
    uqsub8     t, %[triggers], b1
    ands       t, #0x00ff00ff
    cbnz       t, 41f
32:
    uqsub8     t, %[triggers], b2
    ands       t, #0x00ff00ff
    cbnz       t, 42f
33:
    uqsub8     t, %[triggers], b3
    ands       t, #0x00ff00ff
    cbnz       t, 43f
34:
    uqsub8     t, %[triggers], b4
    ands       t, #0x00ff00ff
    cbnz       t, 44f
35:
    uqsub8     t, %[triggers], b5
    ands       t, #0x00ff00ff
    cbnz       t, 45f
36:
    uqsub8     t, %[triggers], b6
    ands       t, #0x00ff00ff
    cbnz       t, 46f
37:
    uqsub8     t, %[triggers], b7
    ands       t, #0x00ff00ff
    cbnz       t, 47f
38:
    subs       %[count], #32
    bne        30b
    b          99f

41:
    subs       %[count], #4
    b          48f
42:
    subs       %[count], #8
    b          48f
43:
    subs       %[count], #12
    b          48f
44:
    subs       %[count], #16
    b          48f
45:
    subs       %[count], #20
    b          48f
46:
    subs       %[count], #24
    b          48f
47:
    subs       %[count], #28
    b          48f

48:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     u
    .unreq     v
    .unreq     x
    .unreq     y
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    .unreq     b4
    .unreq     b5
    .unreq     b6
    .unreq     b7
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "lr"
  );

  return count;
}

//-----------------------------------------------------------------------------
int trigger_find_both_dual(uint32_t buf, uint32_t count)
{
  asm volatile (R"asm(
    t          .req r2
    h          .req r3
    l          .req r4
    b0         .req r5
    b1         .req r6
    b2         .req r7
    b3         .req r8

    ldrb       h, [%[buf]]

    ubfx       t, %[triggers], #0, #8
    sub        t, %[hysteresis] & 0xff
    cmp        h, t
    bls        40f // Look for the rising trigger condition

    ubfx       t, %[triggers], #0, #8
    add        t, %[hysteresis] & 0xff
    cmp        h, t
    bgt        60f // Look for the falling trigger condition

    mov        t, %[hysteresis]
    uqadd8     h, %[triggers], t
    uqsub8     l, %[triggers], t

0:
    ldm        %[buf]!, { b0, b1, b2, b3 }

    uqsub8     t, l, b0
    ands       t, #0x00ff00ff
    cbnz       t, 41f
    uqsub8     t, b0, h
    ands       t, #0x00ff00ff
    cbnz       t, 59f

    uqsub8     t, l, b1
    ands       t, #0x00ff00ff
    cbnz       t, 42f
    uqsub8     t, b1, h
    ands       t, #0x00ff00ff
    cbnz       t, 62f

    uqsub8     t, l, b2
    ands       t, #0x00ff00ff
    cbnz       t, 43f
    uqsub8     t, b2, h
    ands       t, #0x00ff00ff
    cbnz       t, 63f

    uqsub8     t, l, b3
    ands       t, #0x00ff00ff
    cbnz       t, 44f
    uqsub8     t, b3, h
    ands       t, #0x00ff00ff
    cbnz       t, 64f

    subs       %[count], #16
    bne        0b
    b          99f

    // Look for the rising trigger condition
40:
    ldm        %[buf]!, { b0, b1, b2, b3 }

    uqsub8     t, b0, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 78f //58f
41:
    uqsub8     t, b1, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 71f //51f
42:
    uqsub8     t, b2, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 72f//52f
43:
    uqsub8     t, b3, %[triggers]
    ands       t, #0x00ff00ff
    cbnz       t, 73f//53f
44:
    subs       %[count], #16
    bne        40b
    b          99f

59: // 61f is out of the cbnz range, so we add trampoline
    b          61f

    // Look for the falling trigger condition
60:
    ldm        %[buf]!, { b0, b1, b2, b3 }

    uqsub8     t, %[triggers], b0
    ands       t, #0x00ff00ff
    cbnz       t, 78f
61:
    uqsub8     t, %[triggers], b1
    ands       t, #0x00ff00ff
    cbnz       t, 71f
62:
    uqsub8     t, %[triggers], b2
    ands       t, #0x00ff00ff
    cbnz       t, 72f
63:
    uqsub8     t, %[triggers], b3
    ands       t, #0x00ff00ff
    cbnz       t, 73f
64:
    subs       %[count], #16
    bne        60b
    b          99f

71:
    subs       %[count], #4
    b          78f
72:
    subs       %[count], #8
    b          78f
73:
    subs       %[count], #12
78:
    rbit       t, t
    clz        t, t
    lsr        t, #3
    subs       %[count], t

99:
    .unreq     t
    .unreq     h
    .unreq     l
    .unreq     b0
    .unreq     b1
    .unreq     b2
    .unreq     b3
    )asm"
    : [buf] "+r" (buf), [count] "+r" (count)
    : [triggers] "r" (g_trigger_levels), [hysteresis] "I" (TRIGGER_HYSTERESIS)
    : "r2", "r3", "r4", "r5", "r6", "r7", "r8"
  );

  return count;
}


