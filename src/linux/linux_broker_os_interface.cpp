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

#include "linux_broker_os_interface.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <system_error>
#include <utility>

#include "broker_version.h"

namespace
{
constexpr int kMaxLogRotationFiles = 5;

bool RotateLogs(const std::filesystem::path& log_file_path)
{
  std::error_code error;

  for (int index = kMaxLogRotationFiles; index > 0; --index)
  {
    const std::filesystem::path source =
        (index == 1 ? log_file_path : std::filesystem::path(log_file_path.string() + "." + std::to_string(index - 1)));
    const std::filesystem::path destination(log_file_path.string() + "." + std::to_string(index));

    if (!std::filesystem::exists(source, error))
    {
      if (error)
        return false;
      continue;
    }

    std::filesystem::remove(destination, error);
    if (error)
      return false;

    std::filesystem::rename(source, destination, error);
    if (error)
      return false;
  }

  return true;
}
}  // namespace

LinuxBrokerOsInterface::LinuxBrokerOsInterface(std::string config_file_path, std::string log_file_path)
    : config_file_path_(std::move(config_file_path)), log_file_path_(std::move(log_file_path))
{
}

LinuxBrokerOsInterface::~LinuxBrokerOsInterface()
{
  if (log_stream_.is_open())
    log_stream_.close();
}

std::string LinuxBrokerOsInterface::GetLogFilePath() const
{
  return log_file_path_;
}

bool LinuxBrokerOsInterface::OpenLogFile()
{
  const std::filesystem::path log_path(log_file_path_);
  std::error_code             error;

  if (log_path.has_parent_path())
  {
    std::filesystem::create_directories(log_path.parent_path(), error);
    if (error)
    {
      std::cerr << "FATAL: Could not create log directory '" << log_path.parent_path().string() << "' ("
                << error.message() << ").\n";
      return false;
    }
  }

  const bool rotation_succeeded = RotateLogs(log_path);
  log_stream_.open(log_path, std::ios::out | std::ios::trunc);
  if (!log_stream_.is_open())
  {
    std::cerr << "FATAL: Could not open log file '" << log_file_path_ << "'.\n";
    return false;
  }

  const auto time = GetLogTimestamp();
  log_stream_ << "Starting RDMnet Broker Service version " << BrokerVersion::VersionString() << " on "
              << std::setfill('0') << std::setw(4) << time.get().year << "-" << std::setw(2) << time.get().month << "-"
              << std::setw(2) << time.get().day << " at " << std::setw(2) << time.get().hour << ":" << std::setw(2)
              << time.get().minute << ":" << std::setw(2) << time.get().second << "...\n";

  if (!rotation_succeeded)
    log_stream_ << "WARNING: rotating log files failed.\n";

  log_stream_.flush();
  return true;
}

std::pair<std::string, std::ifstream> LinuxBrokerOsInterface::GetConfFile(etcpal::Logger& log)
{
  static_cast<void>(log);
  std::ifstream conf_file(config_file_path_);
  return std::make_pair(config_file_path_, std::move(conf_file));
}

etcpal::LogTimestamp LinuxBrokerOsInterface::GetLogTimestamp()
{
  const auto now = std::chrono::system_clock::now();
  const auto now_time = std::chrono::system_clock::to_time_t(now);
  std::tm    local_time{};
  localtime_r(&now_time, &local_time);

  const auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

  return etcpal::LogTimestamp(
      static_cast<unsigned int>(local_time.tm_year + 1900), static_cast<unsigned int>(local_time.tm_mon + 1),
      static_cast<unsigned int>(local_time.tm_mday), static_cast<unsigned int>(local_time.tm_hour),
      static_cast<unsigned int>(local_time.tm_min), static_cast<unsigned int>(local_time.tm_sec),
      static_cast<unsigned int>(milliseconds), static_cast<int>(local_time.tm_gmtoff / 60));
}

void LinuxBrokerOsInterface::HandleLogMessage(const EtcPalLogStrings& strings)
{
  if (log_stream_.is_open())
  {
    log_stream_ << strings.human_readable << '\n';
    log_stream_.flush();
  }
}
