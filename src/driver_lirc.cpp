/*
 * Copyright 2016 <Admobilize>
 * MATRIX Labs  [http://creator.matrix.one]
 * This file is part of MATRIX Creator MALOS
 *
 * MATRIX Creator MALOS is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include <iostream>
#include <string>

#include "./driver_lirc.h"
#include "./src/driver.pb.h"

namespace matrix_malos {

const bool kLircDriverDebugEnabled = false;

bool LircDriver::ProcessConfig(const DriverConfig& config) {
  LircParams lirc(config.lirc());

  if (lirc.device() == "" || lirc.command() == "" ||
      !isValidLircSymbol(lirc.device()) || !isValidLircSymbol(lirc.command())) {
    zmq_push_error_->Send(
        std::string(
            "Device or command parameter is missing or invalid for the ") +
        kLircDriverName + " driver");
    return false;
  }

  if (kLircDriverDebugEnabled) {
    std::cout << "device :" << lirc.device() << "\t";
    std::cout << "command:" << lirc.command() << std::endl;
  }

  if (system(std::string("irsend SEND_ONCE " + lirc.device() + " " +
                         lirc.command())
                 .c_str()) == -1) {
    zmq_push_error_->Send("ir command failed");
    return false;
  }
  return true;
}

bool LircDriver::isValidLircSymbol(const std::string word) {
  for (const char c : word)
    if (!(isalnum(c) || c == '_')) return false;
  return true;
}
}  // namespace matrix_malos
