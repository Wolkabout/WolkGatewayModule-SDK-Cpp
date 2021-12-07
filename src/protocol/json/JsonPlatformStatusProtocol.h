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

#ifndef WOLKGATEWAYMODULE_JSONPLATFORMSTATUSPROTOCOL_H
#define WOLKGATEWAYMODULE_JSONPLATFORMSTATUSPROTOCOL_H

#include "protocol/PlatformStatusProtocol.h"

namespace wolkabout
{
/**
 * This is the implementation of the PlatformStatusProtocol for the current JSON protocol for local MQTT.
 */
class JsonPlatformStatusProtocol : public PlatformStatusProtocol
{
public:
    /**
     * This is an overridden method from the `Protocol` interface.
     * This is the method that returns the entire list of topics that need to be subscribed to for this protocol.
     *
     * @return The list of topics as strings.
     */
    std::vector<std::string> getInboundChannels() const override;

    /**
     * This is an overridden method from the `Protocol` interface.
     * This is the method that returns the entire list of topics that need to be subscribed to for a specific device
     * key.
     *
     * @param deviceKey The device key for which topics need to be created.
     * @return The list of topics that include the device key.
     */
    std::vector<std::string> getInboundChannelsForDevice(const std::string& deviceKey) const override;

    /**
     * This is an overridden method from the `Protocol` interface.
     * This is the method that returns the device key from the topic that has been passed.
     *
     * @param topic The topic from which the device key should be extracted.
     * @return The device key extracted from the topic, if one could be found.
     */
    std::string extractDeviceKeyFromChannel(const std::string& topic) const override;

    /**
     * This is an overridden method from the `PlatformStatusProtocol` interface.
     * This is the method that attempts to parse an incoming `PlatformStatusMessage`.
     *
     * @param message A received MQTT message that is most likely a `PlatformStatusMessage`.
     * @return A parsed instance of the `PlatformStatusMessage`. If failed to parse, a nullptr will be returned.
     */
    std::unique_ptr<PlatformStatusMessage> parsePlatformStatusMessage(const std::shared_ptr<Message>& message) override;
};
}    // namespace wolkabout

#endif    // WOLKGATEWAYMODULE_JSONPLATFORMSTATUSPROTOCOL_H
