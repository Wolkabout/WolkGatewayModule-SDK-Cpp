/**
 * Copyright 2021 Wolkabout s.r.o.
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

#ifndef WOLKGATEWAYMODULE_PLATFORMSTATUSMESSAGE_H
#define WOLKGATEWAYMODULE_PLATFORMSTATUSMESSAGE_H

#include <string>

namespace wolkabout
{
// This is the enumeration that holds all the possibilities of the connection status values sent out.
enum class ConnectivityStatus
{
    NONE = -1,
    CONNECTED,
    OFFLINE
};

/**
 * This is a utility method that is used to convert a string into a ConnectivityStatus value.
 *
 * @param value The value as a string.
 * @return ConnectivityStatus value parsed from the string. `NONE` if failed to parse.
 */
ConnectivityStatus fromString(const std::string& value);

/**
 * This message carries the platform connectivity status value sent out by the gateway.
 */
class PlatformStatusMessage
{
public:
    /**
     * Default constructor for this class.
     *
     * @param status The status of the connection.
     */
    explicit PlatformStatusMessage(ConnectivityStatus status);

    /**
     * Default getter for the status.
     *
     * @return The status as an enum value.
     */
    ConnectivityStatus getStatus() const;

private:
    // Here is the place for the connectivity status.
    ConnectivityStatus m_status;
};
}    // namespace wolkabout

#endif    // WOLKGATEWAYMODULE_PLATFORMSTATUSMESSAGE_H
