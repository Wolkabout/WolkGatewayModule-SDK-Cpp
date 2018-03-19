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

class ActuatorHandler {
public:
  virtual ~ActuatorHandler() = default;
  virtual std::string getValue() = 0;
  virtual void setValue(std::string value) = 0;
};

template <class T> class ActuatorTemplateHandler : public ActuatorHandler {
public:
  void setValue(std::string value) override {
    try {
      m_value = std::stod(value);
    } catch (...) {
    }
  }

  std::string getValue() override { return std::to_string(m_value); }

private:
  T m_value;
};

template <> class ActuatorTemplateHandler<bool> : public ActuatorHandler {
public:
  void setValue(std::string value) override { m_value = value == "true"; }

  std::string getValue() override { return m_value ? "true" : "false"; }

private:
  bool m_value;
};

template <>
class ActuatorTemplateHandler<std::string> : public ActuatorHandler {
public:
  void setValue(std::string value) override { m_value = value; }

  std::string getValue() override { return m_value; }

private:
  std::string m_value;
};

int main(int /* argc */, char ** /* argv */) {
  auto logger =
      std::unique_ptr<wolkabout::ConsoleLogger>(new wolkabout::ConsoleLogger());
  logger->setLogLevel(wolkabout::LogLevel::DEBUG);
  wolkabout::Logger::setInstance(std::move(logger));

  wolkabout::DeviceConfiguration configuration;
  try {
    configuration = wolkabout::DeviceConfiguration::fromJson("config");
  } catch (...) {
    LOG(ERROR) << "Failed to load configuration";
    return -1;
  }

  class CustomFirmwareInstaller : public wolkabout::FirmwareInstaller {
  public:
    bool install(const std::string &firmwareFile) override {
      // Mock install
      std::cout << "Updating firmware with file " << firmwareFile << std::endl;

      // Optionally delete 'firmwareFile
      return true;
    }
  };

  auto installer = std::make_shared<CustomFirmwareInstaller>();

  std::map<std::string, std::shared_ptr<ActuatorHandler>> handlers;

  std::unique_ptr<wolkabout::Wolk> wolk =
      wolkabout::Wolk::newBuilder()
          .actuationHandler([&](const std::string &deviceKey,
                                const std::string &reference,
                                const std::string &value) -> void {
            std::cout << "Actuation request received - Reference: " << reference
                      << " value: " << value << std::endl;

            //		auto it = handlers.find(reference);
            //		if(it != handlers.end())
            //		{
            //			it->second->setValue(value);
            //		}
          })
          .actuatorStatusProvider(
              [&](const std::string &deviceKey,
                  const std::string &reference) -> wolkabout::ActuatorStatus {
                //		auto it = handlers.find(reference);
                //		if(it != handlers.end())
                //		{
                //			return
                // wolkabout::ActuatorStatus(it->second->getValue(),
                // wolkabout::ActuatorStatus::State::READY);
                //		}

                return wolkabout::ActuatorStatus(
                    "", wolkabout::ActuatorStatus::State::READY);
              })
          .host(configuration.getLocalMqttUri())
          .build();

  wolk->connect();

  //	std::random_device rd;
  //	std::mt19937 mt(rd());

  //    const unsigned interval = configuration.getInterval();

  //    while(true)
  //    {
  //		for(const auto& sensor :
  // configuration.getManifest().getSensors())
  //		{
  //			std::uniform_real_distribution<double>
  // dist(sensor.getMinimum(),
  // sensor.getMaximum()); 			double rand_num = dist(mt);

  //			wolk->addSensorReading(sensor.getReference(), rand_num);
  //		}

  //		wolk->publish();

  //        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
  //    }

  return 0;
}
