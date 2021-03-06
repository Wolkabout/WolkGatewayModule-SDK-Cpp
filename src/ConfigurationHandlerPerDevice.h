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
#ifndef CONFIGURATIONHANDLERPERDEVICE_H
#define CONFIGURATIONHANDLERPERDEVICE_H

#include "core/model/ConfigurationItem.h"

#include <string>
#include <vector>

namespace wolkabout
{
class ConfigurationHandlerPerDevice
{
public:
    /**
     * @brief When new set of device configuration values is given from platform, it will be delivered to this method.
     *        This method should update device configuration with received configuration values.<br>

     *        Must be implemented as non blocking<br>
     *        Must be implemented as thread safe
     * @param deviceKey Device key
     * @param configuration as vector of wolkabout::ConfigurationItem
     */
    virtual void handleConfiguration(const std::string& deviceKey,
                                     const std::vector<ConfigurationItem>& configuration) = 0;

    virtual ~ConfigurationHandlerPerDevice() = default;
};
}    // namespace wolkabout

#endif    // CONFIGURATIONHANDLERPERDEVICE_H
