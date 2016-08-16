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

#ifndef SRC_DRIVER_MANAGER_H_
#define SRC_DRIVER_MANAGER_H_

#include <string>
#include <vector>
#include "./malos_wishbone_base.h"

namespace matrix_malos {

class DriverManager {
 public:
  explicit DriverManager(int port, const std::string bind_scope)
      : driver_info_port_(port), bind_scope_(bind_scope) {}

  // Register a driver.
  void RegisterDriver(const MalosWishboneBase* driver) {
    drivers_.push_back(driver);
  }

  void ServeInfoRequestsForEver();

 private:
  // Port to respond queries about registered drivers.
  int driver_info_port_;
  // Bind scope (interfaces to listeon on).
  std::string bind_scope_;
  // Drivers that have been registered.
  std::vector<const MalosWishboneBase*> drivers_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_MANAGER_H_
