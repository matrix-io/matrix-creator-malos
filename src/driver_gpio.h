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

#ifndef SRC_DRIVER_GPIO_H_
#define SRC_DRIVER_GPIO_H_

#include <memory>

#include "./malos_wishbone_base.h"
#include "matrix_hal/wishbone_bus.h"
#include "matrix_hal/gpio_control.h"

const char kGpioDriverName[] = "Gpio";

namespace matrix_malos {

class GpioDriver : public MalosWishboneBase {
 public:
  GpioDriver() : MalosWishboneBase(kGpioDriverName) {
    SetProvidesUpdates(false);
    SetNeedsKeepalives(false);
    SetNotesForHuman("Write-read. Gpio handler. In development");
  }

  // Load Gpio control for read value or write it
  // in the first case a value is returned in the update channel.
  void SetupWishboneBus(matrix_hal::WishboneBus* wishbone) override {
    gpio_.reset(new matrix_hal::GPIOControl);
    gpio_->Setup(wishbone);
  }

  // Read configuration from GpioParams (from the outside world).
  bool ProcessConfig(const DriverConfig& config) override;

 private:
  // GPIO control
  std::unique_ptr<matrix_hal::GPIOControl> gpio_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_GPIO_H_
