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

#include "./driver_imu.h"

#include "./src/malos.pb.h"
#include "matrix_hal/imu_data.h"

namespace matrix_malos {

bool ImuDriver::SendUpdate() {
  matrix_hal::IMUData data;
  if (!imu_reader_->Read(&data)) {
    return false;
  }

  Imu imu_pb;
  imu_pb.set_yaw(data.yaw);
  imu_pb.set_pitch(data.pitch);
  imu_pb.set_roll(data.roll);

  std::string buffer;
  imu_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

}  // namespace matrix_malos
