/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

extern "C" {
    #include "platform.h"
    #include "build/debug.h"

    #include "blackbox/blackbox.h"
    #include "blackbox/blackbox_io.h"

    #include "drivers/max7456_symbols.h"
    #include "drivers/persistent.h"
    #include "drivers/serial.h"
    #include "drivers/io.h"

    #include "fc/config.h"
    #include "fc/core.h"
    #include "fc/rc_controls.h"
    #include "fc/rc_modes.h"
    #include "fc/runtime_config.h"

    #include "flight/mixer.h"
    #include "flight/pid.h"
    #include "flight/imu.h"

    #include "io/beeper.h"
    #include "io/gps.h"

    #include "osd/osd.h"
    #include "osd/osd_elements.h"

    #include "pg/pg.h"
    #include "pg/pg_ids.h"
    #include "pg/rx.h"

    #include "rx/rx.h"

    #include "sensors/battery.h"

    attitudeEulerAngles_t attitude;
    pidProfile_t *currentPidProfile;
    int16_t rcData[MAX_SUPPORTED_RC_CHANNEL_COUNT];
    uint8_t GPS_numSat;
    uint16_t GPS_distanceToHome;
    int16_t GPS_directionToHome;
    uint32_t GPS_distanceFlownInCm;
    int32_t GPS_coord[2];
    gpsSolutionData_t gpsSol;
    float motor[8];
    float motorOutputHigh = 2047;
    float motorOutputLow = 1000;
    acc_t acc;
    float accAverage[XYZ_AXIS_COUNT];

    PG_REGISTER(batteryConfig_t, batteryConfig, PG_BATTERY_CONFIG, 0);
    PG_REGISTER(blackboxConfig_t, blackboxConfig, PG_BLACKBOX_CONFIG, 0);
    PG_REGISTER(systemConfig_t, systemConfig, PG_SYSTEM_CONFIG, 0);
    PG_REGISTER(pilotConfig_t, pilotConfig, PG_PILOT_CONFIG, 0);
    PG_REGISTER(imuConfig_t, imuConfig, PG_IMU_CONFIG, 0);

    timeUs_t simulationTime = 0;

    void osdRefresh(timeUs_t currentTimeUs);
    uint16_t updateLinkQualitySamples(uint16_t value);
}

/* #define DEBUG_OSD */

#include "unittest_macros.h"
#include "unittest_displayport.h"
#include "gtest/gtest.h"

extern "C" {
PG_REGISTER(flight3DConfig_t, flight3DConfig, PG_MOTOR_3D_CONFIG, 0);

boxBitmask_t rcModeActivationMask;
int16_t debug[DEBUG16_VALUE_COUNT];
uint8_t debugMode = 0;

uint16_t updateLinkQualitySamples(uint16_t value);

extern uint16_t applyRxChannelRangeConfiguraton(int sample, const rxChannelRangeConfig_t *range);
}
void setDefaultSimulationState()
{

    setLinkQualityDirect(LINK_QUALITY_MAX_VALUE);

}

/*
 * Performs a test of the OSD actions on arming.
 * (reused throughout the test suite)
 */
void doTestArm(bool testEmpty = true)
{
    // given
    // craft has been armed
    ENABLE_ARMING_FLAG(ARMED);

    // when
    // sufficient OSD updates have been called
    osdRefresh(simulationTime);

    // then
    // arming alert displayed
    displayPortTestBufferSubstring(12, 7, "ARMED");

    // given
    // armed alert times out (0.5 seconds)
    simulationTime += 0.5e6;

    // when
    // sufficient OSD updates have been called
    osdRefresh(simulationTime);

    // then
    // arming alert disappears
#ifdef DEBUG_OSD
    displayPortTestPrint();
#endif
    if (testEmpty) {
        displayPortTestBufferIsEmpty();
    }
}

/*
 * Auxiliary function. Test is there're stats that must be shown
 */
bool isSomeStatEnabled(void) {
    return (osdConfigMutable()->enabled_stats != 0);
}

/*
 * Performs a test of the OSD actions on disarming.
 * (reused throughout the test suite)
 */
void doTestDisarm()
{
    // given
    // craft is disarmed after having been armed
    DISABLE_ARMING_FLAG(ARMED);

    // when
    // sufficient OSD updates have been called
    osdRefresh(simulationTime);

    // then
    // post flight statistics displayed
    if (isSomeStatEnabled()) {
        displayPortTestBufferSubstring(2, 2, "  --- STATS ---");
    }
}

/*
 * Tests initialisation of the OSD and the power on splash screen.
 */
