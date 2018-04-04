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

#ifndef SRC_DRIVER_IMU_H_
#define SRC_DRIVER_IMU_H_

#include <memory>

#include "./malos_matrixio_base.h"

#include "matrix_hal/imu_sensor.h"
#include "matrix_hal/matrixio_bus.h"

namespace matrix_malos {

class ImuDriver : public MalosWishboneBase {
 public:
  ImuDriver() : MalosWishboneBase("IMU") {
    SetNotesForHuman(
        "Provides yaw/pitch/roll. Check Imu message (protocol buffer)");
  }

  // Receive a copy of the shared wishbone bus. Not owned.
  void SetupMatrixIOBus(matrix_hal::MatrixIOBus* wishbone) override {
    imu_reader_.reset(new matrix_hal::IMUSensor);
    imu_reader_->Setup(wishbone);
  }

  // Send update to 0MQ zqm_push_update_ queue when called.
  bool SendUpdate() override;

 private:
  // Reader of IMU data.
  std::unique_ptr<matrix_hal::IMUSensor> imu_reader_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_IMU_H_
