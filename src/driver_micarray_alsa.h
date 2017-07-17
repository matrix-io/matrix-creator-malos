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

#ifndef SRC_DRIVER_MICARRAY_ALSA_H_
#define SRC_DRIVER_MICARRAY_ALSA_H_

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <thread>
#include <mutex>

#include "./malos_wishbone_base.h"

#include "matrix_hal/direction_of_arrival.h"
#include "matrix_hal/microphone_array.h"
#include "matrix_hal/wishbone_bus.h"

const char kMicArrayAlsaDriverName[] = "MicArray_Alsa";

namespace matrix_malos {

class MicArrayAlsaDriver : public MalosWishboneBase {
 public:
  MicArrayAlsaDriver()
      : MalosWishboneBase(kMicArrayAlsaDriverName), doa_(mics_) {
    SetProvidesUpdates(false);
    SetNeedsKeepalives(false);
    SetNotesForHuman(
        "Simple ALSA Driver for MATRIX Creator's Microphone Array");
  }

  // Receive a copy of the shared wishbone bus. Not owned.
  void SetupWishboneBus(matrix_hal::WishboneBus* wishbone) override {
    mics_.Setup(wishbone);

    // alsa thread.
    std::thread alsa_thread(&MicArrayAlsaDriver::AlsaThread, this);
    alsa_thread.detach();
  }

  // Read configuration of Mic Array (from the outside world).
  bool ProcessConfig(const pb::driver::DriverConfig& config) override;

  // Send update to 0MQ zqm_push_update_ queue when called.
  bool SendUpdate() override;

  // Thread that send audio stream to named pipes
  void AlsaThread();

 private:
  // Microphone array driver
  matrix_hal::MicrophoneArray mics_;
  matrix_hal::DirectionOfArrival doa_;
  mutable std::mutex mutex_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_MICARRAY_ALSA_H_
