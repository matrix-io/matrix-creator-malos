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

#ifndef SRC_DRIVER_UV_H_
#define SRC_DRIVER_UV_H_

#include <memory>

#include "./malos_wishbone_base.h"

#include "matrix_hal/wishbone_bus.h"
#include "matrix_hal/uv_sensor.h"

namespace matrix_malos {

class UVDriver : public MalosWishboneBase {
 public:
  UVDriver() : MalosWishboneBase("UV") {
    SetNotesForHuman(
        "Provides UV index. Check UV index message (protocol "
        "buffer). Also provides a string with the UV risk according to the OMS. See: https://www.epa.gov/sunsafety/uv-index-scale-0");
  }

  // Receive a copy of the shared wishbone bus. Not owned.
  void SetupWishboneBus(matrix_hal::WishboneBus* wishbone) override {
    reader_.reset(new matrix_hal::UVSensor);
    reader_->Setup(wishbone);
  }

  // Send update to 0MQ zqm_push_update_ queue when called.
  bool SendUpdate() override;

 private:
  // Reader of UV data.
  std::unique_ptr<matrix_hal::UVSensor> reader_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_UV_H_
