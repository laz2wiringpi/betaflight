/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#ifdef USE_CMS

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_power.h"

#include "config/feature.h"

#include "sensors/battery.h"
#include "sensors/current.h"
#include "sensors/voltage.h"

#include "fc/config.h"

// voltage menu
voltageMeterSource_e batteryConfig_voltageMeterSource;
uint16_t batteryConfig_vbatmaxcellvoltage;
uint16_t batteryConfig_vbatwarningcellvoltage;
uint16_t batteryConfig_vbatmincellvoltage;
uint8_t batteryConfig_vbathysteresis;
uint8_t voltageSensorADCConfig_vbatscale;
uint8_t batteryConfig_useVBatFilter;

//  current menu

currentMeterSource_e batteryConfig_currentMeterSource;
int16_t currentSensorADCConfig_scale;
int16_t currentSensorADCConfig_offset;

#ifdef USE_VIRTUAL_CURRENT_METER
int16_t currentSensorVirtualConfig_scale;
int16_t currentSensorVirtualConfig_offset;
#endif



static long menuVoltageOnEnter(void)
{

    batteryConfig_voltageMeterSource = batteryConfig()->voltageMeterSource;
    batteryConfig_vbatmaxcellvoltage = batteryConfig()->vbatmaxcellvoltage;
    batteryConfig_vbatwarningcellvoltage = batteryConfig()->vbatwarningcellvoltage;
    batteryConfig_vbatmincellvoltage = batteryConfig()->vbatmincellvoltage;
    batteryConfig_vbathysteresis = batteryConfig()->vbathysteresis;
    batteryConfig_useVBatFilter =  batteryConfig()->useVBatFilter;
    voltageSensorADCConfig_vbatscale = voltageSensorADCConfig(0)->vbatscale;

    return 0;
}

static long menuVoltageOnExit(const OSD_Entry *self)
{
    UNUSED(self);

    batteryConfigMutable()->voltageMeterSource = batteryConfig_voltageMeterSource;
    batteryConfigMutable()->vbatmaxcellvoltage = batteryConfig_vbatmaxcellvoltage;
    batteryConfigMutable()->vbatwarningcellvoltage = batteryConfig_vbatwarningcellvoltage;
    batteryConfigMutable()->vbatmincellvoltage = batteryConfig_vbatmincellvoltage;
    batteryConfigMutable()->vbathysteresis = batteryConfig_vbathysteresis;
    batteryConfigMutable()->useVBatFilter = batteryConfig_useVBatFilter;
    voltageSensorADCConfigMutable(0)->vbatscale = voltageSensorADCConfig_vbatscale;

    return 0;
}

const OSD_Entry menuVoltageEntries[] =
{
    {"--- VOLTAGE ---", OME_Label, NULL, NULL, 0},
    { "SOURCE", OME_TAB, NULL, &(OSD_TAB_t){ &batteryConfig_voltageMeterSource, VOLTAGE_METER_COUNT - 1, voltageMeterSourceNames }, 0 },
    { "MAX", OME_UINT16, NULL, &(OSD_UINT16_t) { &batteryConfig_vbatmaxcellvoltage, VBAT_CELL_VOTAGE_RANGE_MIN, VBAT_CELL_VOTAGE_RANGE_MAX, 1 }, 0 },
    { "WARN", OME_UINT16, NULL, &(OSD_UINT16_t) { &batteryConfig_vbatwarningcellvoltage, VBAT_CELL_VOTAGE_RANGE_MIN, VBAT_CELL_VOTAGE_RANGE_MAX, 1 }, 0 },
    { "CRIT", OME_UINT16, NULL, &(OSD_UINT16_t) { &batteryConfig_vbatmincellvoltage, VBAT_CELL_VOTAGE_RANGE_MIN, VBAT_CELL_VOTAGE_RANGE_MAX, 1 }, 0 },
    { "HYSTERESIS", OME_UINT8, NULL, &(OSD_UINT8_t) { &batteryConfig_vbathysteresis, 0, 20, 1 }, 0 },
    { "SCALE", OME_UINT8, NULL, &(OSD_UINT8_t){ &voltageSensorADCConfig_vbatscale, VBAT_SCALE_MIN, VBAT_SCALE_MAX, 1 }, 0 },  
    { "FILTERED", OME_Bool, NULL, &batteryConfig_useVBatFilter, 0 },

    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu menuVoltage = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUVOLTAGE",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = menuVoltageOnEnter,
    .onExit = menuVoltageOnExit,
    .entries = menuVoltageEntries,
};


