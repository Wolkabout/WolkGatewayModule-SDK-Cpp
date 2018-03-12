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

#ifndef REGISTRATIONPROTOCOL_H
#define REGISTRATIONPROTOCOL_H

#include "utilities/json.hpp"
#include <memory>
#include <string>
#include <vector>

namespace wolkabout
{
class Message;
class DeviceRegistrationRequest;
class DeviceRegistrationResponse;
class DeviceReregistrationResponse;
class DeviceManifest;

class RegistrationProtocol
{
public:
    ~RegistrationProtocol() = delete;

    static std::vector<std::string> getDeviceTopics();
    static std::vector<std::string> getPlatformTopics();

    static std::shared_ptr<Message> make(const std::string& deviceKey, const DeviceRegistrationRequest& request,
                                         bool isGateway = false);
    static std::shared_ptr<Message> make(const std::string& deviceKey, const DeviceReregistrationResponse& response,
                                         bool isGateway = false);

    static std::shared_ptr<DeviceRegistrationRequest> makeRegistrationRequest(std::shared_ptr<Message> message);
    static std::shared_ptr<DeviceRegistrationResponse> makeRegistrationResponse(std::shared_ptr<Message> message);

    static bool makeManifest(const nlohmann::json& text, DeviceManifest& manifest);

    static std::string getDeviceKeyFromChannel(const std::string& channel);

private:
    static const std::vector<std::string> m_devicTopics;
    static const std::vector<std::string> m_platformTopics;

    static const std::vector<std::string> m_deviceMessageTypes;
    static const std::vector<std::string> m_platformMessageTypes;

    static const std::string CHANNEL_DELIMITER;
    static const std::string CHANNEL_WILDCARD;

    static const std::string DEVICE_TO_PLATFORM_DIRECTION;
    static const std::string PLATFORM_TO_DEVICE_DIRECTION;

    static const std::string GATEWAY_PATH_PREFIX;
    static const std::string DEVICE_PATH_PREFIX;
    static const std::string REFERENCE_PATH_PREFIX;

    static const std::string REGISTER_DEVICE_TYPE;
    static const std::string REREGISTER_DEVICE_TYPE;

    static const std::string DEVICE_REGISTRATION_REQUEST_TOPIC_ROOT;
    static const std::string DEVICE_REREGISTRATION_RESPONSE_TOPIC_ROOT;

    static const std::string DEVICE_REGISTRATION_RESPONSE_TOPIC_ROOT;
    static const std::string DEVICE_REREGISTRATION_REQUEST_TOPIC_ROOT;

    static const std::string REGISTRATION_RESPONSE_OK;
    static const std::string REGISTRATION_RESPONSE_ERROR_KEY_CONFLICT;
    static const std::string REGISTRATION_RESPONSE_ERROR_MANIFEST_CONFLICT;
    static const std::string REGISTRATION_RESPONSE_ERROR_MAX_NUMBER_OF_DEVICES_EXCEEDED;
    static const std::string REGISTRATION_RESPONSE_ERROR_READING_PAYLOAD;
    static const std::string REGISTRATION_RESPONSE_ERROR_GATEWAY_NOT_FOUND;
    static const std::string REGISTRATION_RESPONSE_ERROR_NO_GATEWAY_MANIFEST;

    static constexpr int DIRRECTION_POS = 0;
    static constexpr int TYPE_POS = 1;
    static constexpr int GATEWAY_TYPE_POS = 2;
    static constexpr int GATEWAY_KEY_POS = 3;
    static constexpr int DEVICE_TYPE_POS = 2;
    static constexpr int DEVICE_KEY_POS = 3;
    static constexpr int GATEWAY_DEVICE_TYPE_POS = 4;
    static constexpr int GATEWAY_DEVICE_KEY_POS = 5;
    static constexpr int GATEWAY_REFERENCE_TYPE_POS = 4;
    static constexpr int GATEWAY_REFERENCE_VALUE_POS = 5;
    static constexpr int DEVICE_REFERENCE_TYPE_POS = 4;
    static constexpr int DEVICE_REFERENCE_VALUE_POS = 5;
    static constexpr int GATEWAY_DEVICE_REFERENCE_TYPE_POS = 6;
    static constexpr int GATEWAY_DEVICE_REFERENCE_VALUE_POS = 7;
};
}    // namespace wolkabout

#endif
