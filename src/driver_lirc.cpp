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

#include <iostream>
#include "./driver_lirc.h"
#include "./src/driver.pb.h"

namespace matrix_malos {

bool LircDriver::ProcessConfig(const DriverConfig& config) {
  LircParams lirc(config.lirc());

  if (lirc.device() == "" || lirc.command() == "") {
    std::string error_msg("device or command parameter is missing");
    error_msg += kLircDriverName;
    zmq_push_error_->Send(error_msg);
    return false;
  }

  std::cout << "device :" << lirc.device() << "\n";
  std::cout << "command:" << lirc.command() << "\n";

  return true;

}

}  // namespace matrix_malos
