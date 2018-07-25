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

#ifndef __BLE_CONTROL_SERVICE_H__
#define __BLE_CONTROL_SERVICE_H__

#include "ble/BLE.h"


#define CONTROL_SERVICE_UUID                "21c5046267cb63a35c4c82b5b9939aeb"
#define LED_VIBRATE_CHARACTERISTIC_UUID     "21c5046267cb63a35c4c82b5b9939aec"
#define BUTTON_CHARACTERISTIC_UUID          "21c5046267cb63a35c4c82b5b9939aed"

#define UNK_CHARACTERISTIC_UUID             "21c5046267cb63a35c4c82b5b9939aee"
#define UPDATE_REQUEST_CHARACTERISTIC_UUID  "21c5046267cb63a35c4c82b5b9939aef"
#define VERSION_CHARACTERISTIC_UUID         "21c5046267cb63a35c4c82b5b9939af0"

class ControlService {
public:

    ControlService(BLEDevice &ble) :
        _ble(ble),
        _ledVibrateChar(LED_VIBRATE_CHARACTERISTIC_UUID, (uint8_t*)_pattern, 0, 256, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
        _buttonChar(BUTTON_CHARACTERISTIC_UUID, &_button, 1, 1,  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),

        _unkChar(UNK_CHARACTERISTIC_UUID, &_unk),
        _updateRequestChar(UPDATE_REQUEST_CHARACTERISTIC_UUID, &_update),
        _versionChar(VERSION_CHARACTERISTIC_UUID, &_version)
    {
        _buttonChar.setSecurityRequirements(
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED,
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED,
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED);

        GattCharacteristic *charTable[] = {&_ledVibrateChar, &_buttonChar, &_unkChar, &_updateRequestChar, &_versionChar};
        GattService         controlService(CONTROL_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble_error_t res = ble.gattServer().addService(controlService);
        printf("ControlService: %d\n", res);
    }

    GattAttribute::Handle_t ledVibrateHandle() const
    {
        return _ledVibrateChar.getValueHandle();
    }
    
    GattAttribute::Handle_t buttonHandle() const
    {
        return _buttonChar.getValueHandle();
    }
    
    GattAttribute::Handle_t unkHandle() const
    {
        return _unkChar.getValueHandle();
    }

    GattAttribute::Handle_t updateRequestHandle() const
    {
        return _updateRequestChar.getValueHandle();
    }

    GattAttribute::Handle_t versionHandle() const
    {
        return _versionChar.getValueHandle();
    }

private:
    BLEDevice& _ble;
    GattCharacteristic _ledVibrateChar;
    GattCharacteristic _buttonChar;

    WriteOnlyGattCharacteristic<uint8_t> _unkChar;
    WriteOnlyGattCharacteristic<bool> _updateRequestChar;
    ReadOnlyGattCharacteristic<uint8_t> _versionChar;

    uint8_t _pattern[256];
    uint8_t _button;

    uint8_t _unk;
    bool _update;
    uint8_t _version;
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */
