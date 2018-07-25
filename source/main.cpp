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
#include "LEDService.h"
#include "FirmwareService.h"
#include "ControlService.h"
#include "CertificateService.h"
#include "BatteryService.h"
#include "Handshake.h"


DigitalOut alivenessLED(LED1, 0);
//DigitalOut actuatedLED(LED2, 0);

// const static char     DEVICE_NAME[] = "Podemos GO Plus";
const static char     DEVICE_NAME[] = "Pokemon GO Plus";
const static uint8_t  VENDOR_ID[] = {0x62, 0x04, 0xC5, 0x21, 0x00};

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

LEDService *ledServicePtr;
FirmwareService* firmwareServire;
ControlService* controlService;
CertificateService* certificateService;
BatteryService* batteryService;
Handshake* handshake;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    printf("DISCONNECTED: %x\n", params->reason);
    handshake->update(Handshake::ZERO);

    if (params->reason == 0x3D)
    {
        BLE::Instance().securityManager().purgeAllBondingState();
    }

    BLE::Instance().gap().startAdvertising();
}

void connectionCallBack(const Gap::ConnectionCallbackParams_t *params ) {
    printf("CONNECTED\n");
    handshake->init();
}

void blinkCallback(void)
{
    alivenessLED = !alivenessLED; /* Do blinky on LED1 to indicate system aliveness. */
}

/**
 * This callback allows the LEDService to receive updates to the ledState Characteristic.
 *
 * @param[in] params
 *     Information about the characterisitc being updated.
 */
void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    
    if (params->handle == controlService->buttonHandle()) {
        printf("WRITE BUTTON\n");
    }else if (params->handle == controlService->ledVibrateHandle()) {
        printf("WRITE LEDVIBRATE\n");
    }else if (params->handle == controlService->unkHandle()) {
        printf("WRITE UNK\n");
    }else if (params->handle == controlService->updateRequestHandle()) {
        printf("WRITE UPDATEREQUEST\n");
    }else if (params->handle == controlService->versionHandle()) {
        printf("WRITE VERSION\n");
    }
    else if (params->handle == certificateService->centralToSfidaHandle()) {
        printf("WRITE CENTRAL TO SFIDA\n");
    }else if (params->handle == certificateService->sfidaCommandsHandle()) {
        printf("WRITE SFIDA COMMANDS\n");
    }else if (params->handle == certificateService->sfidaToCentralHandle()) {
        printf("WRITE SFIDA TO CENTRAL\n");
    }

    // if ((params->handle == ledServicePtr->getValueHandle()) && (params->len == 1)) {
    //     //actuatedLED = *(params->data);
    // }
}

void onUpdatesEnabled(const GattAttribute::Handle_t handle) {
    if (handle == controlService->buttonHandle()) {
        printf("NOTIFY BUTTON\n");
    }else if (handle == controlService->ledVibrateHandle()) {
        printf("NOTIFY LEDVIBRATE\n");
    }else if (handle == controlService->unkHandle()) {
        printf("NOTIFY UNK\n");
    }else if (handle == controlService->updateRequestHandle()) {
        printf("NOTIFY UPDATEREQUEST\n");
    }else if (handle == controlService->versionHandle()) {
        printf("NOTIFY VERSION\n");
    }
    else if (handle == certificateService->centralToSfidaHandle()) {
        printf("NOTIFY CENTRAL TO SFIDA\n");
    }else if (handle == certificateService->sfidaCommandsHandle()) {
        printf("NOTIFY SFIDA COMMANDS\n");
        handshake->update(Handshake::SUBSCRIBED_SFIDA_COMMANDS);
    }else if (handle == certificateService->sfidaToCentralHandle()) {
        printf("NOTIFY SFIDA TO CENTRAL\n");
    }
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

void securitySetupCompletedCallback(Gap::Handle_t handle, SecurityManager::SecurityCompletionStatus_t status)
{
    if (status == SecurityManager::SEC_STATUS_SUCCESS) {
        printf("Security success\r\n");
        handshake->update(Handshake::BOND_DONE);
    } else {
        printf("Security failed\r\n");
    }
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
    // ble.securityManager().purgeAllBondingState();
    ble.securityManager().onSecuritySetupCompleted(securitySetupCompletedCallback);
    
    /* Some config */
    ble.gap().setDeviceName((uint8_t*)DEVICE_NAME);  // Char 0x2A00
    // ble.gap().setAppearance(GapAdvertisingData::UNKNOWN);  // Char 0x2A01 - Doesn't work, uses AdversitingData class
    // ble.gap().enablePrivacy(false);

    // Gap::PeripheralPrivacyConfiguration_t configuration;
    // ble.gap().getPeripheralPrivacyConfiguration(&configuration);
    // configuration.resolution_strategy = Gap::PeripheralPrivacyConfiguration_t::DO_NOT_RESOLVE;

    /* Callbacks */
    ble.gap().onConnection(connectionCallBack);
    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);
    ble.gattServer().onUpdatesEnabled(onUpdatesEnabled);

    // bool initialValueForLEDCharacteristic = false;
    //ledServicePtr = new LEDService(ble, initialValueForLEDCharacteristic);
    //firmwareServire = new FirmwareService(ble);
    controlService = new ControlService(ble);
    certificateService = new CertificateService(ble);
    batteryService = new BatteryService(ble);
    handshake = new Handshake(ble, certificateService);

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
