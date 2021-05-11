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

#ifndef WOLK_H
#define WOLK_H

#include "ActuationHandlerPerDevice.h"
#include "ActuatorStatusProviderPerDevice.h"
#include "ConfigurationHandlerPerDevice.h"
#include "ConfigurationProviderPerDevice.h"
#include "WolkBuilder.h"
#include "core/model/ActuatorStatus.h"
#include "core/model/DeviceStatus.h"
#include "core/model/PlatformResult.h"
#include "core/utilities/CommandBuffer.h"
#include "model/Device.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace wolkabout
{
class ConnectivityService;
class DataService;
class DeviceStatusService;
class DeviceRegistrationService;
class FileDownloadService;
class FirmwareUpdateService;
class InboundGatewayMessageHandler;
class InboundMessageHandler;
class JsonDFUProtocol;

class Wolk
{
    friend class WolkBuilder;

public:
    ~Wolk();

    /**
     * @brief Initiates wolkabout::WolkBuilder that configures device to connect
     * to WolkAbout IoT Cloud
     * @param device wolkabout::Device
     * @return wolkabout::WolkBuilder instance
     */
    static WolkBuilder newBuilder();

    /**
     * @brief Publishes sensor reading to WolkAbout IoT Cloud<br>
     *        This method is thread safe, and can be called from multiple thread
     * simultaneously
     * @param deviceKey key of the device that holds the sensor
     * @param reference Sensor reference
     * @param value Sensor value<br>
     *              Supported types:<br>
     *               - bool<br>
     *               - float<br>
     *               - double<br>
     *               - signed int<br>
     *               - signed long int<br>
     *               - signed long long int<br>
     *               - unsigned int<br>
     *               - unsigned long int<br>
     *               - unsigned long long int<br>
     *               - string<br>
     *               - char*<br>
     *               - const char*<br>
     * @param rtc Reading POSIX time - Number of seconds since 01/01/1970<br>
     *            If omitted current POSIX time is adopted
     */
    template <typename T>
    void addSensorReading(const std::string& deviceKey, const std::string& reference, T value,
                          unsigned long long int rtc = 0);

    /**
     * @brief Publishes multi-value sensor reading to WolkAbout IoT Cloud<br>
     *        This method is thread safe, and can be called from multiple thread simultaneously
     * @param reference Sensor reference
     * @param values Multi-value sensor values<br>
     *              Supported types:<br>
     *               - bool<br>
     *               - float<br>
     *               - double<br>
     *               - signed int<br>
     *               - signed long int<br>
     *               - signed long long int<br>
     *               - unsigned int<br>
     *               - unsigned long int<br>
     *               - unsigned long long int<br>
     *               - string<br>
     *               - char*<br>
     *               - const char*<br>
     * @param rtc Reading POSIX time - Number of seconds since 01/01/1970<br>
     *            If omitted current POSIX time is adopted
     */
    template <typename T>
    void addSensorReading(const std::string& deviceKey, const std::string& reference, std::initializer_list<T> values,
                          unsigned long long int rtc = 0);

    /**
     * @brief Publishes multi-value sensor reading to WolkAbout IoT Cloud<br>
     *        This method is thread safe, and can be called from multiple thread simultaneously
     * @param reference Sensor reference
     * @param values Multi-value sensor values<br>
     *              Supported types:<br>
     *               - bool<br>
     *               - float<br>
     *               - double<br>
     *               - signed int<br>
     *               - signed long int<br>
     *               - signed long long int<br>
     *               - unsigned int<br>
     *               - unsigned long int<br>
     *               - unsigned long long int<br>
     *               - string<br>
     *               - char*<br>
     *               - const char*<br>
     * @param rtc Reading POSIX time - Number of seconds since 01/01/1970<br>
     *            If omitted current POSIX time is adopted
     */
    template <typename T>
    void addSensorReading(const std::string& deviceKey, const std::string& reference, const std::vector<T> values,
                          unsigned long long int rtc = 0);

    /**
     * @brief Publishes alarm to WolkAbout IoT Cloud<br>
     *        This method is thread safe, and can be called from multiple thread
     * simultaneously
     * @param deviceKey key of the device that holds the alarm
     * @param reference Alarm reference
     * @param active Is alarm active or not
     * @param rtc POSIX time at which event occurred - Number of seconds since
     * 01/01/1970<br> If omitted current POSIX time is adopted
     */
    void addAlarm(const std::string& deviceKey, const std::string& reference, bool active,
                  unsigned long long int rtc = 0);

    /**
     * @brief Invokes ActuatorStatusProvider callback to obtain actuator
     * status<br> This method is thread safe, and can be called from multiple
     * thread simultaneously
     * @param deviceKey key of the device that holds the actuator
     * @param Actuator reference
     */
    void publishActuatorStatus(const std::string& deviceKey, const std::string& reference);

    /**
     * @brief Accepts actuator value directly from the provider for device and reference.
     * @param deviceKey key of the device that holds the actuator
     * @param Actuator reference
     * @param value value
     */
    void publishActuatorStatus(const std::string& deviceKey, const std::string& reference, const std::string& value);

    /**
     * @brief addDeviceStatus
     * @param status
     */
    void addDeviceStatus(const std::string& deviceKey, DeviceStatus::Status status);

    /**
     * @brief Invokes ConfigurationProvider to obtain device configuration, and the publishes it.<br>
     *        This method is thread safe, and can be called from multiple thread simultaneously
     * * @param deviceKey key of the device that holds the configuration
     */
    void publishConfiguration(const std::string& deviceKey);

    /**
     * @brief Invokes ConfigurationProvider to obtain device configuration, and the publishes it.<br>
     *        This method is thread safe, and can be called from multiple thread simultaneously
     * * @param deviceKey key of the device that holds the configuration
     * * @param configurations values for each configuration reference of said device
     */
    void publishConfiguration(const std::string& deviceKey, std::vector<ConfigurationItem> configurations);

    /**
     * @brief connect Establishes connection with WolkAbout IoT platform
     */
    void connect(bool publishRightAway = true);

    /**
     * @brief disconnect Disconnects from WolkAbout IoT platform
     */
    void disconnect();

    /**
     * @brief publish Publishes data
     */
    void publish();

    /**
     * @brief publish Publishes data
     */
    void publish(const std::string& deviceKey);

    /**
     * @brief explicitly publishes device's status
     * @param deviceKey
     * @param status
     */
    void publishDeviceStatus(const std::string& deviceKey, DeviceStatus::Status status);

    /**
     * @brief addDevice Registers device on WolkAbout IoT platform
     * @param device
     */
    void addDevice(const Device& device);

    /**
     * @brief addAssetsToDevice Updates device with assets on WolkAbout IoT platform
     *
     */
    void addAssetsToDevice(std::string deviceKey, bool updateDefaultSemantics,
                           std::vector<ConfigurationTemplate> configurations = {},
                           std::vector<SensorTemplate> sensors = {}, std::vector<AlarmTemplate> alarms = {},
                           std::vector<ActuatorTemplate> actuators = {});

    /**
     * @brief removeDevice
     * @param deviceKey
     */
    void removeDevice(const std::string& deviceKey);

private:
    class ConnectivityFacade;

    Wolk();

    void addToCommandBuffer(std::function<void()> command);

    static unsigned long long int currentRtc();

    void handleActuatorSetCommand(const std::string& key, const std::string& reference, const std::string& value);
    void handleActuatorGetCommand(const std::string& key, const std::string& reference);
    void handleDeviceStatusRequest(const std::string& key);
    void handleConfigurationSetCommand(const std::string& key, const std::vector<ConfigurationItem>& configuration);
    void handleConfigurationGetCommand(const std::string& key);

    void registerDevices();
    void registerDevice(const Device& device);
    void updateDevice(std::string deviceKey, bool updateDefaultSemantics,
                      std::vector<ConfigurationTemplate> configurations = {}, std::vector<SensorTemplate> sensors = {},
                      std::vector<AlarmTemplate> alarms = {}, std::vector<ActuatorTemplate> actuators = {});

    void publishFirmwareVersion(const std::string& deviceKey);
    void publishFirmwareVersions();

    void publishDeviceStatuses();

    std::vector<std::string> getDeviceKeys();
    bool deviceExists(const std::string& deviceKey);
    bool sensorDefinedForDevice(const std::string& deviceKey, const std::string& reference);
    std::vector<std::string> getActuatorReferences(const std::string& deviceKey);
    bool alarmDefinedForDevice(const std::string& deviceKey, const std::string& reference);
    bool actuatorDefinedForDevice(const std::string& deviceKey, const std::string& reference);
    bool configurationItemDefinedForDevice(const std::string& deviceKey, const std::string& reference);

    bool validateAssetsToUpdate(const Device& device, const std::vector<ConfigurationTemplate>& configurations,
                                const std::vector<SensorTemplate>& sensors, const std::vector<AlarmTemplate>& alarms,
                                const std::vector<ActuatorTemplate>& actuators) const;
    void storeAssetsToDevice(Device& device, const std::vector<ConfigurationTemplate>& configurations,
                             const std::vector<SensorTemplate>& sensors, const std::vector<AlarmTemplate>& alarms,
                             const std::vector<ActuatorTemplate>& actuators);

    void handleRegistrationResponse(const std::string& deviceKey, PlatformResult::Code result);
    void handleUpdateResponse(const std::string& deviceKey, PlatformResult::Code result);

    std::unique_ptr<ConnectivityService> m_connectivityService;

    std::function<void(const std::string&, PlatformResult::Code)> m_registrationResponseHandler;

    std::unique_ptr<DataProtocol> m_dataProtocol;
    std::unique_ptr<StatusProtocol> m_statusProtocol;
    std::unique_ptr<RegistrationProtocol> m_registrationProtocol;
    std::unique_ptr<JsonDFUProtocol> m_firmwareUpdateProtocol;

    std::unique_ptr<Persistence> m_persistence;

    std::unique_ptr<InboundGatewayMessageHandler> m_inboundMessageHandler;

    std::shared_ptr<ConnectivityFacade> m_connectivityManager;

    std::function<void(const std::string&, const std::string&, const std::string&)> m_actuationHandlerLambda;
    std::shared_ptr<ActuationHandlerPerDevice> m_actuationHandler;

    std::function<ActuatorStatus(const std::string&, const std::string&)> m_actuatorStatusProviderLambda;
    std::shared_ptr<ActuatorStatusProviderPerDevice> m_actuatorStatusProvider;

    std::function<DeviceStatus::Status(const std::string&)> m_deviceStatusProviderLambda;
    std::shared_ptr<DeviceStatusProvider> m_deviceStatusProvider;

    std::function<void(const std::string&, const std::vector<ConfigurationItem>& configuration)>
      m_configurationHandlerLambda;
    std::shared_ptr<ConfigurationHandlerPerDevice> m_configurationHandler;

    std::function<std::vector<ConfigurationItem>(const std::string&)> m_configurationProviderLambda;
    std::shared_ptr<ConfigurationProviderPerDevice> m_configurationProvider;

    std::shared_ptr<DataService> m_dataService;
    std::shared_ptr<DeviceStatusService> m_deviceStatusService;
    std::shared_ptr<DeviceRegistrationService> m_deviceRegistrationService;
    std::shared_ptr<FirmwareUpdateService> m_firmwareUpdateService;

    std::map<std::string, Device> m_devices;

    std::atomic_bool m_connected;

    std::unique_ptr<CommandBuffer> m_commandBuffer;

    class ConnectivityFacade : public ConnectivityServiceListener
    {
    public:
        ConnectivityFacade(InboundMessageHandler& handler, std::function<void()> connectionLostHandler);

        void messageReceived(const std::string& channel, const std::string& message) override;
        void connectionLost() override;
        std::vector<std::string> getChannels() const override;

    private:
        InboundMessageHandler& m_messageHandler;
        std::function<void()> m_connectionLostHandler;
    };
};
}    // namespace wolkabout

#endif
