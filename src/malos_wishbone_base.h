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

#ifndef SRC_MALOS_WISHBONE_BASE_H_
#define SRC_MALOS_WISHBONE_BASE_H_

#include <memory>
#include <string>

#include "./zmq_push.h"
#include "./zmq_pull.h"

#include "matrix_hal/wishbone_bus.h"
#include "./malos_base.h"
#include "./src/driver.pb.h"

namespace matrix_malos {

class MalosWishboneBase : public MalosBase {
 public:
  explicit MalosWishboneBase(const std::string& driver_name)
      : MalosBase(driver_name) {}

  // This function should be overridden by drivers that use the wishbone bus.
  virtual void SetupWishboneBus(matrix_hal::WishboneBus*) {}
};

}  // namespace matrix_malos

#endif  // SRC_MALOS_WISHBONE_BASE_H_
