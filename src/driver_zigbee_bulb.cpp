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

#include "./driver_zigbee_bulb.h"

#include "./src/driver.pb.h"

namespace matrix_malos {

bool ZigbeeBulbDriver::ProcessConfig(const DriverConfig& config) {
  ZigbeeBulbConfig bulb_config(config.zigbee_bulb());

  std::cout << "ZigbeeBulb Got configuration" << std::endl;
  std::cout << "Connect to " << bulb_config.address() << ":"
            << bulb_config.port() << std::endl;

  tcp_client_.reset(new TcpClient());
  if (tcp_client_->Connect(bulb_config.address(), bulb_config.port())) {
    std::cout << "connected" << std::endl << std::flush;
  } else {
    std::cout << "NOT connected" << std::endl;
    return false;
  }

  std::string line;
  tcp_client_->GetLine(&line);

  std::cout.flush();

  return true;
}
// matrix_hal::EverloopImage image_for_hal;

}  // namespace matrix_malos
