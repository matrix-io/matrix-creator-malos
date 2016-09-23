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

#include "./driver_zigbee_bulb.h"

#include "./src/driver.pb.h"

namespace {

// http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
std::string Trim(const std::string& s) {
  auto wsfront = std::find_if_not(s.begin(), s.end(),
                                  [](int c) { return std::isspace(c); });
  return std::string(
      wsfront,
      std::find_if_not(s.rbegin(), std::string::const_reverse_iterator(wsfront),
                       [](int c) { return std::isspace(c); })
          .base());
}

}  // namespace

namespace matrix_malos {

bool ZigbeeBulbDriver::ProcessConfig(const DriverConfig& config) {
  ZigbeeBulbConfig bulb_config(config.zigbee_bulb());

  if (bulb_config.address() == "" && bulb_config.port() == -1) {
    std::cout << "ZigbeeBulb got command" << std::endl;
    std::cout << "ZigbeeBulb id: " << bulb_config.command().short_id()
              << std::endl;
    std::cout << "ZigbeeBulb cmd: " << bulb_config.command().command()
              << std::endl;

    if (bulb_config.command().command() == ZigBeeBulbCmd::OFF) {
      tcp_client_->Send("zcl on-off off\n");
      char buf[128];
      sprintf(buf, "send 0x%04x 0 1\n", bulb_config.command().short_id());
      tcp_client_->Send(buf);
    }

    return true;
  }

  std::cout << "ZigbeeBulb Got configuration" << std::endl;
  std::cout << "Connect to " << bulb_config.address() << ":"
            << bulb_config.port() << std::endl;

  tcp_client_.reset(new TcpClient());
  if (tcp_client_->Connect(bulb_config.address(), bulb_config.port())) {
    std::cout << "connected" << std::endl << std::flush;
  } else {
    std::cout << "NOT connected" << std::endl;
    return false;
  }

  std::cout.flush();

  return true;
}

const char AnnounceLine[] = "Device Announce: ";

bool ZigbeeBulbDriver::SendUpdate() {
  std::string line;
  if (tcp_client_->GetLine(&line)) {
    line = Trim(line);
    std::cout << "ZigBee: " << line << std::endl;
    std::cout.flush();
    // Check if the line countains an announcement.
    if (line.size() > sizeof AnnounceLine - 1 &&
        line.compare(0, sizeof AnnounceLine - 1, AnnounceLine) == 0) {
      line.erase(0, sizeof AnnounceLine - 1);
      std::cout << "ZigbeeBulbDriver received announce for '" << line << "'"
                << std::endl;
      // Fill out protocol buffer.
      ZigBeeAnnounce announce_pb;
      int device_id = stoi(line, 0, 16);
      std::cout << line << " => " << device_id << std::endl;
      announce_pb.set_short_id(device_id);
      // Send the serialized proto.
      std::string buffer;
      announce_pb.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);
    }
  }
  // Device Announce: 0x17AA
  return true;
}

}  // namespace matrix_malos
