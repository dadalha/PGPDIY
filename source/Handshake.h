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

#ifndef __HANDSHAKE_H__
#define __HANDSHAKE_H__

#include "ble/BLE.h"
#include "CertificateService.h"


class Handshake
{
public:
    enum HandshakeStep
    {
        NONE,

        // No handshake ever done
        CONNECTED,
        BOND_DONE,
        SUBSCRIBED_SFIDA_COMMANDS,

        // Handshake process
        _HANDSHAKE_PAIRING_START,
        SENT_00_00_00_00,
        RESP_00_00_00_00,
        _HANDSHAKE_PAIRING_END
    };

public:
    static void Init(BLEDevice& ble, CertificateService* certificateService)
    {
        if (_instance == NULL)
        {
            _instance = new Handshake(ble, certificateService);
        }
    }

    static Handshake* Instance()
    {
        return _instance;
    }

    bool update()
    {
        if (_step == _pendingState) 
        {
            return false;
        }

        // Select the appropiate function
        bool (Handshake::*stateFnc)();
        switch (_pendingState)
        {
            case NONE:
                stateFnc = &Handshake::onReset;
                break;

            case CONNECTED:
                stateFnc = &Handshake::onConnect;
                break;

            case BOND_DONE:
                stateFnc = &Handshake::onBondDone;
                break;

            case SUBSCRIBED_SFIDA_COMMANDS:
                stateFnc = &Handshake::onSfidaSubscribed;
                break;

            case RESP_00_00_00_00:
                stateFnc = &Handshake::onPairingResponse;
                break;

            // Should never happen
            default:
                return false;
        }

        if ((this->*stateFnc)())
        {
            printf(" >] State change %d -> %d\n", _step, _pendingState);
            _step = _pendingState;
            return true;
        }

        printf(" X] Failed State change %d -> %d\n", _step, _pendingState);

        return false;
    }

private:
    void connectionCallBack(const Gap::ConnectionCallbackParams_t *params)
    {
        changeState(Handshake::CONNECTED);
    }

    void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
    {
        changeState(Handshake::NONE);
    }

    void onDataWritten(const GattWriteCallbackParams *params)
    {
        // Only if centralToSfida
        if (params->handle != _certificateService->centralToSfidaHandle())
        {
            return;
        }

        // Check step and change to next
        if (_step > _HANDSHAKE_PAIRING_START && _step < _HANDSHAKE_PAIRING_END)
        {
            _pendingState = (HandshakeStep)(_step + 1);
        }
    }

    static void onSubscribed(const GattAttribute::Handle_t handle)
    {
        if (handle != Handshake::Instance()->_certificateService->sfidaCommandsHandle())
        {
            return;
        }

        Handshake::Instance()->changeState(SUBSCRIBED_SFIDA_COMMANDS);
    }

    static void securitySetupCompletedCallback(Gap::Handle_t handle, SecurityManager::SecurityCompletionStatus_t status)
    {
        if (status == SecurityManager::SEC_STATUS_SUCCESS) {
            printf("Security success\r\n");
            Handshake::Instance()->changeState(Handshake::BOND_DONE);
        } else {
            printf("Security failed\r\n");
        }
    }

    bool changeState(HandshakeStep step)
    {
        // Steps are consecutive, so simply check it
        if (step != NONE && _step + 1 != step)
        {
            return false;
        }

        _pendingState = step;
        return true;
    }

    bool onReset()
    {
        // Nothing to do yet
        return true;
    }

    bool onConnect()
    {
        // Nothing to do yet
        return true;
    }

    bool onBondDone()
    {
        // Nothing to do yet
        return true;
    }

