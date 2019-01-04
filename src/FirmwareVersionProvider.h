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
#ifndef FIRMWAREVERSIONPROVIDER_H
#define FIRMWAREVERSIONPROVIDER_H

#include <string>

namespace wolkabout
{
class FirmwareVersionProvider
{
public:
    virtual ~FirmwareVersionProvider() = default;

    /**
     * @brief Provide firmware version for specified device
     * @param deviceKey Key of the device
     * @return firmware version
     */
    virtual std::string getFirmwareVersion(const std::string& deviceKey) = 0;
};
}    // namespace wolkabout

#endif    // FIRMWAREVERSIONPROVIDER_H