TEST(LQTest, TestInit)
{
    // given
    // display port is initialised
    displayPortTestInit();

    // and
    // default state values are set
    setDefaultSimulationState();

    // and
    // this battery configuration (used for battery voltage elements)
    batteryConfigMutable()->vbatmincellvoltage = 330;
    batteryConfigMutable()->vbatmaxcellvoltage = 430;

    // when
    // OSD is initialised
    osdInit(&testDisplayPort);

    // then
    // display buffer should contain splash screen
    displayPortTestBufferSubstring(7, 8, "MENU:THR MID");
    displayPortTestBufferSubstring(11, 9, "+ YAW LEFT");
    displayPortTestBufferSubstring(11, 10, "+ PITCH UP");

    // when
    // splash screen timeout has elapsed
    simulationTime += 4e6;
    osdUpdate(simulationTime);

    // then
    // display buffer should be empty
#ifdef DEBUG_OSD
    displayPortTestPrint();
#endif
    displayPortTestBufferIsEmpty();
}
/*
 * Tests the link Quality OSD element.
 */
TEST(LQTest, TestRxLQ)
{
    rssiSource = RSSI_SOURCE_RX_CHANNEL;
    // sample data
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }

// check all but crsf
    EXPECT_EQ(150, rxGetLinkQuality());
    EXPECT_EQ(5, rxGetLinkQualityOsd());

//  check crsf
    rssiSource = RSSI_SOURCE_RX_PROTOCOL_CRSF;

    EXPECT_EQ(150, rxGetLinkQuality());
    EXPECT_EQ(150, rxGetLinkQualityOsd());

/// check when tx off
    setLinkQualityDirect(0);
    EXPECT_EQ(0, rxGetLinkQuality());
    EXPECT_EQ(0, rxGetLinkQualityOsd());
}

/*
 * Tests the OSD_LINK_QUALITY element RSSI_SOURCE_RX_CHANNEL.
 */
TEST(LQTest, TestElementLQ_RX)
{
    // given

    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
    }


    rxConfigMutable()->rssi_scale =100;


    rssiSource = RSSI_SOURCE_RX_CHANNEL;
    EXPECT_EQ(RSSI_SOURCE_RX_CHANNEL, rssiSource);

    osdConfigMutable()->item_pos[OSD_LINK_QUALITY] = OSD_POS(8, 1) | OSD_PROFILE_1_FLAG;
    osdConfigMutable()->link_quality_alarm = 0;

    osdAnalyzeActiveElements();

    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);
    osdRefresh(simulationTime);
    osdRefresh(simulationTime);
    // then
    displayPortTestBufferSubstring(8, 1, "  9");

    // when
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }
    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);

    // then
    displayPortTestBufferSubstring(8, 1, "  0");

    // when
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }

    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);

    // then
    displayPortTestBufferSubstring(8, 1, "  5");
}
/*
 * Tests the OSD_LINK_QUALITY element RSSI_SOURCE_RX_CHANNEL.
 */
TEST(LQTest, TestElementLQ_CRSF)
{
    // given

    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
    }


    rxConfigMutable()->rssi_scale =100;


    rssiSource = RSSI_SOURCE_RX_PROTOCOL_CRSF;
    EXPECT_EQ(RSSI_SOURCE_RX_PROTOCOL_CRSF, rssiSource);

    osdConfigMutable()->item_pos[OSD_LINK_QUALITY] = OSD_POS(8, 1) | OSD_PROFILE_1_FLAG;
    osdConfigMutable()->link_quality_alarm = 0;

    osdAnalyzeActiveElements();

    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);
    osdRefresh(simulationTime);
    osdRefresh(simulationTime);
    // then
    displayPortTestBufferSubstring(8, 1, "300");

    // when
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }
    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);

    // then
    displayPortTestBufferSubstring(8, 1, "  0");

    // when
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }

    displayClearScreen(&testDisplayPort);
    osdRefresh(simulationTime);

    // then
    displayPortTestBufferSubstring(8, 1, "150");
}

