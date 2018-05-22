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
#include "model/DeviceManifest.h"
#include "utilities/ConsoleLogger.h"

#include "model/SensorManifest.h"

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

    wolkabout::DeviceConfiguration appConfiguration;
    try
    {
        appConfiguration = wolkabout::DeviceConfiguration::fromJson(argv[1]);
    }
    catch (std::logic_error& e)
    {
        LOG(ERROR) << "WolkGatewayModule Application: Unable to parse configuration file. Reason: " << e.what();
        return -1;
    }

    static bool switchValue = false;
    static int sliderValue = 0;
    static std::vector<wolkabout::ConfigurationItem> device1configuration = {{{"value1"}, "KEY_1"},
                                                                             {{"50", "32", "-2"}, "KEY_2"}};
    static std::vector<wolkabout::ConfigurationItem> device2configuration = {{{"value3"}, "KEY_3"}};

    wolkabout::SensorManifest temperatureSensor{"Temperature",
                                                "T",
                                                wolkabout::ReadingType::Name::TEMPERATURE,
                                                wolkabout::ReadingType::MeasurmentUnit::CELSIUS,
                                                "",
                                                -273.15,
                                                100000000};
    wolkabout::SensorManifest pressureSensor{
      "Pressure", "P", wolkabout::ReadingType::Name::PRESSURE, wolkabout::ReadingType::MeasurmentUnit::MILLIBAR, "",
      0,          1100};
    wolkabout::SensorManifest humiditySensor{"Humidity",
                                             "H",
                                             wolkabout::ReadingType::Name::HUMIDITY,
                                             wolkabout::ReadingType::MeasurmentUnit::HUMIDITY_PERCENT,
                                             "",
                                             0,
                                             100};

    wolkabout::SensorManifest accelerationSensor{"Acceleration",
                                                 "ACCELEROMETER_REF",
                                                 wolkabout::ReadingType::Name::ACCELEROMETER,
                                                 wolkabout::ReadingType::MeasurmentUnit::METRES_PER_SQUARE_SECOND,
                                                 "",
                                                 0,
                                                 20000};

    wolkabout::ActuatorManifest switchActuator{"Switch", "SW", wolkabout::DataType::BOOLEAN, "Light switch"};
    wolkabout::ActuatorManifest sliderActuator{"Slider", "SL", wolkabout::DataType::NUMERIC, "Light dimmer", 0, 115};
    wolkabout::ActuatorManifest textActuator{"Message", "MSG", wolkabout::DataType::STRING, "Text"};

    wolkabout::AlarmManifest highHumidityAlarm{"High Humidity", wolkabout::AlarmManifest::AlarmSeverity::ALERT, "HH",
                                               "High Humidity", ""};

    wolkabout::ConfigurationManifest configurationItem1{"Item1", "KEY_1", wolkabout::DataType::STRING, "", "value1"};

    wolkabout::ConfigurationManifest configurationItem2{
      "Item2", "KEY_2", wolkabout::DataType::NUMERIC, "", "5", {"x", "y", "z"}, 0, 100};

    wolkabout::ConfigurationManifest configurationItem3{"Item3", "KEY_3", wolkabout::DataType::BOOLEAN, "", "false"};

    wolkabout::DeviceManifest deviceManifest1{
      "DEVICE_MANIFEST_NAME_1",
      "DEVICE_MANIFEST_DESCRIPTION_1",
      "JsonProtocol",
      "",
      {configurationItem1, configurationItem2, configurationItem3},
      {temperatureSensor, accelerationSensor},    //, humiditySensor, pressureSensor},
      {highHumidityAlarm},
      {sliderActuator}};
    wolkabout::Device device1{"DEVICE_NAME_1", "m6939iarfbae6gdn", deviceManifest1};

    wolkabout::DeviceManifest deviceManifest2{
      "DEVICE_MANIFEST_NAME_2", "DEVICE_MANIFEST_DESCRIPTION_2", "JsonProtocol", "", {}, {},    //accelerationSensor},
      {highHumidityAlarm}
      //{switchActuator, sliderActuator}
    };
    wolkabout::Device device2{"DEVICE_NAME_2", "gw_device_key_22-05_3", deviceManifest2};

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
        .deviceStatusProvider([](const std::string& deviceKey) -> wolkabout::DeviceStatus {
            if (deviceKey == "DEVICE_KEY_1")
            {
                return wolkabout::DeviceStatus::CONNECTED;
            }
            else if (deviceKey == "DEVICE_KEY_2")
            {
                return wolkabout::DeviceStatus::SLEEP;
            }

            return wolkabout::DeviceStatus::OFFLINE;
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

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
