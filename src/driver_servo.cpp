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

#include "./driver_servo.h"
#include "./src/driver.pb.h"

const int kGpioOutputMode = 1;
const int kGpioPWMFunction = 1;
const int kGpioPrescaler = 0x5;

const int kServoClockFrequency = 200000000;
const float kServoRatio = 37.7;
const int kServoOffset = 1800;
const float kServoPeriod = 0.02;

namespace matrix_malos {

const bool kServoDriverDebugEnabled = false;

bool ServoDriver::ProcessConfig(const DriverConfig& config) {
  ServoParams servo(config.servo());
  int16_t pin = (int16_t)servo.pin();
  int16_t channel = (int16_t)pin % 4;
  int16_t bank = (int16_t)pin / 4;

  gpio_->SetMode(pin, kGpioOutputMode);
  gpio_->SetFunction(pin, kGpioPWMFunction);
  gpio_->SetPrescaler(bank, kGpioPrescaler);  // set prescaler bank

  uint16_t period_counter = (kServoPeriod * kServoClockFrequency) /
                            ((1 << 5) * 2);  // period for 180 servo type on ms.

  gpio_->Bank(bank).SetPeriod(period_counter);
  int16_t duty_counter = (kServoRatio * servo.angle()) + kServoOffset;
  gpio_->Bank(bank).SetDuty(channel, duty_counter);

  return true;
}

}  // namespace matrix_malos