/*
 * Tests the calculation of statistics with metric unit output.
 * (essentially an abridged version of the previous test
*/
/* TODOLaz
TEST(LQTest, TestLQAlarms)
{
    // given
    // default state is set
    setDefaultSimulationState();
    sensorsSet(SENSOR_GPS);

    // and
    // the following OSD elements are visible
    osdConfigMutable()->item_pos[OSD_LINK_QUALITY]              = OSD_POS(8, 1)  | OSD_PROFILE_1_FLAG;

    // and
    // this set of alarm values
    osdConfigMutable()->link_quality_alarm = 80;

    osdAnalyzeActiveElements();

    // and
    // using the metric unit system
    osdConfigMutable()->units = OSD_UNIT_METRIC;

    // when
    // the craft is armed
    doTestArm(false);


    // then
    // no elements should flash as all values are out of alarm range
    for (int i = 0; i < 30; i++) {
        // Check for visibility every 100ms, elements should always be visible
        simulationTime += 0.1e6;
        osdRefresh(simulationTime);

#ifdef DEBUG_OSD
        printf("%d\n", i);
#endif
        displayPortTestBufferSubstring(8,  1, "9");

    }

    // when
    // half values are out of range
    for (int x = 0; x < LINK_QUALITY_SAMPLE_COUNT; x++) {
     setLinkQualityDirect(updateLinkQualitySamples(300));
     setLinkQualityDirect(updateLinkQualitySamples(0));
    }

    simulationTime += 60e6;
    osdRefresh(simulationTime);


    // then
    // elements showing values in alarm range should flash
    for (int i = 0; i < 15; i++) {
        // Blinking should happen at 5Hz
        simulationTime += 0.2e6;
        osdRefresh(simulationTime);

#ifdef DEBUG_OSD
        printf("%d\n", i);
        displayPortTestPrint();
#endif
        if (i % 2 == 0) {
            displayPortTestBufferSubstring(8,  1, "5");

        } else {
            displayPortTestBufferIsEmpty();
        }
    }
}
*/
// STUBS
extern "C" {

    uint32_t micros() {
        return simulationTime;
    }

    uint32_t millis() {
        return micros() / 1000;
    }

    bool featureIsEnabled(uint32_t f) { return f; }
    void beeperConfirmationBeeps(uint8_t) {}
    bool isBeeperOn() { return false; }
    uint8_t getCurrentPidProfileIndex() { return 0; }
    uint8_t getCurrentControlRateProfileIndex() { return 0; }
    batteryState_e getBatteryState() { return BATTERY_OK; }
    uint8_t getBatteryCellCount() { return 4; }
    uint16_t getBatteryVoltage() { return 1680; }
    uint16_t getBatteryAverageCellVoltage() { return  420; }
    int32_t getAmperage() { return 0; }
    int32_t getMAhDrawn() { return 0; }
    int32_t getEstimatedAltitudeCm() { return 0; }
    int32_t getEstimatedVario() { return 0; }
    unsigned int blackboxGetLogNumber() { return 0; }
    bool isBlackboxDeviceWorking() { return true; }
    bool isBlackboxDeviceFull() { return false; }
    bool isSerialTransmitBufferEmpty(const serialPort_t *) { return false; }
    void serialWrite(serialPort_t *, uint8_t) {}
    bool cmsDisplayPortRegister(displayPort_t *) { return false; }
    uint16_t getCoreTemperatureCelsius(void) { return 0; }
    bool isFlipOverAfterCrashActive(void) { return false; }
    float pidItermAccelerator(void) { return 1.0; }
    uint8_t getMotorCount(void){ return 4; }
    bool areMotorsRunning(void){ return true; }
    bool pidOsdAntiGravityActive(void) { return false; }
    bool failsafeIsActive(void) { return false; }
    bool gpsRescueIsConfigured(void) { return false; }
    int8_t calculateThrottlePercent(void) { return 0; }
    uint32_t persistentObjectRead(persistentObjectId_e) { return 0; }
    void persistentObjectWrite(persistentObjectId_e, uint32_t) {}
    void failsafeOnRxSuspend(uint32_t ) {}
    void failsafeOnRxResume(void) {}
    void featureDisable(uint32_t) { }
    bool rxMspFrameComplete(void) { return false; }
    bool isPPMDataBeingReceived(void) { return false; }
    bool isPWMDataBeingReceived(void) { return false; }
    void resetPPMDataReceivedState(void){ }
    void failsafeOnValidDataReceived(void) { }
    void failsafeOnValidDataFailed(void) { }

    void rxPwmInit(rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
    }

    bool sbusInit(rxConfig_t *initialRxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(initialRxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool spektrumInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool sumdInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool sumhInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool crsfRxInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool jetiExBusInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool ibusInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool xBusInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    bool rxMspInit(rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, rcReadRawDataFnPtr *callback)
    {
        UNUSED(rxConfig);
        UNUSED(rxRuntimeConfig);
        UNUSED(callback);
        return true;
    }

    float pt1FilterGain(float f_cut, float dT)
    {
        UNUSED(f_cut);
        UNUSED(dT);
        return 0.0;
    }

    void pt1FilterInit(pt1Filter_t *filter, float k)
    {
        UNUSED(filter);
        UNUSED(k);
    }

    float pt1FilterApply(pt1Filter_t *filter, float input)
    {
        UNUSED(filter);
        UNUSED(input);
        return 0.0;
    }

}
