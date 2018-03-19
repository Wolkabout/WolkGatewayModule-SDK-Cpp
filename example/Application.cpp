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
#include "service/FirmwareInstaller.h"
#include "utilities/ConsoleLogger.h"

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <thread>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        LOG(ERROR) << "WolkGatewayModule Application: Usage -  " << argv[0] << " [configurationFilePath]";
        return -1;
    }

    auto logger = std::unique_ptr<wolkabout::ConsoleLogger>(new wolkabout::ConsoleLogger());
    logger->setLogLevel(wolkabout::LogLevel::DEBUG);
    wolkabout::Logger::setInstance(std::move(logger));

    wolkabout::DeviceConfiguration configuration;
    try
    {
        configuration = wolkabout::DeviceConfiguration::fromJson(argv[1]);
    }
    catch (std::logic_error& e)
    {
        LOG(ERROR) << "WolkGatewayModule Application: Unable to parse configuration file. Reason: " << e.what();
        return -1;
    }

    static bool switchValue = false;
    static int sliderValue = 0;

    wolkabout::SensorManifest temperatureSensor{"Temperature",
                                                "T",
                                                "Temperature sensor",
                                                "℃",
                                                "TEMPERATURE",
                                                wolkabout::SensorManifest::DataType::NUMERIC,
                                                1,
                                                -273.15,
                                                100000000};
    wolkabout::SensorManifest pressureSensor{
      "Pressure", "P", "Pressure sensor", "mb", "PRESSURE", wolkabout::SensorManifest::DataType::NUMERIC, 1, 0, 1100};
    wolkabout::SensorManifest humiditySensor{
      "Humidity", "H", "Humidity sensor", "%", "HUMIDITY", wolkabout::SensorManifest::DataType::NUMERIC, 1, 0, 100};

    wolkabout::ActuatorManifest switchActuator{
      "Switch", "SW", "Switch actuator", "", "SW", wolkabout::ActuatorManifest::DataType::BOOLEAN, 1, 0, 1};
    wolkabout::ActuatorManifest sliderActuator{
      "Slider", "SL", "Slider actuator", "", "SL", wolkabout::ActuatorManifest::DataType::NUMERIC, 1, 0, 115};

    wolkabout::AlarmManifest highHumidityAlarm{"High Humidity", wolkabout::AlarmManifest::AlarmSeverity::ALERT, "HH",
                                               "High Humidity", ""};

    wolkabout::DeviceManifest deviceManifest1{"DEVICE_MANIFEST_NAME_1",
                                              "DEVICE_MANIFEST_DESCRIPTION_1",
                                              "JsonProtocol",
                                              "",
                                              {},
                                              {temperatureSensor, pressureSensor},
                                              {},
                                              {switchActuator}};
    wolkabout::Device device1{"DEVICE_NAME_1", "DEVICE_KEY_1", deviceManifest1};

    wolkabout::DeviceManifest deviceManifest2{"DEVICE_MANIFEST_NAME_2",
                                              "DEVICE_MANIFEST_DESCRIPTION_2",
                                              "JsonProtocol",
                                              "",
                                              {},
                                              {humiditySensor},
                                              {highHumidityAlarm},
                                              {sliderActuator}};
    wolkabout::Device device2{"DEVICE_NAME_2", "DEVICE_KEY_2", deviceManifest2};

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
        .host(configuration.getLocalMqttUri())
        .build();

    wolk->connect();

    wolk->addDevice(device1);
    wolk->addDevice(device2);

    wolk->addSensorReading("DEVICE_KEY_1", "P", 1024);
    wolk->addSensorReading("DEVICE_KEY_1", "T", 25.6);

    wolk->addSensorReading("DEVICE_KEY_2", "H", 52);
    wolk->addAlarm("DEVICE_KEY_2", "HH", "High Humidity");

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
