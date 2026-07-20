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

#ifndef LINUX_BROKER_SERVICE_H_
#define LINUX_BROKER_SERVICE_H_

#include <atomic>
#include <string>

#include "broker_shell.h"
#include "etcpal/cpp/thread.h"
#include "linux_broker_os_interface.h"

class BrokerService
{
public:
  BrokerService(std::string config_file_path, std::string log_file_path);

  bool Init() { return broker_shell_.Init(); }
  void Deinit() { broker_shell_.Deinit(); }

  bool Run();
  void PrintVersion() { broker_shell_.PrintVersion(); }

private:
  LinuxBrokerOsInterface os_interface_;
  BrokerShell            broker_shell_;
  etcpal::Thread         shell_thread_;
  std::atomic<bool>      shell_finished_{false};
  bool                   shell_succeeded_{false};
};

#endif  // LINUX_BROKER_SERVICE_H_
