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

#include "broker_service.h"

#include <cerrno>
#include <csignal>
#include <utility>

BrokerService::BrokerService(std::string config_file_path, std::string log_file_path)
    : os_interface_(std::move(config_file_path), std::move(log_file_path)), broker_shell_(os_interface_)
{
}

bool BrokerService::Run()
{
  const auto start_result = shell_thread_.Start([this]() {
    shell_succeeded_ = broker_shell_.Run();
    shell_finished_.store(true);
  });
  if (!start_result.IsOk())
    return false;

  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGHUP);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);

  while (!shell_finished_.load())
  {
    const timespec timeout{0, 250000000};
    const int      received_signal = sigtimedwait(&signals, nullptr, &timeout);

    if (received_signal == SIGHUP)
    {
      broker_shell_.log().Info("SIGHUP received - requesting broker restart and configuration reload.");
      broker_shell_.RequestRestart();
    }
    else if ((received_signal == SIGINT) || (received_signal == SIGTERM))
    {
      broker_shell_.AsyncShutdown();
      break;
    }
    else if ((received_signal < 0) && (errno != EAGAIN) && (errno != EINTR))
    {
      broker_shell_.log().Error("Error while waiting for a service signal.");
      broker_shell_.AsyncShutdown();
      break;
    }
  }

  shell_thread_.Join();
  return shell_succeeded_;
}
