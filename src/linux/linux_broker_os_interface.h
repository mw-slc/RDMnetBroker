/******************************************************************************
 * Copyright 2026 ETC Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************
 * This file is a part of RDMnetBroker. For more information, go to:
 * https://github.com/ETCLabs/RDMnetBroker
 *****************************************************************************/

#ifndef LINUX_BROKER_OS_INTERFACE_H_
#define LINUX_BROKER_OS_INTERFACE_H_

#include <fstream>
#include <string>

#include "broker_os_interface.h"

class LinuxBrokerOsInterface final : public BrokerOsInterface
{
public:
  static constexpr const char* kDefaultConfigFilePath = "/etc/rdmnetbroker/broker.conf";
  static constexpr const char* kDefaultLogFilePath = "/var/log/rdmnetbroker/broker.log";

  LinuxBrokerOsInterface(std::string config_file_path, std::string log_file_path);
  ~LinuxBrokerOsInterface();

  // BrokerOsInterface
  std::string                           GetLogFilePath() const override;
  bool                                  OpenLogFile() override;
  std::pair<std::string, std::ifstream> GetConfFile(etcpal::Logger& log) override;

  // etcpal::LogMessageHandler
  etcpal::LogTimestamp GetLogTimestamp() override;
  void                 HandleLogMessage(const EtcPalLogStrings& strings) override;

private:
  std::string   config_file_path_;
  std::string   log_file_path_;
  std::ofstream log_stream_;
};

#endif  // LINUX_BROKER_OS_INTERFACE_H_