static long menuCurrentOnEnter(void)
{

        batteryConfig_currentMeterSource = batteryConfig()->currentMeterSource;
        currentSensorADCConfig_scale = currentSensorADCConfig()->scale;
        currentSensorADCConfig_offset = currentSensorADCConfig()->offset;

    #ifdef USE_VIRTUAL_CURRENT_METER
        currentSensorVirtualConfig_scale = currentSensorVirtualConfig()->scale;
        currentSensorVirtualConfig_offset = currentSensorVirtualConfig()->offset;
    #endif


    return 0;
}

static long menuCurrentOnExit(const OSD_Entry *self)
{
    UNUSED(self);

    batteryConfigMutable()->currentMeterSource = batteryConfig_currentMeterSource;
    currentSensorADCConfigMutable()->scale = currentSensorADCConfig_scale;
    currentSensorADCConfigMutable()->offset = currentSensorADCConfig_offset;


    #ifdef USE_VIRTUAL_CURRENT_METER
        currentSensorVirtualConfigMutable()->scale = currentSensorVirtualConfig_scale;
        currentSensorVirtualConfigMutable()->offset = currentSensorVirtualConfig_offset;
    #endif

    return 0;
}

const OSD_Entry menuCurrentEntries[] =
{
    {"--- CURRENT ---", OME_Label, NULL, NULL, 0},
    { "SORCE", OME_TAB, NULL, &(OSD_TAB_t){ &batteryConfig_currentMeterSource, CURRENT_METER_COUNT - 1, currentMeterSourceNames }, 0 },
    { "SCALE", OME_INT16, NULL, &(OSD_INT16_t){ &currentSensorADCConfig_scale, -16000, 16000, 5 }, 0 },
    { "OFFSET", OME_INT16, NULL, &(OSD_INT16_t){ &currentSensorADCConfig_offset, -16000, 16000, 5 }, 0 },

#ifdef USE_VIRTUAL_CURRENT_METER
    { "VIRT SCALE", OME_INT16, NULL, &(OSD_INT16_t){ &currentSensorVirtualConfig_scale, -16000, 16000, 5 }, 0 },
    { "VIRT OFFSET", OME_INT16, NULL, &(OSD_INT16_t){ &currentSensorVirtualConfig_offset, -16000, 16000, 5 }, 0 },
#endif

    {"BACK", OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu menuCurrent = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUCURRENT",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = menuCurrentOnEnter,
    .onExit = menuCurrentOnExit,
    .entries = menuCurrentEntries,
};


static long cmsx_Power_onEnter(void)
{
    return 0;
}

static long cmsx_Power_onExit(const OSD_Entry *self)
{
    UNUSED(self);

    return 0;
}

static const OSD_Entry cmsx_menuPowerEntries[] =
{
    { "-- POWER --", OME_Label, NULL, NULL, 0},
    {"VOLTAGE",      OME_Submenu, cmsMenuChange, &menuVoltage,         0},
    {"CURRENT",      OME_Submenu, cmsMenuChange, &menuVoltage,         0},

    { "BACK", OME_Back, NULL, NULL, 0 },
    { NULL, OME_END, NULL, NULL, 0 }
};

CMS_Menu cmsx_menuPower = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUPWR",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_Power_onEnter,
    .onExit = cmsx_Power_onExit,
    .entries = cmsx_menuPowerEntries
};

#endif
