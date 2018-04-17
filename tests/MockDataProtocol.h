#ifndef MOCKDATAPROTOCOL_H
#define MOCKDATAPROTOCOL_H

#include "model/ActuatorGetCommand.h"
#include "model/ActuatorSetCommand.h"
#include "model/ConfigurationSetCommand.h"
#include "protocol/DataProtocol.h"

#include <gmock/gmock.h>

class MockDataProtocol : public wolkabout::DataProtocol
{
public:
    MOCK_CONST_METHOD1(extractReferenceFromChannel, std::string(const std::string&));

    const std::string& getName() const override { return m_name; }

    const std::vector<std::string>& getInboundChannels() const override { return m_channels; };

    MOCK_CONST_METHOD1(extractDeviceKeyFromChannel, std::string(const std::string&));

    MOCK_CONST_METHOD1(isActuatorSetMessage, bool(const std::string& channel));

    MOCK_CONST_METHOD1(isActuatorGetMessage, bool(const std::string& channel));

    MOCK_CONST_METHOD1(isConfigurationSetMessage, bool(const std::string& channel));

    MOCK_CONST_METHOD1(isConfigurationGetMessage, bool(const std::string& channel));

    std::unique_ptr<wolkabout::ActuatorGetCommand> makeActuatorGetCommand(
      std::shared_ptr<wolkabout::Message> message) const override
    {
        return std::unique_ptr<wolkabout::ActuatorGetCommand>(makeActuatorGetCommandProxy(message));
    }

    MOCK_CONST_METHOD1(makeActuatorGetCommandProxy,
                       wolkabout::ActuatorGetCommand*(std::shared_ptr<wolkabout::Message> message));

    std::unique_ptr<wolkabout::ActuatorSetCommand> makeActuatorSetCommand(
      std::shared_ptr<wolkabout::Message> message) const override
    {
        return std::unique_ptr<wolkabout::ActuatorSetCommand>(makeActuatorSetCommandProxy(message));
    }

    MOCK_CONST_METHOD1(makeActuatorSetCommandProxy,
                       wolkabout::ActuatorSetCommand*(std::shared_ptr<wolkabout::Message> message));

    std::unique_ptr<wolkabout::ConfigurationSetCommand> makeConfigurationSetCommand(
      std::shared_ptr<wolkabout::Message> message) const override
    {
        return std::unique_ptr<wolkabout::ConfigurationSetCommand>(makeConfigurationSetCommandProxy(message));
    }

    MOCK_CONST_METHOD1(makeConfigurationSetCommandProxy,
                       wolkabout::ConfigurationSetCommand*(std::shared_ptr<wolkabout::Message> message));

    MOCK_CONST_METHOD3(makeMessage, std::shared_ptr<wolkabout::Message>(
                                      const std::string& deviceKey,
                                      std::vector<std::shared_ptr<wolkabout::SensorReading>> sensorReadings,
                                      const std::string& delimiter));
    MOCK_CONST_METHOD2(makeMessage,
                       std::shared_ptr<wolkabout::Message>(const std::string& deviceKey,
                                                           std::vector<std::shared_ptr<wolkabout::Alarm>> alarms));
    MOCK_CONST_METHOD2(makeMessage, std::shared_ptr<wolkabout::Message>(
                                      const std::string& deviceKey,
                                      std::vector<std::shared_ptr<wolkabout::ActuatorStatus>> actuatorStatuses));

    MOCK_CONST_METHOD2(makeFromConfiguration,
                       std::shared_ptr<wolkabout::Message>(const std::string& deviceKey,
                                                           const std::map<std::string, std::string> configuration));

private:
    const std::string m_name{""};
    const std::vector<std::string> m_channels{};
};

#endif    // MOCKDATAPROTOCOL_H
