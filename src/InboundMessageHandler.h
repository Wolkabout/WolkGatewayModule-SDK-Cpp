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

#ifndef INBOUNDMESSAGEHANDLER_H
#define INBOUNDMESSAGEHANDLER_H

#include "connectivity/ConnectivityService.h"
#include "model/ActuatorCommand.h"
#include "model/ActuatorGetCommand.h"
#include "model/ActuatorSetCommand.h"
#include "model/BinaryData.h"
#include "model/Device.h"
#include "model/DeviceRegistrationResponseDto.h"
#include "model/FirmwareUpdateCommand.h"
#include "utilities/CommandBuffer.h"

#include <string>
#include <vector>

namespace wolkabout
{
class InboundMessageHandler : public ConnectivityServiceListener
{
public:
    InboundMessageHandler(Device device);

    void messageReceived(const std::string& topic, const std::string& message) override;

    const std::vector<std::string>& getTopics() const override;

    void setActuatorSetCommandHandler(std::function<void(ActuatorSetCommand)> handler);

    void setActuatorGetCommandHandler(std::function<void(ActuatorGetCommand)> handler);

    void setBinaryDataHandler(std::function<void(BinaryData)> handler);

    void setFirmwareUpdateCommandHandler(std::function<void(FirmwareUpdateCommand)> handler);

    void setRegistrationResponseHandler(std::function<void(std::shared_ptr<DeviceRegistrationResponse>)> handler);

private:
    void addToCommandBuffer(std::function<void()> command);

    Device m_device;

    std::unique_ptr<CommandBuffer> m_commandBuffer;

    std::vector<std::string> m_subscriptionList;

    std::function<void(ActuatorSetCommand)> m_actuationSetHandler;
    std::function<void(ActuatorGetCommand)> m_actuationGetHandler;
    std::function<void(BinaryData)> m_binaryDataHandler;
    std::function<void(FirmwareUpdateCommand)> m_firmwareUpdateHandler;
    std::function<void(std::shared_ptr<DeviceRegistrationResponse>)> m_registrationResponseHandler;

    static const constexpr char* ACTUATION_GET_TOPIC_ROOT = "p2d/actuator_get/d/";
    static const constexpr char* ACTUATION_SET_TOPIC_ROOT = "p2d/actuator_set/d/";
    static const constexpr char* REGISTRATION_RESPONSE_ROOT = "p2d/registration/d/";
    static const constexpr char* FIRMWARE_UPDATE_TOPIC_ROOT = "service/commands/firmware/";
    static const constexpr char* BINARY_TOPIC_ROOT = "service/binary/";
};
}

#endif    // INBOUNDMESSAGEHANDLER_H
