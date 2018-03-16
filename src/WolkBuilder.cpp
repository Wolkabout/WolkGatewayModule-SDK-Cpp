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

#include "WolkBuilder.h"
#include "ActuationHandler.h"
#include "ActuatorStatusProvider.h"
#include "FileHandler.h"
#include "InboundMessageHandler.h"
#include "Wolk.h"
#include "connectivity/ConnectivityService.h"
#include "connectivity/mqtt/MqttConnectivityService.h"
#include "connectivity/mqtt/PahoMqttClient.h"
#include "model/Device.h"
#include "model/FirmwareUpdateCommand.h"
#include "persistence/Persistence.h"
#include "persistence/inmemory/InMemoryPersistence.h"
#include "protocol/json/JsonProtocol.h"
#include "protocol/json/JsonRegistrationProtocol.h"
#include "protocol/json/JsonStatusProtocol.h"
#include "service/DataService.h"
#include "service/DeviceRegistrationService.h"
#include "service/DeviceStatusService.h"
#include "service/FileDownloadService.h"
#include "service/FirmwareUpdateService.h"

#include <functional>
#include <stdexcept>
#include <string>

namespace wolkabout
{
WolkBuilder& WolkBuilder::host(const std::string& host)
{
    m_host = host;
    return *this;
}

WolkBuilder& WolkBuilder::actuationHandler(
  const std::function<void(const std::string&, const std::string&, const std::string&)>& actuationHandler)
{
    m_actuationHandlerLambda = actuationHandler;
    return *this;
}

WolkBuilder& WolkBuilder::actuationHandler(std::shared_ptr<ActuationHandler> actuationHandler)
{
    m_actuationHandler = actuationHandler;
    return *this;
}

WolkBuilder& WolkBuilder::actuatorStatusProvider(
  const std::function<ActuatorStatus(const std::string&, const std::string&)>& actuatorStatusProvider)
{
    m_actuatorStatusProviderLambda = actuatorStatusProvider;
    return *this;
}

WolkBuilder& WolkBuilder::actuatorStatusProvider(std::shared_ptr<ActuatorStatusProvider> actuatorStatusProvider)
{
    m_actuatorStatusProvider = actuatorStatusProvider;
    return *this;
}

WolkBuilder& WolkBuilder::registrationResponseHandler(
  const std::function<void(const std::string&, DeviceRegistrationResponse::Result)>& registrationResponseHandler)
{
    m_registrationResponseHandler = registrationResponseHandler;
    return *this;
}

WolkBuilder& WolkBuilder::withPersistence(std::shared_ptr<Persistence> persistence)
{
    m_persistence = persistence;
    return *this;
}

std::unique_ptr<Wolk> WolkBuilder::build() const
{
    if (!m_actuationHandlerLambda || m_actuationHandler)
    {
        throw std::logic_error("Actuation handler not set.");
    }

    if (!m_actuatorStatusProviderLambda || m_actuatorStatusProvider)
    {
        throw std::logic_error("Actuator status provider not set.");
    }

    auto wolk = std::unique_ptr<Wolk>(new Wolk());

    wolk->m_dataProtocol.reset(new JsonProtocol());

    wolk->m_statusProtocol.reset(new JsonStatusProtocol());
    wolk->m_registrationProtocol.reset(new JsonRegistrationProtocol());

    wolk->m_persistence = m_persistence;

    wolk->m_connectivityService.reset(new MqttConnectivityService(std::make_shared<PahoMqttClient>(), "", "", m_host));

    wolk->m_inboundMessageHandler.reset(new InboundGatewayMessageHandler());

    wolk->m_connectivityManager = std::make_shared<Wolk::ConnectivityFacade>(*wolk->m_inboundMessageHandler, [&] {
        // wolk->m_platformPublisher->disconnected();
        wolk->connect();
    });

    wolk->m_connectivityService->setListener(wolk->m_connectivityManager);

    wolk->m_actuationHandler = std::make_shared<decltype(m_actuationHandlerLambda)>(m_actuationHandlerLambda);
    // TODO !!!
    wolk->m_actuatorStatusProvider =
      std::make_shared<decltype(m_actuatorStatusProviderLambda)>(m_actuatorStatusProviderLambda);

    wolk->m_dataService = std::make_shared<DataService>(
      *wolk->m_dataProtocol, *wolk->m_persistence, *wolk->m_connectivityService,
      [&](const std::string& key, const std::string& reference, const std::string& value) {
          wolk->handleActuatorSetCommand(key, reference, value);
      },
      [&](const std::string& key, const std::string& reference) { wolk->handleActuatorGetCommand(key, reference); });

    wolk->m_deviceStatusService =
      std::make_shared<DeviceStatusService>(*wolk->m_statusProtocol, *wolk->m_connectivityService,
                                            [&](const std::string& key) { wolk->handleDeviceStatusRequest(key); });

    wolk->m_deviceRegistrationService = std::make_shared<DeviceRegistrationService>(
      *wolk->m_registrationProtocol, *wolk->m_connectivityService,
      [&](const std::string& key, DeviceRegistrationResponse::Result result) {
          wolk->handleRegistrationResponse(key, result);
      });

    wolk->m_inboundMessageHandler->addListener(wolk->m_dataService);
    wolk->m_inboundMessageHandler->addListener(wolk->m_deviceStatusService);
    wolk->m_inboundMessageHandler->addListener(wolk->m_deviceRegistrationService);

    wolk->m_connectivityService->setListener(wolk->m_connectivityManager);

    return wolk;
}

wolkabout::WolkBuilder::operator std::unique_ptr<Wolk>() const
{
    return build();
}

WolkBuilder::WolkBuilder() : m_host{MESSAGE_BUS_HOST}, m_persistence{new InMemoryPersistence()} {}
}    // namespace wolkabout
