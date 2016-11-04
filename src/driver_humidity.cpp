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
#include <fstream>
#include <iostream>
#include "./driver_humidity.h"
#include "./src/driver.pb.h"
#include "matrix_hal/humidity_data.h"

namespace matrix_malos {

const char kDefaultCpuTemperatureFileName[] =
    "/sys/class/thermal/thermal_zone0/temp";

bool ApproximatelyEqual(float a, float b, float epsilon) {
  return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

bool ReadCpuTemperature(const std::string &file_name, float *temperature) {
  std::ifstream input_file(file_name);

  if (!input_file.is_open()) return false;

  float cpu_temperature = 0;
  input_file >> cpu_temperature;
  // Maybe we should check that the cpu_temperature makes sense?
  *temperature = static_cast<float>(cpu_temperature) / 1000.0f;

  return true;
}

bool HumidityDriver::SendUpdate() {
  matrix_hal::HumidityData data;
  if (!reader_->Read(&data)) {
    return false;
  }
  Humidity humidity_pb;

  humidity_pb.set_temperature_is_calibrated(calibrated_);
  humidity_pb.set_humidity(data.humidity);
  humidity_pb.set_temperature_raw(data.temperature);

  float cpu_temperature = 0;
  if (!ReadCpuTemperature(kDefaultCpuTemperatureFileName, &cpu_temperature))
    return false;

  if (calibrated_) {
    // Calculate calibrated temperature using the ratio previously calculated
    const float temp_calib =
        data.temperature -
        calibration_ratio_ * (cpu_temperature - data.temperature);
    humidity_pb.set_temperature(temp_calib);
  } else {
    humidity_pb.set_temperature(data.temperature);
  }

  std::string buffer;
  humidity_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

bool HumidityDriver::ProcessConfig(const DriverConfig &config) {
  HumidityParams humidity_params(config.humidity());

  // Check if calibration is needed
  if (!humidity_params.do_calibration()) return false;

  // Resetting the calibrated flag
  calibrated_ = false;

  // Getting temperature data from the humidity sensor
  matrix_hal::HumidityData data;
  if (!reader_->Read(&data)) return false;
  float sensor_temperature = data.temperature;

  // Getting the CPU temperature
  float cpu_temperature = 0;
  if (!ReadCpuTemperature(kDefaultCpuTemperatureFileName, &cpu_temperature))
    return false;

  // Getting the current temperature to use in the calibration
  const float current_temperature =
      static_cast<float>(humidity_params.current_temperature());

  // Check to avoid devide by zero
  if (ApproximatelyEqual(sensor_temperature, cpu_temperature, 0.1)) {
    sensor_temperature = cpu_temperature - 0.1;
  }
  // Calculating the ratio value to use when calibrating the temperature value
  calibration_ratio_ = (sensor_temperature - current_temperature) /
                       (cpu_temperature - sensor_temperature);

  // Updating the calibrated flag
  calibrated_ = true;

  return true;
}

}  // namespace matrix_malos
