/*
 * Copyright 2016 <Admobilize>
 * MATRIX Labs  [http://creator.matrix.one]
 * This file is part of MATRIX Creator MALOS
 * Author: @hpsaturn
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
#include <fstream>
#include <iostream>
#include <string>

#include <matrix_io/malos/v1/driver.pb.h>
#include "./driver_lirc.h"
namespace pb = matrix_io::malos::v1;

namespace matrix_malos {

const bool kLircDriverDebugEnabled = false;

bool LircDriver::ProcessConfig(const pb::driver::DriverConfig& config) {
  pb::comm::LircParams lirc(config.lirc());

  if (!lirc.config().empty()) {
    // LIRC remotes config from MOS via proto
    std::ofstream remotes_config("/etc/lirc/lircd.matrix.conf");
    remotes_config << lirc.config();
    remotes_config.close();

    if (kLircDriverDebugEnabled) {
      std::cerr << "new remote database saved" << std::endl;
    }
    // LIRC service restart
    // TODO(@hpsaturn): migrate to dbus API (not supported in raspbian version)
    if (system(std::string("service lirc stop").c_str()) == -1) {
      zmq_push_error_->Send("LIRC service stop failed!");
      return false;
    }
    if (system(std::string("service lirc start").c_str()) == -1) {
      zmq_push_error_->Send("LIRC service start failed!");
      return false;
    }

    if (kLircDriverDebugEnabled) {
      std::cerr << "LIRC service restart done." << std::endl;
    }
    return true;
  }

  // execute commands over remote device
  if (lirc.device() == "" || lirc.command() == "" ||
      !isValidLircSymbol(lirc.device()) || !isValidLircSymbol(lirc.command())) {
    const std::string msg_error =
        std::string(kLircDriverName) +
        " error: Device or command parameter is missing or invalid.";
    std::cerr << msg_error << std::endl;
    zmq_push_error_->Send(msg_error);

    return false;
  }

  if (kLircDriverDebugEnabled) {
    std::cerr << "device :" << lirc.device() << "\t";
    std::cerr << "command:" << lirc.command() << std::endl;
  }

  if (system(std::string("irsend SEND_ONCE " + lirc.device() + " " +
                         lirc.command())
                 .c_str()) == -1) {
    zmq_push_error_->Send("ir command failed");
    return false;
  }
  return true;
}

bool LircDriver::isValidLircSymbol(const std::string& word) {
  for (const char c : word)
    if (!(isalnum(c) || c == '_' || c == '-')) return false;
  return true;
}
}  // namespace matrix_malos
