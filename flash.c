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
#include "gd32f4xx.h"
#include "hal_gpio.h"
#include "common.h"
#include "utils.h"
#include "flash.h"

/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(CS,       A, 3)
HAL_GPIO_PIN(CLK,      A, 5)
HAL_GPIO_PIN(MISO,     A, 6)
HAL_GPIO_PIN(MOSI,     A, 7)

enum
{
  CMD_READ_JEDEC_ID    = 0x9f,
};

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static int spi_write(int value)
{
  SPI0->DATA = value;
  while (0 == SPI0->STAT_b.RBNE);
  return SPI0->DATA;
}

//-----------------------------------------------------------------------------
static bool flash_test(void)
{
  int id0, id1, id2;

  HAL_GPIO_CS_clr();
  spi_write(CMD_READ_JEDEC_ID);
  id0 = spi_write(0);
  id1 = spi_write(0);
  id2 = spi_write(0);
  HAL_GPIO_CS_set();

  return (id0 == 0xef && id1 == 0x40 && id2 == 0x17);
}

//-----------------------------------------------------------------------------
void flash_init(void)
{
  HAL_GPIO_CS_out();
  HAL_GPIO_CS_set();
  HAL_GPIO_CLK_alt(5);
  HAL_GPIO_MISO_alt(5);
  HAL_GPIO_MOSI_alt(5);

  RCU->APB2EN_b.SPI0EN = 1;

  SPI0->CTL0 = SPI0_CTL0_SPIEN_Msk | SPI0_CTL0_MSTMOD_Msk | (1/*PCLK/4*/ << SPI0_CTL0_PSC_Pos) |
      SPI0_CTL0_CKPH_Msk | SPI0_CTL0_CKPL_Msk | SPI0_CTL0_SWNSSEN_Msk | SPI0_CTL0_SWNSS_Msk;

  delay_cycles(100);

  if (!flash_test())
    error("Flash error");
}

