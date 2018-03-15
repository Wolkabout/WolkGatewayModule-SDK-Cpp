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
#include "protocol/json/RegistrationProtocol.h"
#include "utilities/FileSystemUtils.h"
#include "utilities/json.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace wolkabout
{
using nlohmann::json;

DeviceConfiguration::DeviceConfiguration(std::string name, std::string key, std::string password,
                                         std::string platformMqttUri, unsigned interval, const DeviceManifest& manifest)
: m_name(std::move(name))
, m_key(std::move(key))
, m_password(std::move(password))
, m_platformMqttUri(std::move(platformMqttUri))
, m_interval(interval)
, m_manifest(std::move(manifest))
{
}

const std::string& DeviceConfiguration::getName() const
{
    return m_name;
}

const std::string& DeviceConfiguration::getKey() const
{
    return m_key;
}

const std::string& DeviceConfiguration::getPassword() const
{
    return m_password;
}

const std::string& DeviceConfiguration::getPlatformMqttUri() const
{
    return m_platformMqttUri;
}

unsigned DeviceConfiguration::getInterval() const
{
    return m_interval;
}

const DeviceManifest& DeviceConfiguration::getManifest() const
{
    return m_manifest;
}

wolkabout::DeviceConfiguration DeviceConfiguration::fromJson(const std::string& deviceConfigurationFile)
{
    if (!FileSystemUtils::isFilePresent(deviceConfigurationFile))
    {
        throw std::logic_error("Given gateway configuration file does not exist.");
    }

    std::string deviceConfigurationJson;
    if (!FileSystemUtils::readFileContent(deviceConfigurationFile, deviceConfigurationJson))
    {
        throw std::logic_error("Unable to read gateway configuration file.");
    }

    auto j = json::parse(deviceConfigurationJson);
    const auto name = j.at("name").get<std::string>();
    const auto key = j.at("key").get<std::string>();
    const auto password = j.at("password").get<std::string>();
    const auto platformMqttUri = j.at("host").get<std::string>();
    const auto interval = j.at("readingsInterval").get<unsigned>();
    const auto manifestText = j.at("manifest");

    DeviceManifest manifest;
    if (!RegistrationProtocol::makeManifest(manifestText, manifest))
    {
        throw std::logic_error("Unable to parse gateway manifest.");
    }

    return DeviceConfiguration(name, key, password, platformMqttUri, interval, manifest);
}
}    // namespace wolkabout