    bool onSfidaSubscribed()
    {
        uint8_t command[4] = {0, 0, 0, 0};
        uint8_t data[] = {
            // 4-115 "random data"
            0x32, 0x0a, 0x50, 0x23, 0x68, 0x09, 0xbe, 0x31, 0xfe, 0xc5, 0xeb, 0x67, 0xf4, 0xb5, 0x07, 0x11, 0x82, 0xe1, 0x34, 0x9b, 0xeb, 0x13, 0x69, 0xc4, 0xbd, 0x63, 0x2e, 0x37, 0x8a, 0xd5, 0x94, 0x53, 0x59, 0x19, 0x03, 0x59, 0x35, 0x0f, 0xa7, 0xb4, 0x86, 0xc5, 0xec, 0xcf, 0x6b, 0xb8, 0xbf, 0x30, 0x59, 0x5b, 0x31, 0x13, 0xb7, 0x24, 0x19, 0x4a, 0x85, 0x31, 0x34, 0xe7, 0xb0, 0xc6, 0x63, 0x7d, 0x5d, 0xab, 0x5c, 0xef, 0xcc, 0x4c, 0xbd, 0xb1, 0x78, 0x1f, 0x91, 0x7b, 0x54, 0xf8, 0x4f, 0xed, 0x1b, 0x87, 0x70, 0x37, 0x24, 0x4f, 0x5c, 0x61, 0x08, 0xde, 0x77, 0x0c, 0xe3, 0x5c, 0xa5, 0x9c, 0x93, 0x7d, 0xd1, 0xd8, 0xf8, 0x8f, 0x08, 0xa5, 0xb4, 0x20, 0x0f, 0x54, 0xcf, 0x29, 0x54, 0x4a,

            // MAC in reverse order (will be filled in later)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

            // 256 "device data"
            0x8c, 0x10, 0x78, 0x8b, 0xfd, 0xcc, 0xb8, 0x94, 0x91, 0xec, 0x06, 0x85, 0xac, 0x8f, 0xee, 0x50, 0xfd, 0xcf, 0xf3, 0x14, 0x63, 0x58, 0xb2, 0x9e, 0x91, 0xed, 0x7e, 0x17, 0xf9, 0x61, 0xdb, 0x26, 0x47, 0x5f, 0xdb, 0x11, 0xea, 0xf8, 0xcc, 0xf0, 0xe3, 0x2b, 0x45, 0x82, 0x8c, 0x29, 0xf6, 0xac, 0xc2, 0x7d, 0x3a, 0x50, 0x6c, 0x69, 0xc6, 0xb1, 0x6a, 0x72, 0x99, 0xc6, 0x8d, 0x5f, 0x68, 0x04, 0x8d, 0xb9, 0x73, 0x5b, 0xc3, 0x83, 0x95, 0x42, 0x76, 0x31, 0x7b, 0x13, 0x38, 0xc9, 0x09, 0x65, 0x3d, 0x5a, 0x86, 0xa3, 0x34, 0x35, 0xf0, 0x51, 0xf6, 0x97, 0xac, 0x11, 0xf5, 0x04, 0xcb, 0x43, 0x50, 0x34, 0x1e, 0x91, 0x4a, 0x70, 0xc6, 0xef, 0xa9, 0x8d, 0xa9, 0x08, 0x46, 0xc5, 0x39, 0x1b, 0xad, 0xad, 0x82, 0xff, 0x5d, 0x10, 0xbd, 0x4c, 0x2f, 0x9d, 0xfe, 0x5c, 0x04, 0x56, 0x3e, 0x88, 0x3a, 0xf4, 0x30, 0xac, 0xf5, 0x58, 0xf8, 0x29, 0x2a, 0x48, 0x74, 0x14, 0x1b, 0xd8, 0xad, 0x64, 0x75, 0x8c, 0x26, 0x26, 0xc5, 0x9c, 0x7c, 0x49, 0x33, 0x4f, 0x15, 0xa2, 0x1b, 0x89, 0x90, 0x03, 0x7f, 0xc2, 0x5f, 0xe5, 0x97, 0xc9, 0x55, 0x2d, 0x25, 0xef, 0x4b, 0x67, 0x85, 0x99, 0x9d, 0xc1, 0xfb, 0x27, 0x20, 0xdb, 0x97, 0x7d, 0x1c, 0xfe, 0x58, 0x5a, 0x6a, 0xd3, 0x9d, 0x2e, 0x1f, 0x18, 0x68, 0xd8, 0x6e, 0xa5, 0xe0, 0x0e, 0x47, 0x1f, 0xe5, 0x29, 0x94, 0xad, 0x97, 0x35, 0xb6, 0xac, 0xa4, 0x17, 0x17, 0x56, 0x04, 0xa8, 0x74, 0x78, 0x61, 0x42, 0x15, 0xb9, 0x07, 0x4b, 0x76, 0x3a, 0x16, 0x0b, 0x11, 0x84, 0xd9, 0x74, 0xfc, 0x4b, 0xc9, 0x49, 0x95, 0x84, 0xca, 0x3e, 0x46, 0xc6, 0xda, 0xee, 0x8b, 0xbe, 0xa1, 0x3a, 0xcd, 0xd7, 0xc0, 0x7a, 0x42, 0x58, 0x27, 0x4a, 0xd6, 0x0a
        };

        // Fill in MAC
        getMAC((uint8_t*)((intptr_t)data + 112));

        _certificateService->send(command, data, sizeof(data));
        _pendingState = SENT_00_00_00_00; // _step will be overwritten after returning

        return true;
    }

    bool onPairingResponse()
    {
        uint16_t bytesRead;
        uint8_t* buffer = _certificateService->read(&bytesRead);

        // Answer should be 20 bytes long
        if (bytesRead != 20)
        {
            return false;
        }

        // First 4 bytes should match
        if (buffer[0] != 0x01 || buffer[1] != 0x00 || buffer[2] != 0x00 || buffer[3] != 0x00)
        {
            return false;
        }

        // 16 bytes remaning (IV? Key?)
        // Answer is 01-00-00-00 + 48 bytes

        printf("Read %d bytes\n", bytesRead);

        return true;
    }

    void getMAC(uint8_t* address)
    {
        // Address is already in reverse order
        Gap::AddressType_t addr_type;
        BLE::Instance().gap().getAddress(&addr_type, address);
    }

private:
    Handshake(BLEDevice &ble, CertificateService* certificateService) :
        _ble(ble),
        _step(NONE),
        _pendingState(NONE)
    {
        _certificateService = certificateService;

        // Register callbacks
        _ble.gap().onConnection(this, &Handshake::connectionCallBack);
        _ble.gap().onDisconnection(this, &Handshake::disconnectionCallback);
        _ble.gattServer().onDataWritten(this, &Handshake::onDataWritten);
        _ble.gattServer().onUpdatesEnabled(&Handshake::onSubscribed);
        _ble.securityManager().onSecuritySetupCompleted(&Handshake::securitySetupCompletedCallback);
    }

private:
    static Handshake* _instance;

    BLEDevice& _ble;
    CertificateService* _certificateService;

    HandshakeStep _step;
    HandshakeStep _pendingState;
};

#endif /* #ifndef __HANDSHAKE_H__ */
