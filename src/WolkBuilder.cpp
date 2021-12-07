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

#include "ActuationHandlerPerDevice.h"
#include "ActuatorStatusProviderPerDevice.h"
#include "Wolk.h"
#include "core/InboundMessageHandler.h"
#include "core/connectivity/ConnectivityService.h"
#include "core/connectivity/mqtt/MqttConnectivityService.h"
#include "core/connectivity/mqtt/PahoMqttClient.h"
#include "core/persistence/InMemoryPersistence.h"
#include "core/persistence/Persistence.h"
#include "core/protocol/json/JsonDFUProtocol.h"
#include "core/protocol/json/JsonProtocol.h"
#include "core/protocol/json/JsonRegistrationProtocol.h"
#include "core/protocol/json/JsonStatusProtocol.h"
#include "model/Device.h"
#include "service/DataService.h"
#include "service/DeviceRegistrationService.h"
#include "service/DeviceStatusService.h"
#include "service/FirmwareUpdateService.h"
#include "protocol/json/JsonPlatformStatusProtocol.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>

namespace wolkabout
{
WolkBuilder& WolkBuilder::host(const std::string& host)
{
    m_host = host;
    return *this;
}

WolkBuilder& WolkBuilder::actuationHandler(
  std::function<void(const std::string&, const std::string&, const std::string&)> actuationHandler)
{
    m_actuationHandlerLambda = std::move(actuationHandler);
    m_actuationHandler.reset();
    return *this;
}

WolkBuilder& WolkBuilder::actuationHandler(std::shared_ptr<ActuationHandlerPerDevice> actuationHandler)
{
    m_actuationHandler = std::move(actuationHandler);
    m_actuationHandlerLambda = nullptr;
    return *this;
}

WolkBuilder& WolkBuilder::actuatorStatusProvider(
  std::function<ActuatorStatus(const std::string&, const std::string&)> actuatorStatusProvider)
{
    m_actuatorStatusProviderLambda = std::move(actuatorStatusProvider);
    m_actuatorStatusProvider.reset();
    return *this;
}

WolkBuilder& WolkBuilder::actuatorStatusProvider(
  std::shared_ptr<ActuatorStatusProviderPerDevice> actuatorStatusProvider)
{
    m_actuatorStatusProvider = std::move(actuatorStatusProvider);
    m_actuatorStatusProviderLambda = nullptr;
    return *this;
}

WolkBuilder& WolkBuilder::configurationHandler(
  std::function<void(const std::string&, const std::vector<ConfigurationItem>& configuration)> configurationHandler)
{
    m_configurationHandlerLambda = std::move(configurationHandler);
    m_configurationHandler.reset();
    return *this;
}

WolkBuilder& WolkBuilder::configurationHandler(std::shared_ptr<ConfigurationHandlerPerDevice> configurationHandler)
{
    m_configurationHandler = std::move(configurationHandler);
    m_configurationHandlerLambda = nullptr;
    return *this;
}

WolkBuilder& WolkBuilder::configurationProvider(
  std::function<std::vector<ConfigurationItem>(const std::string&)> configurationProvider)
{
    m_configurationProviderLambda = std::move(configurationProvider);
    m_configurationProvider.reset();
    return *this;
}

WolkBuilder& WolkBuilder::configurationProvider(std::shared_ptr<ConfigurationProviderPerDevice> configurationProvider)
{
    m_configurationProvider = std::move(configurationProvider);
    m_configurationProviderLambda = nullptr;
    return *this;
}

WolkBuilder& WolkBuilder::deviceStatusProvider(
  std::function<DeviceStatus::Status(const std::string& deviceKey)> deviceStatusProvider)
{
    m_deviceStatusProviderLambda = std::move(deviceStatusProvider);
    m_deviceStatusProvider.reset();
    return *this;
}

WolkBuilder& WolkBuilder::deviceStatusProvider(std::shared_ptr<DeviceStatusProvider> deviceStatusProvider)
{
    m_deviceStatusProvider = std::move(deviceStatusProvider);
    m_deviceStatusProviderLambda = nullptr;
    return *this;
}

WolkBuilder& WolkBuilder::withPersistence(std::unique_ptr<Persistence> persistence)
{
    m_persistence.reset(persistence.release());
    return *this;
}

WolkBuilder& WolkBuilder::withFirmwareUpdate(std::shared_ptr<FirmwareInstaller> installer,
                                             std::shared_ptr<FirmwareVersionProvider> provider)
{
    m_firmwareInstaller = std::move(installer);
    m_firmwareVersionProvider = std::move(provider);
    return *this;
}

WolkBuilder& WolkBuilder::withRegistrationResponseHandler(
  std::function<void(const std::string&, PlatformResult::Code)> registrationResponseHandler)
{
    m_registrationResponseHandler = std::move(registrationResponseHandler);
    return *this;
}

WolkBuilder& WolkBuilder::withPlatformStatusListener(std::shared_ptr<PlatformStatusListener> listener)
{
    m_platformStatusListener = std::move(listener);
    m_platformStatusCallback = {};
    return *this;
}

WolkBuilder& WolkBuilder::withPlatformStatusListener(PlatformStatusCallback callback)
{
    m_platformStatusCallback = std::move(callback);
    m_platformStatusListener = {};
    return *this;
}

std::unique_ptr<Wolk> WolkBuilder::build()
{
    if (!m_actuationHandlerLambda && !m_actuationHandler)
    {
        throw std::logic_error("Actuation handler not set.");
    }

    if (!m_actuatorStatusProviderLambda && !m_actuatorStatusProvider)
    {
        throw std::logic_error("Actuator status provider not set.");
    }

    if (!m_deviceStatusProviderLambda && !m_deviceStatusProvider)
    {
        throw std::logic_error("Device status provider not set.");
    }

    if ((m_configurationHandlerLambda == nullptr && m_configurationProviderLambda != nullptr) ||
        (m_configurationHandlerLambda != nullptr && m_configurationProviderLambda == nullptr))
    {
        throw std::logic_error("Both ConfigurationProvider and ConfigurationHandler must be set.");
    }

    if ((m_configurationHandler && !m_configurationProvider) || (!m_configurationHandler && m_configurationProvider))
    {
        throw std::logic_error("Both ConfigurationProvider and ConfigurationHandler must be set.");
    }

    if (!m_firmwareInstaller != !m_firmwareVersionProvider)
    {
        throw std::logic_error("Both FirmwareInstaller and FirmwareVersionProvider must be set.");
    }

    auto wolk = std::unique_ptr<Wolk>(new Wolk());

    wolk->m_dataProtocol.reset(new JsonProtocol());
    wolk->m_statusProtocol.reset(new JsonStatusProtocol(false));
    wolk->m_registrationProtocol.reset(new JsonRegistrationProtocol(false));
    wolk->m_firmwareUpdateProtocol.reset(new JsonDFUProtocol());

    wolk->m_persistence.reset(m_persistence.release());

    wolk->m_connectivityService.reset(new MqttConnectivityService(std::make_shared<PahoMqttClient>(), "", "", m_host));

    wolk->m_inboundMessageHandler.reset(new InboundGatewayMessageHandler());

    wolk->m_connectivityManager = std::make_shared<Wolk::ConnectivityFacade>(*wolk->m_inboundMessageHandler,
                                                                             [&]
                                                                             {
                                                                                 wolk->m_connected = false;
                                                                                 wolk->connect();
                                                                             });

    wolk->m_connectivityService->setListener(wolk->m_connectivityManager);

    wolk->m_actuationHandler = m_actuationHandler;
    wolk->m_actuationHandlerLambda = m_actuationHandlerLambda;

    wolk->m_actuatorStatusProvider = m_actuatorStatusProvider;
    wolk->m_actuatorStatusProviderLambda = m_actuatorStatusProviderLambda;

    wolk->m_configurationHandler = m_configurationHandler;
    wolk->m_configurationHandlerLambda = m_configurationHandlerLambda;

    wolk->m_configurationProvider = m_configurationProvider;
    wolk->m_configurationProviderLambda = m_configurationProviderLambda;

    wolk->m_deviceStatusProvider = m_deviceStatusProvider;
    wolk->m_deviceStatusProviderLambda = m_deviceStatusProviderLambda;

    if (m_registrationResponseHandler)
        wolk->m_registrationResponseHandler = m_registrationResponseHandler;

    const auto rawPointer = wolk.get();

    wolk->m_dataService = std::make_shared<DataService>(
      *wolk->m_dataProtocol, *wolk->m_persistence, *wolk->m_connectivityService,
      [rawPointer](const std::string& key, const std::string& reference, const std::string& value)
      { rawPointer->handleActuatorSetCommand(key, reference, value); },
      [rawPointer](const std::string& key, const std::string& reference)
      { rawPointer->handleActuatorGetCommand(key, reference); },
      [rawPointer](const std::string& key, const std::vector<ConfigurationItem>& configuration)
      { rawPointer->handleConfigurationSetCommand(key, configuration); },
      [rawPointer](const std::string& key) { rawPointer->handleConfigurationGetCommand(key); });

    wolk->m_deviceStatusService = std::make_shared<DeviceStatusService>(
      *wolk->m_statusProtocol, *wolk->m_connectivityService,
      [rawPointer](const std::string& key) { rawPointer->handleDeviceStatusRequest(key); });

    wolk->m_deviceRegistrationService = std::make_shared<DeviceRegistrationService>(
      *wolk->m_registrationProtocol, *wolk->m_connectivityService,
      [rawPointer](const std::string& key, PlatformResult::Code result)
      { rawPointer->handleRegistrationResponse(key, result); },
      [rawPointer](const std::string& key, PlatformResult::Code result)
      { rawPointer->handleUpdateResponse(key, result); });

    // Firmware update service
    if (m_firmwareInstaller != nullptr)
    {
        wolk->m_firmwareUpdateService =
          std::make_shared<FirmwareUpdateService>(*wolk->m_firmwareUpdateProtocol, m_firmwareInstaller,
                                                  m_firmwareVersionProvider, *wolk->m_connectivityService);

        wolk->m_inboundMessageHandler->addListener(wolk->m_firmwareUpdateService);
    }

    // Check if any of the platform status parameters are set
    if (m_platformStatusListener != nullptr)
    {
        wolk->m_platformStatusProtocol = std::unique_ptr<JsonPlatformStatusProtocol>{new JsonPlatformStatusProtocol};
        wolk->m_platformStatusService =
          std::make_shared<PlatformStatusService>(*wolk->m_platformStatusProtocol, std::move(m_platformStatusListener));
        wolk->m_inboundMessageHandler->addListener(wolk->m_platformStatusService);
    }
    else if (m_platformStatusCallback)
    {
        wolk->m_platformStatusProtocol = std::unique_ptr<JsonPlatformStatusProtocol>{new JsonPlatformStatusProtocol};
        wolk->m_platformStatusService =
          std::make_shared<PlatformStatusService>(*wolk->m_platformStatusProtocol, std::move(m_platformStatusCallback));
        wolk->m_inboundMessageHandler->addListener(wolk->m_platformStatusService);
    }

    wolk->m_inboundMessageHandler->addListener(wolk->m_dataService);
    wolk->m_inboundMessageHandler->addListener(wolk->m_deviceStatusService);
    wolk->m_inboundMessageHandler->addListener(wolk->m_deviceRegistrationService);

    wolk->m_connectivityService->setListener(wolk->m_connectivityManager);

    return wolk;
}

wolkabout::WolkBuilder::operator std::unique_ptr<Wolk>()
{
    return build();
}

WolkBuilder::WolkBuilder()
: m_host{MESSAGE_BUS_HOST}
, m_actuationHandlerLambda{nullptr}
, m_actuationHandler{nullptr}
, m_actuatorStatusProviderLambda{nullptr}
, m_actuatorStatusProvider{nullptr}
, m_configurationHandlerLambda{nullptr}
, m_configurationHandler{nullptr}
, m_configurationProviderLambda{nullptr}
, m_configurationProvider{nullptr}
, m_deviceStatusProviderLambda{nullptr}
, m_deviceStatusProvider{nullptr}
, m_persistence{new InMemoryPersistence()}
, m_firmwareInstaller{nullptr}
, m_firmwareVersionProvider{nullptr}
{
}
}    // namespace wolkabout
