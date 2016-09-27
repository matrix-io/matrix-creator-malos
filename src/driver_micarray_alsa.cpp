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

#include "./driver_micarray_alsa.h"

#include "./src/driver.pb.h"
#include "matrix_hal/microphone_array.h"

namespace matrix_malos {

bool MicArrayAlsaDriver::ProcessConfig(const DriverConfig& config) {
  // TODO (andres.calderon@admobilize.com):  handle SetGain
  return true;
}

void MicArrayAlsaDriver::AlsaThread() {
  for (uint16_t c = 0; c < mics_->Channels(); c++) {
    std::string name = "/tmp/matrix_micarray_channel_" + std::to_string(c);

    /* create the FIFO (named pipe) */
    if (mkfifo(name.c_str(), 0666) != 0) {
      /*std::cerr << "unable to create " << name << " FIFO." << std::endl;*/
    }
  }

  int named_pipe_handle;
  std::valarray<int16_t> buffer(mics_->NumberOfSamples());
  while (true) {
    mics_->Read(); /* Reading 8-mics buffer from de FPGA */
    for (uint16_t c = 0; c < mics_->Channels(); c++) {
      std::string name = "/tmp/matrix_micarray_channel_" + std::to_string(c);
      named_pipe_handle = open(name.c_str(), O_WRONLY | O_NONBLOCK);

      for (uint32_t s = 0; s < mics_->NumberOfSamples(); s++)
        buffer[s] = mics_->At(s, c);

      write(named_pipe_handle, &buffer[0],
            sizeof(int16_t) * mics_->NumberOfSamples());
      close(named_pipe_handle);
    }
  }
}

}  // namespace matrix_malos
