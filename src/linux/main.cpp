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

#include <cstdlib>
#include <iostream>
#include <string>

#include <pthread.h>
#include <signal.h>

#include "broker_service.h"

namespace
{
void PrintUsage(const char* program_name)
{
  std::cout << "Usage: " << program_name << " [--config PATH] [--log-file PATH] [--version]\n"
            << "\n"
            << "  --config PATH    Broker configuration file (default: "
            << LinuxBrokerOsInterface::kDefaultConfigFilePath << ")\n"
            << "  --log-file PATH  Broker log file (default: " << LinuxBrokerOsInterface::kDefaultLogFilePath << ")\n"
            << "  --version        Print version information and exit\n"
            << "  --help           Print this help and exit\n";
}

bool BlockServiceSignals()
{
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGHUP);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);
  return pthread_sigmask(SIG_BLOCK, &signals, nullptr) == 0;
}
}  // namespace

int main(int argc, char* argv[])
{
  std::string config_file_path = LinuxBrokerOsInterface::kDefaultConfigFilePath;
  std::string log_file_path = LinuxBrokerOsInterface::kDefaultLogFilePath;
  bool        print_version = false;

  for (int index = 1; index < argc; ++index)
  {
    const std::string argument(argv[index]);
    if ((argument == "--config") || (argument == "--log-file"))
    {
      if (++index >= argc)
      {
        std::cerr << "Missing path after " << argument << ".\n";
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
      }

      if (argument == "--config")
        config_file_path = argv[index];
      else
        log_file_path = argv[index];
    }
    else if (argument == "--version")
    {
      print_version = true;
    }
    else if (argument == "--help")
    {
      PrintUsage(argv[0]);
      return EXIT_SUCCESS;
    }
    else
    {
      std::cerr << "Unknown argument: " << argument << "\n";
      PrintUsage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  BrokerService service(config_file_path, log_file_path);
  if (print_version)
  {
    service.PrintVersion();
    return EXIT_SUCCESS;
  }

  if (!BlockServiceSignals())
  {
    std::cerr << "Could not block service signals.\n";
    return EXIT_FAILURE;
  }

  if (!service.Init())
    return EXIT_FAILURE;

  const bool run_succeeded = service.Run();
  service.Deinit();
  return run_succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}
