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

#include "./src/malos.pb.h"

namespace matrix_malos {

bool ZigbeeBulbDriver::ProcessConfig(const DriverConfig& config) {
  ZigbeeBulbConfig bulb_config(config.zigbee_bulb());

  std::cerr << "ZigbeeBulb Got configuration" << std::endl;
  std::cerr << "Connect to" << bulb_config.address() << ":" << bulb_config.port() << std::endl;

  return true;
}
  //matrix_hal::EverloopImage image_for_hal;

}  // namespace matrix_malos
