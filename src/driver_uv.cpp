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

#include "./driver_uv.h"
#include <matrix_io/malos/v1/driver.pb.h>
#include <string>
#include "matrix_hal/uv_data.h"

namespace pb = matrix_io::malos::v1;

namespace matrix_malos {

bool UVDriver::SendUpdate() {
  matrix_hal::UVData data;
  if (!reader_->Read(&data)) {
    return false;
  }

  pb::sense::UV uv_pb;
  uv_pb.set_uv_index(data.uv);
  std::string oms_risk;
  if (data.uv < 3.0) {
    oms_risk = "Low";
  } else if (data.uv < 6.0) {
    oms_risk = "Moderate";
  } else if (data.uv < 8.0) {
    oms_risk = "High";
  } else if (data.uv < 11.0) {
    oms_risk = "Very High";
  } else {
    oms_risk = "Extreme";
  }
  uv_pb.set_oms_risk(oms_risk);

  std::string buffer;
  uv_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

}  // namespace matrix_malos
