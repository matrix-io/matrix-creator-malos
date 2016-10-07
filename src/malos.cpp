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

#include <chrono>
#include <iostream>
#include <thread>

#include "./driver_manager.h"
#include "./driver_imu.h"
#include "./driver_humidity.h"
#include "./driver_pressure.h"
#include "./driver_uv.h"
#include "./driver_everloop.h"
#include "./driver_zigbee_bulb.h"
#include "./driver_micarray_alsa.h"
#include "./driver_lirc.h"
#include "./driver_servo.h"

#include "matrix_hal/wishbone_bus.h"

const int kBasePort = 20012;

const char kUnsecureBindScope[] = "*";

namespace matrix_malos {

int RunServer() {
  std::cerr << "**************" << std::endl;
  std::cerr << "MALOS starting" << std::endl;
  std::cerr << "**************" << std::endl;
  std::cerr << std::endl;

  matrix_hal::WishboneBus* wishbone_bus = new matrix_hal::WishboneBus();
  wishbone_bus->SpiInit();

  DriverManager driver_manager(kBasePort, kUnsecureBindScope);
  std::cerr << "You can query specific driver info using port " +
                   std::to_string(20012)
            << "." << std::endl;

  ImuDriver driver_imu;
  driver_imu.SetupWishboneBus(wishbone_bus);
  if (!driver_imu.Init(kBasePort + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_imu);

  HumidityDriver driver_humidity;
  driver_humidity.SetupWishboneBus(wishbone_bus);
  if (!driver_humidity.Init(kBasePort + 4 * 1 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_humidity);

  EverloopDriver driver_everloop;
  driver_everloop.SetupWishboneBus(wishbone_bus);
  if (!driver_everloop.Init(kBasePort + 4 * 2 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_everloop);

  PressureDriver driver_pressure;
  driver_pressure.SetupWishboneBus(wishbone_bus);
  if (!driver_pressure.Init(kBasePort + 4 * 3 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_pressure);

  UVDriver driver_uv;
  driver_uv.SetupWishboneBus(wishbone_bus);
  if (!driver_uv.Init(kBasePort + 4 * 4 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_uv);

  ZigbeeBulbDriver driver_zigbee_bulb;
  if (!driver_zigbee_bulb.Init(kBasePort + 4 * 5 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_zigbee_bulb);

  MicArrayAlsaDriver driver_micarray_drive;
  driver_micarray_drive.SetupWishboneBus(wishbone_bus);

  if (!driver_micarray_drive.Init(kBasePort + 4 * 6 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_micarray_drive);

  LircDriver driver_lirc;
  if (!driver_lirc.Init(kBasePort + 4 * 7 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_lirc);

  ServoDriver driver_servo;
  if (!driver_servo.Init(kBasePort + 4 * 8 + 1, kUnsecureBindScope)) {
    return 1;
  }
  driver_manager.RegisterDriver(&driver_servo);

  driver_manager.ServeInfoRequestsForEver();

  return 0;  // Never reached.
}
}  // namespace matrix_malos

int main(int, char* []) { return matrix_malos::RunServer(); }
