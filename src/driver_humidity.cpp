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

// Empirical fit of saturated vapor density versus Celsius Temperature from : http://hyperphysics.phy-astr.gsu.edu/hbase/kinetic/relhum.html#c3
float SatVaporDensityfromTemp(float T){
  return  5.018       +  
          0.32321 * 1      * T +
          8.1847  * 0.001  * T * T +
          3.1243  * 0.0001 * T * T * T;
}

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

  float cpu_temp = CPUTemp(); // Get the CPU Temperature
  
  // Calculate calibrated temperature
  float temp_calib = calibration_ratio_ * (cpu_temp - data.temperature); 

  // Calculate calibrated humidity using the calibrated temperature: 
  // from http://hyperphysics.phy-astr.gsu.edu/hbase/kinetic/relhum.html#c4
  float hum_calib = SatVaporDensityfromTemp(data.temperature)/SatVaporDensityfromTemp(temp_calib) * data.humidity;

  Humidity humidity_pb;
  humidity_pb.set_humidity(data.humidity);
  humidity_pb.set_temperature(data.temperature);
  humidity_pb.set_humidity_calib(hum_calib);
  humidity_pb.set_temperature_calib(temp_calib);

  std::string buffer;
  humidity_pb.SerializeToString(&buffer);
  zqm_push_update_->Send(buffer);

  return true;
}

bool HumidityDriver::ProcessConfig(const DriverConfig& config) {
  
  TemperatureCalibParams temp_calib(config.temp_calib());
  float current_temp = (int16_t)temp_calib.current_temp();

  // Getting temperature data from the humidity sensor
  matrix_hal::HumidityData data;
  if (!reader_->Read(&data)) {
    return false;
  }

  // Getting the CPU temperature
  float cpu_temp = CPUTemp();
  if(cpu_temp == 0)
    return false;

  calibration_ratio_ = (data.temperature - current_temp )/(cpu_temp - data.temperature);

  return true;
}

}  // namespace matrix_malos

