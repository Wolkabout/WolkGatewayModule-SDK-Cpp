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

#include "core/utilities/Logger.h"
#include "protocol/json/JsonPlatformStatusProtocol.h"

namespace wolkabout
{
const std::string PLATFORM_STATUS_CHANNEL = "p2d/connection_status";

std::vector<std::string> JsonPlatformStatusProtocol::getInboundChannels() const
{
    return {PLATFORM_STATUS_CHANNEL};
}

std::vector<std::string> JsonPlatformStatusProtocol::getInboundChannelsForDevice(
  const std::string& /** deviceKey **/) const
{
    return {};
}

std::string JsonPlatformStatusProtocol::extractDeviceKeyFromChannel(const std::string& /** topic **/) const
{
    // Absolutely no messages in this protocol actually require a device key to be extracted from the topic.
    return {};
}

std::unique_ptr<PlatformStatusMessage> JsonPlatformStatusProtocol::parsePlatformStatusMessage(
  const std::shared_ptr<Message>& message)
{
    LOG(TRACE) << METHOD_INFO;

    // Check that the topic is correct
    if (message->getChannel() != PLATFORM_STATUS_CHANNEL)
    {
        LOG(ERROR) << "Failed to parse incoming 'PlatformStatusMessage' -> The topic is not correct!";
        return nullptr;
    }

    // Now try to parse the content
    auto status = fromString(message->getContent());
    if (status == ConnectivityStatus::NONE)
    {
        LOG(ERROR) << "Failed to parse incoming 'PlatformStatusMessage' -> The content is not a 'ConnectionStatus'.";
        return nullptr;
    }

    // Now create a message
    return std::unique_ptr<PlatformStatusMessage>(new PlatformStatusMessage(status));
}
}    // namespace wolkabout
