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
#ifndef FIRMWAREUPDATESERVICE_H
#define FIRMWAREUPDATESERVICE_H

#include "InboundGatewayMessageHandler.h"

#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace wolkabout
{
class FirmwareInstaller;
class FirmwareVersionProvider;
class FirmwareUpdateCommand;
class FirmwareUpdateResponse;
class FirmwareUpdateProtocol;
class ConnectivityService;

class FirmwareUpdateService : public MessageListener
{
public:
    FirmwareUpdateService(FirmwareUpdateProtocol& protocol, std::shared_ptr<FirmwareInstaller> firmwareInstaller,
                          std::shared_ptr<FirmwareVersionProvider> firmwareVersionProvider,
                          ConnectivityService& connectivityService);

    void messageReceived(std::shared_ptr<Message> message) override;
    const Protocol& getProtocol() override;

private:
    class LocalFileDownloader
    {
    public:
        enum class ErrorCode
        {
            FILE_DOES_NOT_EXIST,
            UNSPECIFIED_ERROR
        };

        void download(const std::string& filePath, std::function<void(const std::string& path)> onSuccess,
                      std::function<void(ErrorCode)> onFail);
    } m_fileDownloader;

    void handleFirmwareUpdateCommand(const FirmwareUpdateCommand& command, const std::string deviceKey);

    void urlDownload(const std::string& deviceKey, const std::string& url, bool autoInstall);

    void downloadCompleted(const std::string& filePath, const std::string& deviceKey, bool autoInstall);

    void downloadFailed(LocalFileDownloader::ErrorCode errorCode, const std::string& deviceKey);

    void install(const std::string& deviceKey, const std::string& firmwareFilePath);

    void sendResponse(const FirmwareUpdateResponse& response, const std::string& deviceKey);

    void addToCommandBuffer(std::function<void()> command);

    FirmwareUpdateProtocol& m_protocol;

    std::shared_ptr<FirmwareInstaller> m_firmwareInstaller;
    std::shared_ptr<FirmwareVersionProvider> m_firmwareVersionProvider;

    ConnectivityService& m_connectivityService;

    CommandBuffer m_commandBuffer;
};
}    // namespace wolkabout

#endif    // FIRMWAREUPDATESERVICE_H
