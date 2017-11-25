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

#include <string>

#include "./driver_micarray_alsa.h"
#include <matrix_io/malos/v1/driver.pb.h>
#include "matrix_hal/microphone_array.h"

namespace pb = matrix_io::malos::v1;

namespace matrix_malos {

bool MicArrayAlsaDriver::ProcessConfig(const pb::driver::DriverConfig& config) {
  pb::io::MicArrayParams micarray_config(config.micarray());

  mics_.SetGain(static_cast<int16_t>(micarray_config.gain()));

  mics_.CalculateDelays(micarray_config.azimutal_angle(),
                        micarray_config.polar_angle(),
                        micarray_config.radial_distance_mm(),
                        micarray_config.sound_speed_mmseg());

  mics_.SetSamplingRate(micarray_config.sampling_frequency_hz());

  return true;
}

bool MicArrayAlsaDriver::SendUpdate() {
  pb::io::MicArrayParams mics_params;

  mics_params.set_azimutal_angle(doa_.GetAzimutalAngle());
  mics_params.set_polar_angle(doa_.GetPolarAngle());

  mics_params.set_sampling_frequency_hz(mics_.SamplingRate());
  mics_params.set_gain(mics_.Gain());

  std::string buffer;
  mics_params.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

void MicArrayAlsaDriver::AlsaThread() {
  // building fifo for each channel + fifo for the beamformed channel
  for (uint16_t c = 0; c < mics_.Channels() + 1; ++c) {
    const std::string name =
        "/tmp/matrix_micarray_channel_" + std::to_string(c);

    /* create the FIFO (named pipe) */
    if (mkfifo(name.c_str(), 0666) != 0) {
      if (errno == EEXIST) {
        continue;
      } else {
        std::cerr << "Unable to create " << name
                  << " fifo (ERROR:" << strerror(errno) << ")" << std::endl;
        return;
      }
    }
  }
  doa_.Init();
  int named_pipe_handle;
  std::valarray<int16_t> buffer(mics_.NumberOfSamples());
  while (true) {

    mics_.Read(); /* Reading 8-mics buffer from de FPGA */

    doa_.Calculate();

    for (uint16_t c = 0; c < mics_.Channels(); ++c) {
      const std::string name =
          "/tmp/matrix_micarray_channel_" + std::to_string(c);
      // TODO(andres.calderon@admobilize.com):  handle error
      named_pipe_handle = open(name.c_str(), O_WRONLY | O_NONBLOCK);

      for (uint32_t s = 0; s < mics_.NumberOfSamples(); ++s)
        buffer[s] = mics_.At(s, c);

      // TODO(andres.calderon@admobilize.com):  handle error
      write(named_pipe_handle, &buffer[0],
            sizeof(int16_t) * mics_.NumberOfSamples());

      close(named_pipe_handle);
    }

    // Write to pipe beamformed channel
    const std::string name =
        "/tmp/matrix_micarray_channel_" + std::to_string(mics_.Channels());
    // TODO(andres.calderon@admobilize.com):  handle error
    named_pipe_handle = open(name.c_str(), O_WRONLY | O_NONBLOCK);

    for (uint32_t s = 0; s < mics_.NumberOfSamples(); ++s)
      buffer[s] = mics_.Beam(s);

    // TODO(andres.calderon@admobilize.com):  handle error
    write(named_pipe_handle, &buffer[0],
          sizeof(int16_t) * mics_.NumberOfSamples());

    close(named_pipe_handle);
  }
}

}  // namespace matrix_malos
