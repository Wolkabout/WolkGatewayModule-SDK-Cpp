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

#include "Configuration.h"
#include "Wolk.h"
#include "model/DeviceTemplate.h"
#include "utilities/ConsoleLogger.h"

#include "model/SensorTemplate.h"

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>

int main(int argc, char** argv)
{
    auto logger = std::unique_ptr<wolkabout::ConsoleLogger>(new wolkabout::ConsoleLogger());
    logger->setLogLevel(wolkabout::LogLevel::DEBUG);
    wolkabout::Logger::setInstance(std::move(logger));

    if (argc < 2)
    {
        LOG(ERROR) << "WolkGatewayModule Application: Usage -  " << argv[0] << " [configurationFilePath]";
        return -1;
    }

    wolkabout::DeviceConfiguration appConfiguration = [&] {
        try
        {
            return wolkabout::DeviceConfiguration::fromJson(argv[1]);
        }
        catch (std::logic_error& e)
        {
            LOG(ERROR) << "WolkGatewayModule Application: Unable to parse configuration file. Reason: " << e.what();
            std::exit(-1);
        }
    }();

    static bool switchValue = false;
    static int sliderValue = 0;
    static std::vector<wolkabout::ConfigurationItem> device1configuration = {{{"value1"}, "KEY_1"},
                                                                             {{"50", "32", "-2"}, "KEY_2"}};
    static std::vector<wolkabout::ConfigurationItem> device2configuration = {{{"value3"}, "KEY_3"}};

    wolkabout::SensorTemplate temperatureSensor{"Temperature",
                                                "T",
                                                wolkabout::ReadingType::Name::TEMPERATURE,
                                                wolkabout::ReadingType::MeasurmentUnit::CELSIUS,
                                                "",
                                                -273.15,
                                                100000000};
    wolkabout::SensorTemplate pressureSensor{
      "Pressure", "P", wolkabout::ReadingType::Name::PRESSURE, wolkabout::ReadingType::MeasurmentUnit::MILLIBAR, "",
      0,          1100};
    wolkabout::SensorTemplate humiditySensor{"Humidity",
                                             "H",
                                             wolkabout::ReadingType::Name::HUMIDITY,
                                             wolkabout::ReadingType::MeasurmentUnit::HUMIDITY_PERCENT,
                                             "",
                                             0,
                                             100};

    wolkabout::SensorTemplate accelerationSensor{"Acceleration",
                                                 "ACCELEROMETER_REF",
                                                 wolkabout::ReadingType::Name::ACCELEROMETER,
                                                 wolkabout::ReadingType::MeasurmentUnit::METRES_PER_SQUARE_SECOND,
                                                 "",
                                                 0,
                                                 20000};

    wolkabout::ActuatorTemplate switchActuator{"Switch", "SW", wolkabout::DataType::BOOLEAN, "Light switch"};
    wolkabout::ActuatorTemplate sliderActuator{"Slider", "SL", wolkabout::DataType::NUMERIC, "Light dimmer", 0, 115};
    wolkabout::ActuatorTemplate textActuator{"Message", "MSG", wolkabout::DataType::STRING, "Text"};

    wolkabout::AlarmTemplate highHumidityAlarm{"High Humidity", "HH", ""};

    wolkabout::ConfigurationTemplate configurationItem1{"Item1", "KEY_1", wolkabout::DataType::STRING, "", "value1"};

    wolkabout::ConfigurationTemplate configurationItem2{
      "Item2", "KEY_2", wolkabout::DataType::NUMERIC, "", "5", {"x", "y", "z"}, 0, 100};

    wolkabout::ConfigurationTemplate configurationItem3{"Item3", "KEY_3", wolkabout::DataType::BOOLEAN, "", "false"};

    wolkabout::DeviceTemplate deviceTemplate1{{configurationItem1, configurationItem2},
                                              {temperatureSensor, humiditySensor},
                                              {},
                                              {switchActuator, textActuator},
                                              "DFU"};
    wolkabout::Device device1{"DEVICE_NAME_1", "DEVICE_KEY_1", deviceTemplate1};

    wolkabout::DeviceTemplate deviceTemplate2{
      {configurationItem3}, {pressureSensor, accelerationSensor}, {highHumidityAlarm}, {sliderActuator}, "DFU"};
    wolkabout::Device device2{"DEVICE_NAME_2", "DEVICE_KEY_2", deviceTemplate2};

    static int device1firmwareVersion = 1;
    static int device2firmwareVersion = 1;

    class FirmwareInstallerImpl : public wolkabout::FirmwareInstaller
    {
    public:
        void install(const std::string& deviceKey, const std::string& firmwareFile,
                     std::function<void(const std::string& deviceKey)> onSuccess,
                     std::function<void(const std::string& deviceKey)> onFail) override
        {
            LOG(INFO) << "Install firmware: " << firmwareFile << ", for device " << deviceKey;
            if (deviceKey == "DEVICE_KEY_1")
            {
                ++device1firmwareVersion;
                onSuccess(deviceKey);
            }
            else
            {
                onFail(deviceKey);
            }
        }

        bool abort(const std::string& deviceKey) override
        {
            LOG(INFO) << "Abort firmware installation for device " << deviceKey;
            return false;
        }
    };

    class FirmwareVersionProviderImpl : public wolkabout::FirmwareVersionProvider
    {
    public:
        std::string getFirmwareVersion(const std::string& deviceKey)
        {
            if (deviceKey == "DEVICE_KEY_1")
            {
                return std::to_string(device1firmwareVersion) + ".0.0";
            }
            else if (deviceKey == "DEVICE_KEY_2")
            {
                return std::to_string(device2firmwareVersion) + ".0.0";
            }
            return "";
        }
    };

    auto installer = std::make_shared<FirmwareInstallerImpl>();
    auto provider = std::make_shared<FirmwareVersionProviderImpl>();

    std::unique_ptr<wolkabout::Wolk> wolk =
      wolkabout::Wolk::newBuilder()
        .actuationHandler(
          [&](const std::string& deviceKey, const std::string& reference, const std::string& value) -> void {
              std::cout << "Actuation request received - Reference: " << reference << " value: " << value << std::endl;
              if (deviceKey == "DEVICE_KEY_1" && reference == "SW")
              {
                  switchValue = value == "true" ? true : false;
              }
              else if (deviceKey == "DEVICE_KEY_2" && reference == "SL")
              {
                  try
                  {
                      sliderValue = std::stoi(value);
                  }
                  catch (...)
                  {
                  }
              }
          })
        .actuatorStatusProvider([&](const std::string& deviceKey,
                                    const std::string& reference) -> wolkabout::ActuatorStatus {
            if (deviceKey == "DEVICE_KEY_1" && reference == "SW")
            {
                return wolkabout::ActuatorStatus(switchValue ? "true" : "false",
                                                 wolkabout::ActuatorStatus::State::READY);
            }
            else if (deviceKey == "DEVICE_KEY_2" && reference == "SL")
            {
                return wolkabout::ActuatorStatus(std::to_string(sliderValue), wolkabout::ActuatorStatus::State::READY);
            }

            return wolkabout::ActuatorStatus("", wolkabout::ActuatorStatus::State::READY);
        })
        .deviceStatusProvider([](const std::string& deviceKey) -> wolkabout::DeviceStatus::Status {
            if (deviceKey == "DEVICE_KEY_1")
            {
                return wolkabout::DeviceStatus::Status::CONNECTED;
            }
            else if (deviceKey == "DEVICE_KEY_2")
            {
                return wolkabout::DeviceStatus::Status::CONNECTED;
            }

            return wolkabout::DeviceStatus::Status::OFFLINE;
        })
        .configurationHandler(
          [&](const std::string& deviceKey, const std::vector<wolkabout::ConfigurationItem>& configuration) {
              if (deviceKey == "DEVICE_KEY_1")
              {
                  // device1configuration = configuration;
              }
              else if (deviceKey == "DEVICE_KEY_2")
              {
                  // device2configuration = configuration;
              }
          })
        .configurationProvider([&](const std::string& deviceKey) -> std::vector<wolkabout::ConfigurationItem> {
            if (deviceKey == "DEVICE_KEY_1")
            {
                return device1configuration;
            }
            else if (deviceKey == "DEVICE_KEY_2")
            {
                return device2configuration;
            }

            return {};
        })
        .withFirmwareUpdate(installer, provider)
        .host(appConfiguration.getLocalMqttUri())
        .build();

    wolk->addDevice(device1);
    wolk->addDevice(device2);

    wolk->connect();

    wolk->addSensorReading("DEVICE_KEY_1", "P", 1024);
    wolk->addSensorReading("DEVICE_KEY_1", "T", 25.6);

    wolk->addSensorReading("DEVICE_KEY_2", "H", 52);
    wolk->addAlarm("DEVICE_KEY_2", "HH", "High Humidity");

    wolk->addSensorReading("DEVICE_KEY_2", "ACCELEROMETER_REF", {0, -5, 10});

    wolk->addDeviceStatus("DEVICE_KEY_1", wolkabout::DeviceStatus::Status::CONNECTED);
    wolk->addDeviceStatus("DEVICE_KEY_2", wolkabout::DeviceStatus::Status::CONNECTED);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
