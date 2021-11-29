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
#include "timer.h"
#include "utils.h"
#include "common.h"
#include "config.h"

/*- Definitions -------------------------------------------------------------*/
#define MAGIC              0x78656c41
#define VERSION            1

#define FLASH_START        0x08000000
#define ENTRY_SIZE         (1024)
#define STORAGE_SIZE       (256*1024)
#define STORAGE_OFFSET     (256*1024)
#define STORAGE_START      (FLASH_START + STORAGE_OFFSET)
#define ENTRIES_COUNT      (STORAGE_SIZE / ENTRY_SIZE)

#define SECTOR_0_INDEX     (6)
#define SECTOR_0_OFFSET    (0)
#define SECTOR_1_INDEX     (7)
#define SECTOR_1_OFFSET    (128*1024)

#define TIMER_INTERVAL     1000

#define FMC_KEY_KEY1       0x45670123
#define FMC_KEY_KEY2       0xcdef89ab

#define FMC_STAT_ALL_ERRORS (FMC_STAT_OPERR_Msk | FMC_STAT_WPERR_Msk | \
    FMC_STAT_PGMERR_Msk | FMC_STAT_PGSERR_Msk | FMC_STAT_RDDERR_Msk)

/*- Variables ---------------------------------------------------------------*/
Config config;
static int g_config_timer = 0;
static int g_entry_offset;
static Config g_config_copy;
static bool g_flash_erase_busy = false;
static bool g_flash_write_busy = false;
static uint32_t *g_flash_write_addr = 0;
static uint32_t *g_flash_write_data = 0;
static uint32_t g_flash_write_size = 0;

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline Config *get_entry(int index)
{
  return (Config *)(STORAGE_START + index * ENTRY_SIZE);
}

//-----------------------------------------------------------------------------
static void flash_unlock(void)
{
  FMC->KEY = FMC_KEY_KEY1;
  FMC->KEY = FMC_KEY_KEY2;
  FMC->CTL = 0;

  if (FMC->CTL & FMC_CTL_LK_Msk)
    error("Flash unlock error");
}

//-----------------------------------------------------------------------------
static void flash_erase(int index)
{
  uint32_t cmd = FMC_CTL_SER_Msk | (index << FMC_CTL_SN_Pos);

  FMC->CTL = cmd;
  FMC->CTL = cmd | FMC_CTL_START_Msk;

  g_flash_erase_busy = true;
}

//-----------------------------------------------------------------------------
static bool flash_erase_task(void)
{
  if (!g_flash_erase_busy)
    return false;

  if (FMC->STAT & FMC_STAT_BUSY_Msk)
    return true;

  if (FMC->STAT & FMC_STAT_ALL_ERRORS)
    error("Flash erase error");

  FMC->CTL = 0;

  g_flash_erase_busy = false;

  return false;
}

//-----------------------------------------------------------------------------
static void flash_write(uint32_t *addr, uint32_t *data, int size)
{
  g_flash_write_addr = addr;
  g_flash_write_data = data;
  g_flash_write_size = size;
  g_flash_write_busy = true;
}

//-----------------------------------------------------------------------------
static bool flash_write_task(void)
{
  if (!g_flash_write_busy)
    return false;

  if (FMC->STAT & FMC_STAT_BUSY_Msk)
    return true;

  if (FMC->STAT & FMC_STAT_ALL_ERRORS)
    error("Flash write error");

  if (g_flash_write_size > 0)
  {
    FMC->CTL = (2/*WORD*/ << FMC_CTL_PSZ_Pos) | FMC_CTL_PG_Msk;

    *g_flash_write_addr = *g_flash_write_data;
    g_flash_write_addr++;
    g_flash_write_data++;
    g_flash_write_size -= sizeof(uint32_t);
    return true;
  }
  else
  {
    FMC->CTL = 0;
    g_flash_write_busy = false;
  }

  return false;
}

//-----------------------------------------------------------------------------
static bool is_entry_valid(Config *entry)
{
  if (entry->magic != MAGIC)
    return false;

  if (entry->version != VERSION)
    return false;

  if (entry->size != sizeof(Config))
    return false;

  if (crc32_calc((uint32_t *)entry, sizeof(Config) - sizeof(uint32_t)) != entry->crc)
    return false;

  return true;
}

