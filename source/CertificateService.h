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

#ifndef __BLE_CERTIFICATE_SERVICE_H__
#define __BLE_CERTIFICATE_SERVICE_H__

#include "ble/BLE.h"


#define CERTIFICATE_SERVICE_UUID                "bbe877095b894433ab7f8b8eef0d8e37"
#define SFIDA_COMMANDS_CHARACTERISTIC_UUID      "bbe877095b894433ab7f8b8eef0d8e39"
#define CENTRAL_TO_SFIDA_CHARACTERISTIC_UUID    "bbe877095b894433ab7f8b8eef0d8e38"
#define SFIDA_TO_CENTRAL_CHARACTERISTIC_UUID    "bbe877095b894433ab7f8b8eef0d8e3a"


class CertificateService {
public:

    CertificateService(BLEDevice &ble) :
        _ble(ble),
        _sfidaCommandsChar(SFIDA_COMMANDS_CHARACTERISTIC_UUID, (uint8_t*)_command, 0, 4, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        _centralToSfidaChar(CENTRAL_TO_SFIDA_CHARACTERISTIC_UUID, (uint8_t*)_readBuffer, 0, 256, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
        _sfidaToCentralChar(SFIDA_TO_CENTRAL_CHARACTERISTIC_UUID, (uint8_t*)_writeBuffer, 0, 378, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ)
    {
        _sfidaCommandsChar.setSecurityRequirements(
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED,
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED,
            GattCharacteristic::SecurityRequirement_t::UNAUTHENTICATED);

        GattCharacteristic *charTable[] = {&_sfidaCommandsChar, &_centralToSfidaChar, &_sfidaToCentralChar};
        GattService         certificateService(CERTIFICATE_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble_error_t res = _ble.gattServer().addService(certificateService);
        printf("CertificateService: %d\n", res);
    }

    GattAttribute::Handle_t sfidaCommandsHandle() const
    {
        return _sfidaCommandsChar.getValueHandle();
    }

    GattAttribute::Handle_t centralToSfidaHandle() const
    {
        return _centralToSfidaChar.getValueHandle();
    }

    GattAttribute::Handle_t sfidaToCentralHandle() const
    {
        return _sfidaToCentralChar.getValueHandle();
    }

    void send(uint8_t (&command)[4], uint8_t* data = NULL, uint16_t len = 0)
    {
        // Copy command to both buffers
        memcpy((void*)_command, (void*)command, 4);
        memcpy((void*)_writeBuffer, (void*)command, 4);

        // If there is any data, copy to sfidaToCentral
        if (data != NULL)
        {
            memcpy((void*)((intptr_t)_writeBuffer + 4), (void*)data, (size_t)len);
        }

        // Write sfidaToCentral
        ble_error_t res = _ble.gattServer().write(
            sfidaToCentralHandle(),
            _writeBuffer,
            len + 4
        );

        printf(" >] Write sfidaToCentral: %d\n", res);

        // Write sfidaCommands
        res = _ble.gattServer().write(
            sfidaCommandsHandle(),
            _command,
            4
        );

        printf(" >] Write sfidaCommands: %d\n", res);
    }
    
    uint8_t* read(uint16_t* bytesRead)
    {
        _ble.gattServer().read(
            centralToSfidaHandle(),
            _readBuffer,
            bytesRead
        );

        return _readBuffer;
    }

private:
    BLEDevice& _ble;
    GattCharacteristic _sfidaCommandsChar;
    GattCharacteristic _centralToSfidaChar;
    GattCharacteristic _sfidaToCentralChar;

    uint8_t _command[4];
    uint8_t _readBuffer[378];
    uint8_t _writeBuffer[256];
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */
