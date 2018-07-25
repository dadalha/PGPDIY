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
        // No handshake ever done
        ZERO,
        BOND_DONE,
        SUBSCRIBED_SFIDA_COMMANDS,
    };

    Handshake(BLEDevice &ble, CertificateService* certificateService) :
        _ble(ble),
        _step(ZERO)
    {
        _certificateService = certificateService;
    }

    bool init()
    {
        // TODO: Do we need anything?
        return true;
    }

    bool update(HandshakeStep step)
    {
        switch (step)
        {
            case ZERO:
                return onReset();

            case BOND_DONE:
                return onBondingDone();

            case SUBSCRIBED_SFIDA_COMMANDS:
                return onSfidaSubscribed();
        }

        return false;
    }

private:
    bool onReset()
    {
        _step = ZERO;
        return true;
    }

    bool onBondingDone()
    {
        if (_step != ZERO)
        {
            return false;
        }

        _step = BOND_DONE;
        return true;
    }

    bool onSfidaSubscribed()
    {
        if (_step != BOND_DONE)
        {
            return false;
        }

        //uint8_t command[4] = {0, 0, 0, 0};
        //_certificateService->send(command);

        _step = SUBSCRIBED_SFIDA_COMMANDS;
        return true;
    }

private:
    BLEDevice& _ble;
    CertificateService* _certificateService;

    HandshakeStep _step;
};

#endif /* #ifndef __HANDSHAKE_H__ */
