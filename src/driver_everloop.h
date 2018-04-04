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

#ifndef SRC_DRIVER_EVERLOOP_H_
#define SRC_DRIVER_EVERLOOP_H_

#include <memory>

#include "./malos_matrixio_base.h"

#include "matrix_hal/everloop.h"
#include "matrix_hal/matrixio_bus.h"

const char kEverloopDriverName[] = "Everloop";

namespace matrix_malos {

class EverloopDriver : public MalosWishboneBase {
 public:
  EverloopDriver() : MalosWishboneBase(kEverloopDriverName) {
    SetProvidesUpdates(false);
    SetNeedsKeepalives(false);
    SetNotesForHuman(
        "Write-only. There are 35 leds. Values range from 0 to 255. Check "
        "message EverloopImage (protocol buffer)");
  }

  // Receive a copy of the shared wishbone bus. Not owned.
  void SetupMatrixIOBus(matrix_hal::MatrixIOBus* wishbone) override {
    writer_.reset(new matrix_hal::Everloop);
    writer_->Setup(wishbone);
  }

  // Read configuration of LEDs (from the outside world).
  bool ProcessConfig(const pb::driver::DriverConfig& config) override;

 private:
  // Everloop writer.
  std::unique_ptr<matrix_hal::Everloop> writer_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_EVERLOOP_H_
