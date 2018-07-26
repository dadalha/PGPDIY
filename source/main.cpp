/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "FirmwareService.h"
#include "ControlService.h"
#include "CertificateService.h"
#include "BatteryService.h"
#include "Handshake.h"


DigitalOut alivenessLED(LED1, 0);

// const static char     DEVICE_NAME[] = "Podemos GO Plus";
const static char     DEVICE_NAME[] = "Pokemon GO Plus";
const static uint8_t  VENDOR_ID[] = {0x62, 0x04, 0xC5, 0x21, 0x00};

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

FirmwareService* firmwareServire;
ControlService* controlService;
CertificateService* certificateService;
BatteryService* batteryService;

// Static declarations
Handshake* Handshake::_instance = NULL;


void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    printf("Disconnection reason: %x\n", params->reason);

    if (params->reason == 0x3D)
    {
        BLE::Instance().securityManager().purgeAllBondingState();
    }

    BLE::Instance().gap().startAdvertising();
}

void connectionCallBack(const Gap::ConnectionCallbackParams_t *params )
{
    printf("Connected\n");
}

void blinkCallback(void)
{
    alivenessLED = !alivenessLED; /* Do blinky on LED1 to indicate system aliveness. */
    Handshake::Instance()->update();
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

void printMacAddress()
{
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);
    printf("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
        printf("%02x:", address[i]);
    }
    printf("%02x\r\n", address[0]);
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    /* Security */
    bool enableBonding = true;
    bool requireMITM   = false;
    ble.securityManager().init(enableBonding, requireMITM, SecurityManager::IO_CAPS_NONE);
    
    /* Some config */
    ble.gap().setDeviceName((uint8_t*)DEVICE_NAME);  // Char 0x2A00
    // ble.gap().setAppearance(GapAdvertisingData::UNKNOWN);  // Char 0x2A01 - Doesn't work, uses AdversitingData class

    /* Callbacks */
    ble.gap().onConnection(connectionCallBack);
    ble.gap().onDisconnection(disconnectionCallback);

    //firmwareServire = new FirmwareService(ble);
    controlService = new ControlService(ble);
    certificateService = new CertificateService(ble);
    batteryService = new BatteryService(ble);
    
    // Initial setup
    Handshake::Init(ble, certificateService);

    /* setup advertising */
    GapAdvertisingData advertisingData;
    advertisingData.addAppearance(GapAdvertisingData::UNKNOWN);
    advertisingData.addFlags(GapAdvertisingData::LE_GENERAL_DISCOVERABLE | GapAdvertisingData::BREDR_NOT_SUPPORTED);
    advertisingData.addData(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t*)DEVICE_NAME, sizeof(DEVICE_NAME));
    advertisingData.addData((GapAdvertisingData::DataType_t)0x20, VENDOR_ID, sizeof(VENDOR_ID));
    
    ble.gap().setAdvertisingPayload(advertisingData);
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

    printMacAddress();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    eventQueue.call_every(500, blinkCallback);

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}
