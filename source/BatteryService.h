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

#ifndef __BLE_BATTERY_SERVICE_H__
#define __BLE_BATTERY_SERVICE_H__

#include "ble/BLE.h"


#define BATTERY_SERVICE_UUID                "0000180f00001000800000805f9b34fb"
#define LEVEL_CHARACTERISTIC_UUID           "00002a1900001000800000805f9b34fb"


class BatteryService {
public:

    BatteryService(BLEDevice &ble) :
        _ble(ble),
        _levelChar(LEVEL_CHARACTERISTIC_UUID, &_level)
    {
        GattCharacteristic *charTable[] = {&_levelChar};
        GattService         batteryService(BATTERY_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble_error_t res = ble.gattServer().addService(batteryService);
        printf("BatteryService: %d\n", res);
    }

    GattAttribute::Handle_t levelHandle() const
    {
        return _levelChar.getValueHandle();
    }

private:
    BLEDevice& _ble;
    ReadOnlyGattCharacteristic<uint8_t> _levelChar;

    uint8_t _level;
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */
