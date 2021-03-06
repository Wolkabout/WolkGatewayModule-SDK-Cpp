/*
 * Copyright 2018 WolkAbout Technology s.r.o.
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

#ifndef DEVICESTATUSSERVICE_H
#define DEVICESTATUSSERVICE_H

#include "core/InboundMessageHandler.h"
#include "core/model/DeviceStatus.h"

#include <functional>
#include <string>

namespace wolkabout
{
class StatusProtocol;
class ConnectivityService;

typedef std::function<void(const std::string&)> StatusRequestHandler;

class DeviceStatusService : public MessageListener
{
public:
    DeviceStatusService(StatusProtocol& protocol, ConnectivityService& connectivityService,
                        const StatusRequestHandler& statusRequestHandler);

    void messageReceived(std::shared_ptr<Message> message) override;
    const Protocol& getProtocol() override;

    void publishDeviceStatusUpdate(const std::string& deviceKey, DeviceStatus::Status status);

    void publishDeviceStatusResponse(const std::string& deviceKey, DeviceStatus::Status status);

    void devicesUpdated(const std::vector<std::string>& deviceKeys);

private:
    StatusProtocol& m_protocol;
    ConnectivityService& m_connectivityService;

    StatusRequestHandler m_statusRequestHandler;
};
}    // namespace wolkabout

#endif    // DEVICESTATUSSERVICE_H
