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

#include <string>
#include "./driver_uv.h"

#include "./src/malos.pb.h"
#include "matrix_hal/uv_data.h"

namespace matrix_malos {

bool UVDriver::SendUpdate() {
  matrix_hal::UVData data;
  if (!reader_->Read(&data)) {
    return false;
  }

  UV uv_pb;
  uv_pb.set_uv_index(data.uv);
  // uv_pb.set_oms_risk(oms_risk);

  std::string buffer;
  uv_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

}  // namespace matrix_malos
