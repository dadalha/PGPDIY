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
    };

    Handshake(BLEDevice &ble, CertificateService* certificateService) :
        _ble(ble),
        _step(NONE),
        _pendingState(NONE)
    {
        _certificateService = certificateService;
    }

    bool changeState(HandshakeStep step)
    {
        switch (step)
        {
            case NONE:
                break;

            case CONNECTED:
                if (_step != NONE) 
                {
                    return false;
                }
                break;

            case BOND_DONE:
                if (_step != CONNECTED)
                {
                    return false;
                }
                break;

            case SUBSCRIBED_SFIDA_COMMANDS:
                if (_step != BOND_DONE)
                {
                    return false;
                }
                break;
        }

        _pendingState = step;
        return true;
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

            // Should never happen
            default:
                return false;
        }

        if ((this->*stateFnc)())
        {
            _step = _pendingState;
            return true;
        }

        return false;
    }

private:
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
        _certificateService->send(command);
        return true;
    }

private:
    BLEDevice& _ble;
    CertificateService* _certificateService;

    HandshakeStep _step;
    HandshakeStep _pendingState;
};

#endif /* #ifndef __HANDSHAKE_H__ */
