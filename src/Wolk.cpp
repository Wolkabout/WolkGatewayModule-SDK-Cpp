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

#include "Wolk.h"
#include "ActuationHandler.h"
#include "ActuatorStatusProvider.h"
#include "WolkBuilder.h"
#include "connectivity/ConnectivityService.h"
#include "model/ActuatorCommand.h"
#include "model/ActuatorStatus.h"
#include "model/Device.h"
#include "model/DeviceRegistrationRequestDto.h"
#include "protocol/json/RegistrationProtocol.h"
#include "service/DataService.h"
#include "service/FirmwareUpdateService.h"
#include "utilities/Logger.h"

#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

#define INSTANTIATE_ADD_SENSOR_READING_FOR(x)                                                                    \
    template void Wolk::addSensorReading<x>(const std::string& deviceKey, const std::string& reference, x value, \
                                            unsigned long long rtc)

namespace wolkabout
{
WolkBuilder Wolk::newBuilder()
{
    return WolkBuilder();
}

template <>
void Wolk::addSensorReading(const std::string& deviceKey, const std::string& reference, std::string value,
                            unsigned long long rtc)
{
    if (rtc == 0)
    {
        rtc = Wolk::currentRtc();
    }

    addToCommandBuffer([=]() -> void { m_dataService->addSensorReading(deviceKey, reference, value, rtc); });
}

template <typename T>
void Wolk::addSensorReading(const std::string& deviceKey, const std::string& reference, T value, unsigned long long rtc)
{
    addSensorReading(deviceKey, reference, std::to_string(value), rtc);
}

template <>
void Wolk::addSensorReading(const std::string& deviceKey, const std::string& reference, bool value,
                            unsigned long long rtc)
{
    addSensorReading(deviceKey, reference, std::string(value ? "true" : "false"), rtc);
}

template <>
void Wolk::addSensorReading(const std::string& deviceKey, const std::string& reference, char* value,
                            unsigned long long rtc)
{
    addSensorReading(deviceKey, reference, std::string(value), rtc);
}

template <>
void Wolk::addSensorReading(const std::string& deviceKey, const std::string& reference, const char* value,
                            unsigned long long rtc)
{
    addSensorReading(deviceKey, reference, std::string(value), rtc);
}

INSTANTIATE_ADD_SENSOR_READING_FOR(float);
INSTANTIATE_ADD_SENSOR_READING_FOR(double);
INSTANTIATE_ADD_SENSOR_READING_FOR(signed int);
INSTANTIATE_ADD_SENSOR_READING_FOR(signed long int);
INSTANTIATE_ADD_SENSOR_READING_FOR(signed long long int);
INSTANTIATE_ADD_SENSOR_READING_FOR(unsigned int);
INSTANTIATE_ADD_SENSOR_READING_FOR(unsigned long int);
INSTANTIATE_ADD_SENSOR_READING_FOR(unsigned long long int);

void Wolk::addAlarm(const std::string& deviceKey, const std::string& reference, const std::string& value,
                    unsigned long long rtc)
{
    if (rtc == 0)
    {
        rtc = Wolk::currentRtc();
    }

    addToCommandBuffer([=]() -> void { m_dataService->addAlarm(deviceKey, reference, value, rtc); });
}

void Wolk::publishActuatorStatus(const std::string& deviceKey, const std::string& reference)
{
    addToCommandBuffer([=]() -> void {
        m_dataService->acquireActuatorStatus(deviceKey, reference);
        m_dataService->publishActuatorStatuses();
    });
}

void Wolk::registerDevice(const Device& device)
{
    DeviceRegistrationRequest request{device};
    auto message = RegistrationProtocol::make(device.getKey(), request);

    if (message)
    {
        addToCommandBuffer([=]() -> void { m_connectivityService->publish(message); });
    }
}

void Wolk::registerDevices() {}

void Wolk::connect()
{
    addToCommandBuffer([=]() -> void {
        if (m_connectivityService->connect())
        {
            registerDevices();

            for (const auto& kvp : m_devices)
            {
                for (const std::string& actuatorReference : kvp.second.getActuatorReferences())
                {
                    publishActuatorStatus(kvp.first, actuatorReference);
                }
            }

            publish();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            connect();
        }
    });
}

void Wolk::disconnect()
{
    addToCommandBuffer([=]() -> void { m_connectivityService->disconnect(); });
}

void Wolk::publish()
{
    addToCommandBuffer([=]() -> void {
        m_dataService->publishActuatorStatuses();
        m_dataService->publishAlarms();
        m_dataService->publishSensorReadings();

        if (!m_persistence->isEmpty())
        {
            publish();
        }
    });
}

bool Wolk::addDevice(const Device& device)
{
    const std::string deviceKey = device.getKey();
    if (auto it = m_devices.find(deviceKey) != m_devices.end())
    {
        LOG(ERROR) << "Device with key '" << deviceKey << "' was already added";
        return false;
    }

    m_devices[deviceKey] = device;
    return true;
}

void Wolk::removeDevice(const std::string& deviceKey)
{
    auto it = m_devices.find(deviceKey);
    if (it != m_devices.end())
    {
        m_devices.erase(it);
    }
}

Wolk::Wolk()
{
    m_commandBuffer = std::unique_ptr<CommandBuffer>(new CommandBuffer());
}

void Wolk::addToCommandBuffer(std::function<void()> command)
{
    m_commandBuffer->pushCommand(std::make_shared<std::function<void()>>(command));
}

unsigned long long Wolk::currentRtc()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::seconds>(duration).count());
}

void Wolk::handleRegistrationResponse(std::shared_ptr<DeviceRegistrationResponse> response)
{
    if (m_registrationResponseHandler)
    {
        m_registrationResponseHandler(response->getReference(), response->getResult());
    }
}

Wolk::ConnectivityFacade::ConnectivityFacade(InboundMessageHandler& handler,
                                             std::function<void()> connectionLostHandler)
: m_messageHandler{handler}, m_connectionLostHandler{connectionLostHandler}
{
}

void Wolk::ConnectivityFacade::messageReceived(const std::string& channel, const std::string& message)
{
    m_messageHandler.messageReceived(channel, message);
}

void Wolk::ConnectivityFacade::connectionLost()
{
    m_connectionLostHandler();
}

std::vector<std::string> Wolk::ConnectivityFacade::getChannels() const
{
    return m_messageHandler.getChannels();
}
}    // namespace wolkabout
