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
#include "fonts.h"

/*- Constants ---------------------------------------------------------------*/

// --- 6x8 ---
const Font font_6x8 =
{
  .width  = 6,
  .height = 8,
  .pitch  = 6,
  .data   = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // " "
    0x04, 0x41, 0x10, 0x04, 0x40, 0x00, // "!"
    0x8a, 0xa2, 0x00, 0x00, 0x00, 0x00, // """
    0x8a, 0xf2, 0x29, 0x9f, 0xa2, 0x00, // "#"
    0x84, 0x57, 0x38, 0xd4, 0x43, 0x00, // "$"
    0xc3, 0x84, 0x10, 0x42, 0x86, 0x01, // "%"
    0x46, 0x52, 0x08, 0x55, 0x62, 0x01, // "&"
    0x06, 0x21, 0x00, 0x00, 0x00, 0x00, // "'"
    0x08, 0x21, 0x08, 0x02, 0x81, 0x00, // "("
    0x02, 0x81, 0x20, 0x08, 0x21, 0x00, // ")"
    0x80, 0x42, 0x7c, 0x84, 0x02, 0x00, // "*"
    0x00, 0x41, 0x7c, 0x04, 0x01, 0x00, // "+"
    0x00, 0x00, 0x00, 0x06, 0x21, 0x00, // ","
    0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, // "-"
    0x00, 0x00, 0x00, 0x80, 0x61, 0x00, // "."
    0x00, 0x84, 0x10, 0x42, 0x00, 0x00, // "/"
    0x4e, 0x94, 0x55, 0x53, 0xe4, 0x00, // "0"
    0x84, 0x41, 0x10, 0x04, 0xe1, 0x00, // "1"
    0x4e, 0x04, 0x21, 0x84, 0xf0, 0x01, // "2"
    0x1f, 0x42, 0x20, 0x50, 0xe4, 0x00, // "3"
    0x08, 0xa3, 0x24, 0x1f, 0x82, 0x00, // "4"
    0x5f, 0xf0, 0x40, 0x50, 0xe4, 0x00, // "5"
    0x8c, 0x10, 0x3c, 0x51, 0xe4, 0x00, // "6"
    0x1f, 0x84, 0x10, 0x82, 0x20, 0x00, // "7"
    0x4e, 0x14, 0x39, 0x51, 0xe4, 0x00, // "8"
    0x4e, 0x14, 0x79, 0x10, 0x62, 0x00, // "9"
    0x80, 0x61, 0x00, 0x86, 0x01, 0x00, // ":"
    0x80, 0x61, 0x00, 0x06, 0x21, 0x00, // ";"
    0x10, 0x42, 0x08, 0x04, 0x02, 0x01, // "<"
    0x00, 0xf0, 0x01, 0x1f, 0x00, 0x00, // "="
    0x81, 0x40, 0x20, 0x84, 0x10, 0x00, // ">"
    0x4e, 0x04, 0x21, 0x04, 0x40, 0x00, // "?"
    0x4e, 0x04, 0x59, 0x55, 0xe5, 0x00, // "@"
    0x4e, 0x14, 0x45, 0x5f, 0x14, 0x01, // "A"
    0x4f, 0x14, 0x3d, 0x51, 0xf4, 0x00, // "B"
    0x4e, 0x14, 0x04, 0x41, 0xe4, 0x00, // "C"
    0x47, 0x12, 0x45, 0x51, 0x72, 0x00, // "D"
    0x5f, 0x10, 0x3c, 0x41, 0xf0, 0x01, // "E"
    0x5f, 0x10, 0x1c, 0x41, 0x10, 0x00, // "F"
    0x4e, 0x14, 0x04, 0x59, 0xe4, 0x00, // "G"
    0x51, 0x14, 0x7d, 0x51, 0x14, 0x01, // "H"
    0x0e, 0x41, 0x10, 0x04, 0xe1, 0x00, // "I"
    0x1c, 0x82, 0x20, 0x48, 0x62, 0x00, // "J"
    0x51, 0x52, 0x0c, 0x45, 0x12, 0x01, // "K"
    0x41, 0x10, 0x04, 0x41, 0xf0, 0x01, // "L"
    0xd1, 0x56, 0x45, 0x51, 0x14, 0x01, // "M"
    0x51, 0x34, 0x55, 0x59, 0x14, 0x01, // "N"
    0x4e, 0x14, 0x45, 0x51, 0xe4, 0x00, // "O"
    0x4f, 0x14, 0x3d, 0x41, 0x10, 0x00, // "P"
    0x4e, 0x14, 0x45, 0x55, 0x62, 0x01, // "Q"
    0x4f, 0x14, 0x3d, 0x45, 0x12, 0x01, // "R"
    0x5e, 0x10, 0x38, 0x10, 0xf4, 0x00, // "S"
    0x1f, 0x41, 0x10, 0x04, 0x41, 0x00, // "T"
    0x51, 0x14, 0x45, 0x51, 0xe4, 0x00, // "U"
    0x51, 0x14, 0x45, 0x91, 0x42, 0x00, // "V"
    0x51, 0x14, 0x55, 0xd5, 0x16, 0x01, // "W"
    0x51, 0xa4, 0x10, 0x4a, 0x14, 0x01, // "X"
    0x51, 0xa4, 0x10, 0x04, 0x41, 0x00, // "Y"
    0x1f, 0x84, 0x10, 0x42, 0xf0, 0x01, // "Z"
    0x1c, 0x41, 0x10, 0x04, 0xc1, 0x01, // "["
    0x40, 0x20, 0x10, 0x08, 0x04, 0x00, // "\"
    0x07, 0x41, 0x10, 0x04, 0x71, 0x00, // "]"
    0x84, 0x12, 0x01, 0x00, 0x00, 0x00, // "^"
    0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, // "_"
    0x02, 0x81, 0x00, 0x00, 0x00, 0x00, // "`"
    0x00, 0xe0, 0x40, 0x5e, 0xe4, 0x01, // "a"
    0x41, 0xd0, 0x4c, 0x51, 0xf4, 0x00, // "b"
    0x00, 0xe0, 0x04, 0x41, 0xe4, 0x00, // "c"
    0x10, 0x64, 0x65, 0x51, 0xe4, 0x01, // "d"
    0x00, 0xe0, 0x44, 0x5f, 0xe0, 0x00, // "e"
    0x8c, 0x24, 0x1c, 0x82, 0x20, 0x00, // "f"
    0x00, 0xe0, 0x45, 0x1e, 0xc4, 0x00, // "g"
    0x41, 0xd0, 0x4c, 0x51, 0x14, 0x01, // "h"
    0x04, 0x60, 0x10, 0x04, 0xe1, 0x00, // "i"
    0x08, 0xc0, 0x20, 0x48, 0x62, 0x00, // "j"
    0x82, 0x20, 0x29, 0x86, 0x22, 0x01, // "k"
    0x06, 0x41, 0x10, 0x04, 0xe1, 0x00, // "l"
    0x00, 0xb0, 0x54, 0x55, 0x14, 0x01, // "m"
    0x00, 0xd0, 0x4c, 0x51, 0x14, 0x01, // "n"
    0x00, 0xe0, 0x44, 0x51, 0xe4, 0x00, // "o"
    0x00, 0xf0, 0x44, 0x4f, 0x10, 0x00, // "p"
    0x00, 0x60, 0x65, 0x1e, 0x04, 0x01, // "q"
    0x00, 0xd0, 0x4c, 0x41, 0x10, 0x00, // "r"
    0x00, 0xe0, 0x04, 0x0e, 0xf4, 0x00, // "s"
    0x82, 0x70, 0x08, 0x82, 0xc4, 0x00, // "t"
    0x00, 0x10, 0x45, 0x51, 0x66, 0x01, // "u"
    0x00, 0x10, 0x45, 0x91, 0x42, 0x00, // "v"
    0x00, 0x10, 0x45, 0x55, 0xa5, 0x00, // "w"
    0x00, 0x10, 0x29, 0x84, 0x12, 0x01, // "x"
    0x00, 0x10, 0x45, 0x1e, 0xe4, 0x00, // "y"
    0x00, 0xf0, 0x21, 0x84, 0xf0, 0x01, // "z"
    0x08, 0x41, 0x08, 0x04, 0x81, 0x00, // "{"
    0x04, 0x41, 0x10, 0x04, 0x41, 0x00, // "|"
    0x02, 0x41, 0x20, 0x04, 0x21, 0x00, // "}"
    0x00, 0x40, 0xa8, 0x10, 0x00, 0x00, // "~"
  }
};

