#ifndef DEVICESTATUSSERVICE_H
#define DEVICESTATUSSERVICE_H

#include "InboundMessageHandler.h"
#include "model/DeviceStatus.h"

#include <string>

namespace wolkabout
{
class StatusProtocol;
class ConnectivityService;

typedef std::function<void(const std::string&)> StatusRequestHandler;

class DeviceStatusService : public MessageListener
{
public:
    DeviceStatusService(StatusProtocol& protocol, ConnectivityService& connectivityService,
                        const StatusRequestHandler& statusRequestHandler);

    void messageReceived(std::shared_ptr<Message> message) override;
    const Protocol& getProtocol() override;

    void publishDeviceStatus(const std::string& deviceKey, DeviceStatus status);

private:
    StatusProtocol& m_protocol;
    ConnectivityService& m_connectivityService;

    StatusRequestHandler m_statusRequestHandler;
};
}

#endif    // DEVICESTATUSSERVICE_H
