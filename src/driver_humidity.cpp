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

#include "./driver_humidity.h"
#include "./src/driver.pb.h"
#include "matrix_hal/humidity_data.h" 

namespace matrix_malos {

float CPUTemp(){
  
  FILE *temp_file;
  double temp;
  temp_file = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
  
  if (temp_file == NULL)
    return 0;
  
  fscanf (temp_file, "%lf", &temp);
  fclose (temp_file);
  
  return (float)temp/1000.0;
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

  if(calibrated_){
    float cpu_temp = CPUTemp(); // Get the CPU Temperature
    // Calculate calibrated temperature using the ratio previously calculated 
    float temp_calib = data.temperature - calibration_ratio_ * (cpu_temp - data.temperature);  
    humidity_pb.set_temperature(temp_calib);
  } else{
    humidity_pb.set_temperature(data.temperature);
  }

  std::string buffer;
  humidity_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

bool HumidityDriver::ProcessConfig(const DriverConfig& config) { 
  HumidityParams humidity_params(config.humidity());
  // Check if calibration is needed
  if(!humidity_params.do_calibration())
    return false;
  // Resetting the calibrated flag
  calibrated_ = false;
  // Getting the current temperature to use in the calibration 
  float current_temp = (float)humidity_params.current_temp();
  // Getting temperature data from the humidity sensor
  matrix_hal::HumidityData data;
  if (!reader_->Read(&data)) {
    return false;
  }
  // Getting the CPU temperature 
  float cpu_temp = CPUTemp();
  if(cpu_temp == 0)
    return false;
  // Calculating the ratio for future calibrations
  calibration_ratio_ = (data.temperature - current_temp )/(cpu_temp - data.temperature);
  // Updating the calibrated flag
  calibrated_ = true;
  
  return true;
}

}  // namespace matrix_malos

