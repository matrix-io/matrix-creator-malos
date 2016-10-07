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
#include "matrix_hal/everloop_image.h"
#include "matrix_hal/gpio_control.h"

#define INPUT 0
#define OUTPUT 1
#define PWM 1
#define CLK_FRQ 200000000

namespace matrix_malos {

const bool kServoDriverDebugEnabled = false;

bool ServoDriver::ProcessConfig(const DriverConfig& config) {

  ServoParams servo(config.servo());

  matrix_hal::WishboneBus bus;
  bus.SpiInit();

  matrix_hal::GPIOControl gpio;
  gpio.Setup(&bus);

  int16_t pin      = (int16_t)servo.pin();
  int16_t channel  = (int16_t)pin%4;
  int16_t bank     = (int16_t)pin/4;

  gpio.SetMode(pin, OUTPUT);     // pin output mode 
  gpio.SetFunction(pin, PWM);    // pin 4, PWM output */
  gpio.SetPrescaler(bank, 0x5);  // set prescaler bank 0 */

  uint16_t period_counter = (0.02 * CLK_FRQ) / ((1 << 5) * 2);
  int16_t duty_counter = 0;

  gpio.Bank(bank).SetPeriod(period_counter);
  duty_counter = (37.7 * servo.angle()) + 1800;
  std::cout << " Servo angle  : " << servo.angle() << "\t";
  std::cout << " Duty counter : " << duty_counter  << "\n";
  gpio.Bank(bank).SetDuty(channel, duty_counter);

  return true;

}

}  // namespace matrix_malos