// --- 8x16 (Terminus) ---
const Font terminus_8x16 =
{
  .width  = 8,
  .height = 16,
  .pitch  = 16,
  .data   = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // " "
    0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "!"
    0x00, 0x66, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // """
    0x00, 0x00, 0x36, 0x36, 0x36, 0x7f, 0x36, 0x36, 0x7f, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, // "#"
    0x00, 0x08, 0x08, 0x3e, 0x6b, 0x0b, 0x0b, 0x3e, 0x68, 0x68, 0x6b, 0x3e, 0x08, 0x08, 0x00, 0x00, // "$"
    0x00, 0x00, 0x66, 0x6b, 0x36, 0x30, 0x18, 0x18, 0x0c, 0x6c, 0xd6, 0x66, 0x00, 0x00, 0x00, 0x00, // "%"
    0x00, 0x00, 0x1c, 0x36, 0x36, 0x1c, 0x6e, 0x3b, 0x33, 0x33, 0x3b, 0x6e, 0x00, 0x00, 0x00, 0x00, // "&"
    0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "'"
    0x00, 0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, // "("
    0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, // ")"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x1c, 0x7f, 0x1c, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "*"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "+"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, // ","
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "-"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "."
    0x00, 0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, // "/"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x73, 0x7b, 0x6f, 0x67, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "0"
    0x00, 0x00, 0x18, 0x1c, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00, 0x00, 0x00, 0x00, // "1"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x00, // "2"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x60, 0x3c, 0x60, 0x60, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "3"
    0x00, 0x00, 0x60, 0x70, 0x78, 0x6c, 0x66, 0x63, 0x7f, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, // "4"
    0x00, 0x00, 0x7f, 0x03, 0x03, 0x03, 0x3f, 0x60, 0x60, 0x60, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "5"
    0x00, 0x00, 0x3c, 0x06, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "6"
    0x00, 0x00, 0x7f, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, // "7"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "8"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x30, 0x1e, 0x00, 0x00, 0x00, 0x00, // "9"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // ":"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x00, 0x00, 0x00, // ";"
    0x00, 0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, // "<"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "="
    0x00, 0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, // ">"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x30, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "?"
    0x00, 0x00, 0x3e, 0x63, 0x73, 0x6b, 0x6b, 0x6b, 0x6b, 0x73, 0x03, 0x7e, 0x00, 0x00, 0x00, 0x00, // "@"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "A"
    0x00, 0x00, 0x3f, 0x63, 0x63, 0x63, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3f, 0x00, 0x00, 0x00, 0x00, // "B"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x03, 0x03, 0x03, 0x03, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "C"
    0x00, 0x00, 0x1f, 0x33, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x33, 0x1f, 0x00, 0x00, 0x00, 0x00, // "D"
    0x00, 0x00, 0x7f, 0x03, 0x03, 0x03, 0x1f, 0x03, 0x03, 0x03, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x00, // "E"
    0x00, 0x00, 0x7f, 0x03, 0x03, 0x03, 0x1f, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, // "F"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x03, 0x03, 0x7b, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "G"
    0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "H"
    0x00, 0x00, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, // "I"
    0x00, 0x00, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1e, 0x00, 0x00, 0x00, 0x00, // "J"
    0x00, 0x00, 0x63, 0x63, 0x33, 0x1b, 0x0f, 0x0f, 0x1b, 0x33, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "K"
    0x00, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x00, // "L"
    0x00, 0x00, 0x41, 0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "M"
    0x00, 0x00, 0x63, 0x63, 0x63, 0x67, 0x6f, 0x7b, 0x73, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "N"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "O"
    0x00, 0x00, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3f, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, // "P"
    0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7b, 0x3e, 0x60, 0x00, 0x00, 0x00, // "Q"
    0x00, 0x00, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3f, 0x0f, 0x1b, 0x33, 0x63, 0x00, 0x00, 0x00, 0x00, // "R"
    0x00, 0x00, 0x3e, 0x63, 0x03, 0x03, 0x3e, 0x60, 0x60, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "S"
    0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "T"
    0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "U"
    0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36, 0x36, 0x36, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, // "V"
    0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x41, 0x00, 0x00, 0x00, 0x00, // "W"
    0x00, 0x00, 0x63, 0x63, 0x36, 0x36, 0x1c, 0x1c, 0x36, 0x36, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "X"
    0x00, 0x00, 0xc3, 0xc3, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "Y"
    0x00, 0x00, 0x7f, 0x60, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x00, // "Z"
    0x00, 0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00, 0x00, 0x00, 0x00, // "["
    0x00, 0x00, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, // "\"
    0x00, 0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00, 0x00, 0x00, 0x00, // "]"
    0x00, 0x18, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "^"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, // "_"
    0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "`"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x60, 0x7e, 0x63, 0x63, 0x63, 0x7e, 0x00, 0x00, 0x00, 0x00, // "a"
    0x00, 0x00, 0x03, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3f, 0x00, 0x00, 0x00, 0x00, // "b"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x03, 0x03, 0x03, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "c"
    0x00, 0x00, 0x60, 0x60, 0x60, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x00, 0x00, 0x00, 0x00, // "d"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x63, 0x7f, 0x03, 0x03, 0x3e, 0x00, 0x00, 0x00, 0x00, // "e"
    0x00, 0x00, 0x78, 0x0c, 0x0c, 0x3f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00, // "f"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x3e, 0x00, 0x00, // "g"
    0x00, 0x00, 0x03, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "h"
    0x00, 0x00, 0x18, 0x18, 0x00, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, // "i"
    0x00, 0x00, 0x60, 0x60, 0x00, 0x70, 0x60, 0x60, 0x60, 0x60, 0x60, 0x66, 0x66, 0x3c, 0x00, 0x00, // "j"
    0x00, 0x00, 0x03, 0x03, 0x03, 0x63, 0x33, 0x1b, 0x0f, 0x1b, 0x33, 0x63, 0x00, 0x00, 0x00, 0x00, // "k"
    0x00, 0x00, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00, 0x00, // "l"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x00, 0x00, 0x00, 0x00, // "m"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "n"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00, 0x00, 0x00, 0x00, // "o"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3f, 0x03, 0x03, 0x00, 0x00, // "p"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x00, 0x00, // "q"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7b, 0x0f, 0x07, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, // "r"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x03, 0x03, 0x3e, 0x60, 0x60, 0x3f, 0x00, 0x00, 0x00, 0x00, // "s"
    0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x3f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x78, 0x00, 0x00, 0x00, 0x00, // "t"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x00, 0x00, 0x00, 0x00, // "u"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x63, 0x36, 0x36, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, // "v"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x6b, 0x6b, 0x6b, 0x6b, 0x3e, 0x00, 0x00, 0x00, 0x00, // "w"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x36, 0x1c, 0x36, 0x63, 0x63, 0x00, 0x00, 0x00, 0x00, // "x"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x3e, 0x00, 0x00, // "y"
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x7f, 0x00, 0x00, 0x00, 0x00, // "z"
    0x00, 0x00, 0x38, 0x0c, 0x0c, 0x0c, 0x06, 0x0c, 0x0c, 0x0c, 0x0c, 0x38, 0x00, 0x00, 0x00, 0x00, // "{"
    0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, // "|"
    0x00, 0x00, 0x0e, 0x18, 0x18, 0x18, 0x30, 0x18, 0x18, 0x18, 0x18, 0x0e, 0x00, 0x00, 0x00, 0x00, // "}"
    0x00, 0xce, 0xdb, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // "~"
  }
};

