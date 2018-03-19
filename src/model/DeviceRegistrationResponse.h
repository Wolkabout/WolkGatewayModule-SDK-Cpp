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

#ifndef DEVICEREGISTRATIONRESPONSE_H
#define DEVICEREGISTRATIONRESPONSE_H

namespace wolkabout {
class DeviceRegistrationResponse {
public:
  enum class Result {
    OK,
    ERROR_KEY_CONFLICT,
    ERROR_MAXIMUM_NUMBER_OF_DEVICES_EXCEEDED,
    ERROR_MANIFEST_CONFLICT,
    ERROR_READING_PAYLOAD,
    ERROR_GATEWAY_NOT_FOUND,
    ERROR_NO_GATEWAY_MANIFEST
  };

  DeviceRegistrationResponse() = default;
  DeviceRegistrationResponse(DeviceRegistrationResponse::Result result);

  virtual ~DeviceRegistrationResponse() = default;

  DeviceRegistrationResponse::Result getResult() const;

private:
  DeviceRegistrationResponse::Result m_result;
};
} // namespace wolkabout

#endif // DEVICEREGISTRATIONRESPONSE_H
