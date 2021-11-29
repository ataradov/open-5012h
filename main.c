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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "gd32f4xx.h"
#include "hal_gpio.h"
#include "lcd.h"
#include "utils.h"
#include "flash.h"
#include "timer.h"
#include "config.h"
#include "buttons.h"
#include "battery.h"
#include "capture.h"
#include "scope.h"

/*- Definitions -------------------------------------------------------------*/
#define RESET_TO_DEFAULT     (BTN_SHIFT | BTN_SAVE)
#define CALIBRATION_MODE     (BTN_SHIFT | BTN_MODE)

/*- Variables ---------------------------------------------------------------*/

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static void sys_init(void)
{
  RCU->AHB1EN |= RCU_AHB1EN_PAEN_Msk | RCU_AHB1EN_PBEN_Msk | RCU_AHB1EN_PCEN_Msk |
      RCU_AHB1EN_PDEN_Msk | RCU_AHB1EN_PEEN_Msk | RCU_AHB1EN_PFEN_Msk |
      RCU_AHB1EN_PGEN_Msk | RCU_AHB1EN_PHEN_Msk | RCU_AHB1EN_PIEN_Msk;

  // Stop execution of the program before it has a chance to configure high speed
  // clocks. This helps with programming.
  if (buttons_read() & BTN_F2)
    while (1);

  RCU->CTL_b.HXTALEN = 1;
  while (0 == RCU->CTL_b.HXTALSTB);

  RCU->PLL = RCU_PLL_PLLSEL_Msk | (20 << RCU_PLL_PLLPSC_Pos) | (500 << RCU_PLL_PLLN_Pos) |
      (0 << RCU_PLL_PLLP_Pos) | (15 << RCU_PLL_PLLQ_Pos);

  RCU->CTL_b.PLLEN = 1;
  while (0 == RCU->CTL_b.PLLSTB);

  RCU->CFG0 = (2/*CK_PLLP*/ << RCU_CFG0_SCS_Pos) | (0/*CK_SYS*/ << RCU_CFG0_AHBPSC_Pos) |
      (5/*DIV 4*/ << RCU_CFG0_APB1PSC_Pos) | (4/*DIV 2*/ << RCU_CFG0_APB2PSC_Pos) |
      (0 << RCU_CFG0_RTCDIV_Pos);
  while (RCU->CFG0_b.SCSS != 2);
}

//-----------------------------------------------------------------------------
static void print_value(int x, int y, char *name, uint32_t value)
{
  static const char hex[] = "0123456789abcdef";
  char str[9];

  for (int i = 0; i < 8; i++)
    str[i] = hex[(value >> ((7-i)*4)) & 0xf];

  str[8] = 0;

  lcd_puts((x+1)*8, 2 + y*16, name);
  lcd_puts((x+6)*8, 2 + y*16, "= 0x");
  lcd_puts((x+10)*8, 2 + y*16, str);
}

//-----------------------------------------------------------------------------
void irq_handler_hard_fault_c(uint32_t lr, uint32_t msp, uint32_t psp)
{
  uint32_t s_r0, s_r1, s_r2, s_r3, s_r12, s_lr, s_pc, s_psr;
  uint32_t r_CFSR, r_HFSR, r_DFSR, r_AFSR, r_BFAR, r_MMAR;
  uint32_t *sp = (uint32_t *)((lr & 4) ? psp : msp);

  s_r0  = sp[0];
  s_r1  = sp[1];
  s_r2  = sp[2];
  s_r3  = sp[3];
  s_r12 = sp[4];
  s_lr  = sp[5];
  s_pc  = sp[6];
  s_psr = sp[7];

  r_CFSR = SCB->CFSR;  // Configurable Fault Status Register (MMSR, BFSR and UFSR)
  r_HFSR = SCB->HFSR;  // Hard Fault Status Register
  r_DFSR = SCB->DFSR;  // Debug Fault Status Register
  r_MMAR = SCB->MMFAR; // MemManage Fault Address Register
  r_BFAR = SCB->BFAR;  // Bus Fault Address Register
  r_AFSR = SCB->AFSR;  // Auxiliary Fault Status Register

  asm("nop"); // Setup breakpoint here

  lcd_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, LCD_BLACK_COLOR);
  lcd_set_color(LCD_BLACK_COLOR, LCD_WHITE_COLOR);
  lcd_set_font(FONT_LARGE);
  lcd_puts(120, 2, "HARD FAULT");

  print_value(0,  2, "R0",  s_r0);
  print_value(0,  3, "R1",  s_r1);
  print_value(0,  4, "R2",  s_r2);
  print_value(0,  5, "R3",  s_r3);
  print_value(0,  6, "R12", s_r12);
  print_value(0,  7, "LR",  s_lr);
  print_value(0,  8, "PC",  s_pc);
  print_value(0,  9, "PSR", s_psr);
  print_value(0, 10, "SP",  (lr & 4) ? psp : msp);
  print_value(0, 11, "MSP", msp);
  print_value(0, 12, "PSP", psp);

  print_value(20, 2, "CFSR", r_CFSR);
  print_value(20, 3, "HFSR", r_HFSR);
  print_value(20, 4, "DFSR", r_DFSR);
  print_value(20, 5, "MMAR", r_MMAR);
  print_value(20, 6, "BFAR", r_BFAR);
  print_value(20, 7, "AFSR", r_AFSR);

  while (1);
}

//-----------------------------------------------------------------------------
__attribute__((naked)) void irq_handler_hard_fault(void)
{
  asm volatile (R"asm(
    mov    r0, lr
    mrs    r1, msp
    mrs    r2, psp
    b      irq_handler_hard_fault_c
    )asm"
  );
}

//-----------------------------------------------------------------------------
void error(char *text)
{
  int len;

  __disable_irq();

  capture_stop();
  capture_disable_clock();
  lcd_set_backlight_level(50);

  lcd_set_font(FONT_LARGE);
  lcd_set_color(LCD_WHITE_COLOR, LCD_RED_COLOR);
  lcd_fill_rect(0, 0, LCD_WIDTH, LCD_HEIGHT, LCD_WHITE_COLOR);

  for (len = 0; text[len]; len++);

  lcd_puts(LCD_WIDTH/2 - len*4, 112, text);

  while (1);
}

//-----------------------------------------------------------------------------
void battery_low_handler(void)
{
  error("BATTERY LOW");
}

//-----------------------------------------------------------------------------
void buttons_handler(int buttons)
{
  scope_buttons_handler(buttons);

#if 1  // Debug only, makes it easier to program things
  if (buttons & BTN_F2)
  {
    capture_disable_clock();
    while (1);
  }
#endif
}

//-----------------------------------------------------------------------------
int main(void)
{
  int buttons;

  sys_init();
  timer_init();
  lcd_init();
  crc32_init();
  //flash_init();
  config_init();
  buttons_init();
  battery_init();
  capture_init();

  lcd_set_font(FONT_LARGE);
  lcd_set_color(LCD_BLACK_COLOR, LCD_WHITE_COLOR);
  lcd_set_backlight_level(config.lcd_bl_level);

  buttons = buttons_state();

  if ((buttons & RESET_TO_DEFAULT) == RESET_TO_DEFAULT)
    config_reset();

  scope_init((buttons & CALIBRATION_MODE) == CALIBRATION_MODE);

  while (1)
  {
    timer_task();
    scope_task();
    battery_task();
    buttons_task();
    config_task();
  }

  return 0;
}

