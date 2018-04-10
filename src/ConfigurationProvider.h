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
#ifndef CONFIGURATIONPROVIDER_H
#define CONFIGURATIONPROVIDER_H

#include <map>
#include <string>

namespace wolkabout
{
class ConfigurationProvider
{
public:
    virtual ~ConfigurationProvider() = default;

    std::map<std::string, std::string> operator()(const std::string& deviceKey) { return getConfiguration(deviceKey); }

private:
    /**
     * @brief Device configuration provider callback
     *        Reads device configuration and returns it as Map<String, String>
     *        with device configuration reference as map key,
     *        and device configuration value as map value.<br>
     *
     *        Must be implemented as non blocking<br>
     *        Must be implemented as thread safe
     * @return Device configuration as std::map<std::string, std::string>
     */
    virtual std::map<std::string, std::string> getConfiguration(const std::string& deviceKey) = 0;
};
}    // namespace wolkabout

#endif    // CONFIGURATIONPROVIDER_H
