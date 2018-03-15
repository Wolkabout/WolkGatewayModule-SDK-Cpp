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

#include "service/DataService.h"
#include "connectivity/ConnectivityService.h"
#include "model/ActuatorGetCommand.h"
#include "model/ActuatorSetCommand.h"
#include "model/Message.h"
#include "model/SensorReading.h"
#include "persistence/Persistence.h"
#include "protocol/DataProtocol.h"
#include "utilities/Logger.h"

namespace wolkabout
{
const std::string DataService::PERSISTENCE_KEY_DELIMITER = "+";

DataService::DataService(
  DataProtocol& protocol, Persistence& persistence, ConnectivityService& connectivityService,
  std::function<void(const std::string&, const std::string&, const std::string&)>& actuationHandler,
  std::function<ActuatorStatus(const std::string&, const std::string&)>& actuatorStatusProvider)
: m_protocol{protocol}
, m_persistence{persistence}
, m_connectivityService{connectivityService}
, m_actuationHandler{actuationHandler}
, m_actuatorStatusProvider{actuatorStatusProvider}
{
}

void DataService::messageReceived(std::shared_ptr<Message> message)
{
    const std::string deviceKey = m_protocol.extractDeviceKeyFromChannel(message->getChannel());
    if (deviceKey.empty())
    {
        LOG(WARN) << "Unable to extract device key from channel: " << message->getChannel();
        return;
    }

    if (m_protocol.isActuatorGetMessage(message->getChannel()))
    {
        auto command = m_protocol.makeActuatorGetCommand(message);
        if (!command)
        {
            LOG(WARN) << "Unable to parse message contents: " << message->getContent();
            return;
        }

        ActuatorStatus status = m_actuatorStatusProvider(deviceKey, command->getReference());
    }
    else if (m_protocol.isActuatorSetMessage(message->getChannel()))
    {
        auto command = m_protocol.makeActuatorSetCommand(message);
        if (!command)
        {
            LOG(WARN) << "Unable to parse message contents: " << message->getContent();
            return;
        }

        m_actuationHandler(deviceKey, command->getReference(), command->getValue());
    }
    else
    {
        LOG(WARN) << "Unable to parse message channel: " << message->getChannel();
    }
}

const Protocol& DataService::getProtocol()
{
    return m_protocol;
}

void DataService::addSensorReading(const std::string& deviceKey, const std::string& reference, const std::string& value,
                                   unsigned long long int rtc)
{
    auto sensorReading = std::make_shared<SensorReading>(value, reference, rtc);

    m_persistence.putSensorReading(makePersistenceKey(deviceKey, reference), sensorReading);
}

void DataService::addAlarm(const std::string& deviceKey, const std::string& reference, const std::string& value,
                           unsigned long long int rtc)
{
    auto alarm = std::make_shared<Alarm>(value, reference, rtc);

    m_persistence.putAlarm(makePersistenceKey(deviceKey, reference), alarm);
}

void DataService::acquireActuatorStatus(const std::string& deviceKey, const std::string& reference)
{
    const ActuatorStatus actuatorStatus = m_actuatorStatusProvider.operator()(deviceKey, reference);

    auto actuatorStatusWithRef =
      std::make_shared<ActuatorStatus>(actuatorStatus.getValue(), reference, actuatorStatus.getState());

    m_persistence.putActuatorStatus(makePersistenceKey(deviceKey, reference), actuatorStatusWithRef);
}

void DataService::publishSensorReadings()
{
    for (const auto& key : m_persistence.getSensorReadingsKeys())
    {
        const auto sensorReadings = m_persistence.getSensorReadings(key, PUBLISH_BATCH_ITEMS_COUNT);

        auto pair = parsePersistenceKey(key);
        if (pair.first.empty() || pair.second.empty())
        {
            LOG(ERROR) << "Unable to parse persistence key: " << key;
            break;
        }

        const std::shared_ptr<Message> outboundMessage = m_protocol.makeMessage(pair.first, sensorReadings);

        if (outboundMessage && m_connectivityService.publish(outboundMessage))
        {
            m_persistence.removeSensorReadings(key, PUBLISH_BATCH_ITEMS_COUNT);
        }
    }
}

void DataService::publishAlarms()
{
    for (const auto& key : m_persistence.getAlarmsKeys())
    {
        const auto alarms = m_persistence.getAlarms(key, PUBLISH_BATCH_ITEMS_COUNT);

        auto pair = parsePersistenceKey(key);
        if (pair.first.empty() || pair.second.empty())
        {
            LOG(ERROR) << "Unable to parse persistence key: " << key;
            break;
        }

        const std::shared_ptr<Message> outboundMessage = m_protocol.makeMessage(pair.first, alarms);

        if (outboundMessage && m_connectivityService.publish(outboundMessage))
        {
            m_persistence.removeAlarms(key, PUBLISH_BATCH_ITEMS_COUNT);
        }
    }
}

void DataService::publishActuatorStatuses()
{
    for (const auto& key : m_persistence.getGetActuatorStatusesKeys())
    {
        const auto actuatorStatus = m_persistence.getActuatorStatus(key);

        auto pair = parsePersistenceKey(key);
        if (pair.first.empty() || pair.second.empty())
        {
            LOG(ERROR) << "Unable to parse persistence key: " << key;
            break;
        }

        const std::shared_ptr<Message> outboundMessage = m_protocol.makeMessage(pair.first, {actuatorStatus});

        if (outboundMessage && m_connectivityService.publish(outboundMessage))
        {
            m_persistence.removeActuatorStatus(key);
        }
    }
}

std::string DataService::makePersistenceKey(const std::string& deviceKey, const std::string& reference)
{
    return deviceKey + PERSISTENCE_KEY_DELIMITER + reference;
}

std::pair<std::string, std::string> DataService::parsePersistenceKey(const std::string& key)
{
    auto pos = key.find(PERSISTENCE_KEY_DELIMITER);
    if (pos == std::string::npos)
    {
        return std::make_pair("", "");
    }

    auto deviceKey = key.substr(0, pos);
    auto reference = key.substr(pos + PERSISTENCE_KEY_DELIMITER.size(), std::string::npos);

    return std::make_pair(deviceKey, reference);
}
}    // namespace wolkabout
