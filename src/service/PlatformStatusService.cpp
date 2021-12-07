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
#include "service/PlatformStatusService.h"

#include <utility>

namespace wolkabout
{
PlatformStatusService::PlatformStatusService(PlatformStatusProtocol& protocol,
                                             std::shared_ptr<PlatformStatusListener> listener)
: m_protocol(protocol), m_listener(std::move(listener))
{
}

PlatformStatusService::PlatformStatusService(PlatformStatusProtocol& protocol, PlatformStatusCallback callback)
: m_protocol(protocol), m_lambda(std::move(callback))
{
}

void PlatformStatusService::messageReceived(std::shared_ptr<Message> message)
{
    LOG(TRACE) << METHOD_INFO;

    // Try to parse the message with the protocol
    auto parsed = std::shared_ptr<PlatformStatusMessage>(m_protocol.parsePlatformStatusMessage(message));
    if (parsed == nullptr)
    {
        LOG(ERROR) << "Failed to handle received message -> The message was not parsed.";
        return;
    }

    // Now, do an external call with the received data.
    if (m_listener)
    {
        m_commandBuffer.pushCommand(std::make_shared<std::function<void()>>(
          [this, parsed]() { m_listener->platformStatus(parsed->getStatus()); }));
    }
    else if (m_lambda)
    {
        m_commandBuffer.pushCommand(
          std::make_shared<std::function<void()>>([this, parsed]() { m_lambda(parsed->getStatus()); }));
    }
}

const Protocol& PlatformStatusService::getProtocol()
{
    return m_protocol;
}
}    // namespace wolkabout
