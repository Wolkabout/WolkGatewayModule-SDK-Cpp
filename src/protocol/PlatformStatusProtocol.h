/**
 * Copyright 2021 Wolkabout s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef WOLKGATEWAYMODULE_PLATFORMSTATUSPROTOCOL_H
#define WOLKGATEWAYMODULE_PLATFORMSTATUSPROTOCOL_H

#include "core/model/Message.h"
#include "core/protocol/Protocol.h"
#include "protocol/json/messages/PlatformStatusMessage.h"

namespace wolkabout
{
/**
 * This is a definition of a protocol that is meant for receiving platform connectivity status messages.
 */
class PlatformStatusProtocol : public Protocol
{
public:
    /**
     * This is a method that is capable of parsing an incoming PlatformStatusMessage.
     *
     * @param message The incoming MQTT message.
     * @return The parsed PlatformStatusMessage. If the message can not be parsed, a nullptr will be returned.
     */
    virtual std::unique_ptr<PlatformStatusMessage> parsePlatformStatusMessage(
      const std::shared_ptr<Message>& message) = 0;
};
}    // namespace wolkabout

#endif    // WOLKGATEWAYMODULE_PLATFORMSTATUSPROTOCOL_H
