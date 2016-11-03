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

#include "./driver_gpio.h"
#include "./src/driver.pb.h"

namespace matrix_malos {

const bool kGpioDriverDebugEnabled = false;

bool GpioDriver::ProcessConfig(const DriverConfig& config) {
  GpioParams gpio_config(config.gpio());
  int16_t pin = static_cast<int16_t>(gpio_config.pin());
  int16_t mode = (int16_t)gpio_config.mode();

  gpio_->SetMode(pin, mode);

  if (mode == GpioParams::INPUT) {
    GpioParams gpiopb;
    gpiopb.set_value(gpio_->GetGPIOValue(pin));
    std::string buffer;
    gpiopb.SerializeToString(&buffer);
    zqm_push_update_->Send(buffer);
  } else if (mode == GpioParams::OUTPUT) {
    int16_t value = gpio_config.value();
    gpio_->SetGPIOValue(pin, value);
  } else {
    zmq_push_error_->Send(
        "invalid gpio mode. check the proto GpioParams (file "
        "driver.proto)");
    return false;
  }

  return true;
}

}  // namespace matrix_malos