//-----------------------------------------------------------------------------
static int find_last_entry(void)
{
  int max_count = -1;
  int index = -1;

  for (int i = 0; i < ENTRIES_COUNT; i++)
  {
    Config *entry = get_entry(i);

    if (!is_entry_valid(entry))
      continue;

    if (entry->count > max_count)
    {
      index = i;
      max_count = entry->count;
    }
  }

  return index;
}

//-----------------------------------------------------------------------------
static bool config_changed(void)
{
  return crc32_calc((uint32_t *)&config, sizeof(Config) - sizeof(uint32_t)) != config.crc;
}

//-----------------------------------------------------------------------------
static void config_save(void)
{
  g_entry_offset = (g_entry_offset + ENTRY_SIZE) % STORAGE_SIZE;

  if (g_entry_offset == SECTOR_0_OFFSET)
    flash_erase(SECTOR_0_INDEX);
  else if (g_entry_offset == SECTOR_1_OFFSET)
    flash_erase(SECTOR_1_INDEX);

  config.count++;
  config.crc = crc32_calc((uint32_t *)&config, sizeof(Config) - sizeof(uint32_t));

  g_config_copy = config;

  flash_write((uint32_t *)(STORAGE_START + g_entry_offset),
      (uint32_t *)&g_config_copy, sizeof(Config));
}

//-----------------------------------------------------------------------------
void config_init(void)
{
  int index = find_last_entry();

  flash_unlock();

  if (index == -1)
  {
    flash_erase(SECTOR_0_INDEX);

    config_reset();

    config.count = 0;
    config.power_cycles = 0;
    config.charge_cycles = 0;

    g_entry_offset = 0;
  }
  else
  {
    Config *entry = get_entry(index);
    config = *entry;
    g_entry_offset = index * ENTRY_SIZE;
  }

  config.power_cycles++;

  timer_add(&g_config_timer);
}

//-----------------------------------------------------------------------------
void config_reset(void)
{
  config.magic   = MAGIC;
  config.size    = sizeof(Config);
  config.version = VERSION;

  // Keep config.count
  // Keep config.power_cycles
  // Keep config.charge_cycles

  config.lcd_bl_level           = 100;

  config.ac_coupling            = false;
  config.x10                    = false;

  config.trigger_mode           = TRIGGER_MODE_AUTO;
  config.trigger_edge           = TRIGGER_EDGE_RISE;
  config.trigger_level          = 0;
  config.trigger_level_mv       = 0;

  config.horizontal_scale       = HS_100_us;
  config.horizontal_position    = 0;
  config.horizontal_position_px = 0;
  config.horizontal_period      = 1; // Dummy value

  config.vertical_scale         = VS_200_mV;
  config.vertical_position      = 0;
  config.vertical_position_mv   = 0;
  config.vertical_mult          = 1; // Dummy value

  config.sample_rate_limit      = 0;

  config.measure_display        = false;

  for (int i = 0; i < ARRAY_SIZE(config.padding); i++)
    config.padding[i] = 0;

  config.calib_channel_delta    = -5;
  config.calib_dac_zero         = 2010;

  config.calib_dac_mult[VS_50_mV]  = 2308;
  config.calib_dac_mult[VS_100_mV] = 4548;
  config.calib_dac_mult[VS_200_mV] = 5277;
  config.calib_dac_mult[VS_500_mV] = 5384;
  config.calib_dac_mult[VS_1_V]    = 5160;
  config.calib_dac_mult[VS_2_V]    = 5082;
  config.calib_dac_mult[VS_5_V]    = 5542;
  config.calib_dac_mult[VS_10_V]   = 5630;

  config.calib_vs_mult[VS_50_mV]   = 4941;
  config.calib_vs_mult[VS_100_mV]  = 4808;
  config.calib_vs_mult[VS_200_mV]  = 8343;
  config.calib_vs_mult[VS_500_mV]  = 20449;
  config.calib_vs_mult[VS_1_V]     = 42447;
  config.calib_vs_mult[VS_2_V]     = 86232;
  config.calib_vs_mult[VS_5_V]     = 196726;
  config.calib_vs_mult[VS_10_V]    = 386029;
}

//-----------------------------------------------------------------------------
void config_task(void)
{
  if (flash_erase_task())
    return;

  if (flash_write_task())
    return;

  if (g_config_timer == 0)
  {
    if (config_changed())
      config_save();

    g_config_timer = TIMER_INTERVAL;
  }
}


