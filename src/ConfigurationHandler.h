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
#ifndef CONFIGURATIONHANDLER_H
#define CONFIGURATIONHANDLER_H

#include <map>
#include <string>

namespace wolkabout
{
class ConfigurationHandler
{
public:
    virtual ~ConfigurationHandler() = default;

    void operator()(const std::string& deviceKey, const std::map<std::string, std::string>& configuration)
    {
        handleConfiguration(deviceKey, configuration);
    }

private:
    /**
     * @brief When new set of device configuration values is given from platform, it will be delivered to this method.
     *        This method should update device configuration with received configuration values.<br>

     *        Must be implemented as non blocking<br>
     *        Must be implemented as thread safe
     * @param configuration std::map<std::string, std::string> with device configuration reference as map key,
     *                      and device configuration value as map value
     */
    virtual void handleConfiguration(const std::string& deviceKey,
                                     const std::map<std::string, std::string>& configuration) = 0;
};
}    // namespace wolkabout

#endif    // CONFIGURATIONHANDLER_H
