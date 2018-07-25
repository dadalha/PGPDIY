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

#ifndef __BLE_FIRMWARE_SERVICE_H__
#define __BLE_FIRMWARE_SERVICE_H__

#include "ble/BLE.h"


#define FIRMWARE_SERVICE_UUID              "0000fef5000010008000-00805f9b34fb"
#define UPDATE_REQUEST_CHARACTERISTIC_UUID "21c5046267cb63a35c4c82b5b9939aef"
#define VERSION_CHARACTERISTIC_UUID        "21c5046267cb63a35c4c82b5b9939af0"


class FirmwareService {
public:

    FirmwareService(BLEDevice &ble) :
        _ble(ble),
        _updateRequestChar(UPDATE_REQUEST_CHARACTERISTIC_UUID, &_request, 1, 1, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
        _versionChar(VERSION_CHARACTERISTIC_UUID, &_version, 1, 1,  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ)
    {
        GattCharacteristic *charTable[] = {&_updateRequestChar, &_versionChar};
        GattService         firmwareService(FIRMWARE_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        printf("FirmwareService: %d\n", ble.gattServer().addService(firmwareService));
    }

private:
    BLEDevice& _ble;
    GattCharacteristic _updateRequestChar;
    GattCharacteristic _versionChar;

    uint8_t _request;
    uint8_t _version;
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */
