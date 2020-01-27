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

#ifndef _BUTTONS_H_
#define _BUTTONS_H_

/*- Definitions -------------------------------------------------------------*/
enum
{
  BTN_STOP         = (1 << 0),
  BTN_UP           = (1 << 1),
  BTN_EDGE         = (1 << 2),
  BTN_MODE         = (1 << 3),
  BTN_F1           = (1 << 4),
  BTN_RIGHT        = (1 << 5),
  BTN_AC_DC        = (1 << 6),
  BTN_AUTO         = (1 << 7),

  BTN_50P          = (1 << 8),
  BTN_MENU         = (1 << 9),
  BTN_TRIG_UP      = (1 << 10),
  BTN_LEFT         = (1 << 11),
  BTN_TRIG_DOWN    = (1 << 12),
  BTN_SAVE         = (1 << 13),
  BTN_TRIG         = (1 << 14),
  BTN_DOWN         = (1 << 15),

  BTN_F2           = (1 << 16),
  BTN_1X_10X       = (1 << 17),
  BTN_SHIFT        = (1 << 17),

  BTN_REPEAT       = (1 << 18),
};

/*- Prototypes --------------------------------------------------------------*/
void buttons_init(void);
int buttons_read(void);
int buttons_state(void);
void buttons_task(void);

void buttons_handler(int buttons);

#endif // _BUTTONS_H_


