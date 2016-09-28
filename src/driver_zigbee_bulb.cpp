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

  // When address is empty and port is -1 we got a command.
  if (bulb_config.address() == "" && bulb_config.port() == -1) {
    if (tcp_client_.get() == nullptr) {
      zmq_push_error_->Send(
          "ZigBee bulb driver hasn't been configured. Did you restart MALOS?");
      return false;
    }

    std::cerr << "ZigbeeBulb got command" << std::endl;
    std::cerr << "ZigbeeBulb id: " << bulb_config.command().short_id()
              << std::endl;
    std::cerr << "ZigbeeBulb cmd: " << bulb_config.command().command()
              << std::endl;

    // TODO(nelson.castillo): Improve readability of this function.

    // Only send the command if no errors are found.
    std::string command;

    if (bulb_config.command().command() == ZigBeeBulbCmd::OFF) {
      command = "zcl on-off off";
    } else if (bulb_config.command().command() == ZigBeeBulbCmd::ON) {
      command = "zcl on-off on";
    } else if (bulb_config.command().command() == ZigBeeBulbCmd::TOGGLE) {
      command = "zcl on-off toggle";
    } else if (bulb_config.command().command() == ZigBeeBulbCmd::IDENTIFY) {
      command = "zcl identify id 3";
    } else if (bulb_config.command().command() == ZigBeeBulbCmd::LEVEL) {
      if (bulb_config.command().params_size() != 2) {
        zmq_push_error_->Send(
            "Invalid number of parameters for LEVEL command.  Two"
            "parameters are required: level and transition time.");
        return false;
      }
      if (bulb_config.command().params(0) > 255) {
        zmq_push_error_->Send(
            "Invalid level for LEVEL command. level needs values between 0 and "
            "255");
        return false;
      }
      if (bulb_config.command().params(1) > 65535) {
        zmq_push_error_->Send(
            "Invalid transition time for LEVEL command. Transition time needs "
            "values between 0 and 65535");
        return false;
      }
      // TODO(nelson.castillo): Do not allocate in the stack.
      char buf[128];
      std::snprintf(buf, sizeof buf, "zcl level-control mv-to-level %d %d",
                    bulb_config.command().params(0),
                    bulb_config.command().params(1));
      command = buf;
    } else if (bulb_config.command().command() == ZigBeeBulbCmd::COLOR) {
      if (bulb_config.command().params_size() != 3) {
        zmq_push_error_->Send(
            "Invalid number of parameters for COLOR command.  Three"
            "parameters are required: hue, saturation and transition time.");
        return false;
      }
      if (bulb_config.command().params(0) > 255) {
        zmq_push_error_->Send(
            "Invalid hue for COLOR command. Hue needs values between 0 and "
            "255");
        return false;
      }
      if (bulb_config.command().params(1) > 255) {
        zmq_push_error_->Send(
            "Invalid saturation for COLOR command. Saturation needs values "
            "between 0 and "
            "255");
        return false;
      }
      if (bulb_config.command().params(2) > 65535) {
        zmq_push_error_->Send(
            "Invalid transition time for COLOR command. Transition time needs "
            "values between 0 and 65535");
        return false;
      }
      // TODO(nelson.castillo): Do not allocate in the stack.
      char buf[128];
      std::snprintf(
          buf, sizeof buf, "zcl color-control movetohueandsat %d %d %d",
          bulb_config.command().params(0), bulb_config.command().params(1),
          bulb_config.command().params(2));
      command = buf;
    } else {
      zmq_push_error_->Send(
          "invalid  command. check the proto zigbeebulbcmd (file "
          "driver.proto)");
      return false;
    }
    // TODO(nelson.castillo): Do not allocate in the stack.
    char buf[128];
    if (bulb_config.command().endpoint() == 0) {
      // Generic bulb.
      std::snprintf(buf, sizeof buf, "send 0x%04x 0 1\n",
                    bulb_config.command().short_id());
    } else if (bulb_config.command().endpoint() == 0xb) {
      // Philips bulb. We haven't figured out how to use the endpoints yet.
      std::snprintf(buf, sizeof buf, "send 0x%04x 0 0xb\n",
                    bulb_config.command().short_id());
    } else {
      zmq_push_error_->Send(
          "Invalid  endpoint. We only use 0x0 and 0xb. Should we use more? Let "
          "us know! The enpoint:" +
          std::to_string(bulb_config.command().endpoint()));
      return false;
    }

    tcp_client_->Send(command + "\n");
    tcp_client_->Send(buf);

    return true;
  }

  // Otherwise a new connection is established.

  std::cerr << "ZigbeeBulb Got configuration" << std::endl;
  std::cerr << "Connect to " << bulb_config.address() << ":"
            << bulb_config.port() << std::endl;

  tcp_client_.reset(new TcpClient());
  if (tcp_client_->Connect(bulb_config.address(), bulb_config.port())) {
    std::cerr << "connected" << std::endl << std::flush;
  } else {
    std::cerr << "NOT connected" << std::endl;
    zmq_push_error_->Send("Could not connect to ZigBee gateway at " +
                          bulb_config.address() + ":" +
                          std::to_string(bulb_config.port()));
    return false;
  }

  std::cerr.flush();

  return true;
}

const char AnnounceLine[] = "Device Announce: ";

bool ZigbeeBulbDriver::SendUpdate() {
  std::string line;
  while (tcp_client_->GetLine(&line)) {
    line = Trim(line);
    std::cerr << "ZigBee: " << line << std::endl;
    std::cerr.flush();
    // Check if the line countains an announcement.
    if (line.size() > sizeof AnnounceLine - 1 &&
        line.compare(0, sizeof AnnounceLine - 1, AnnounceLine) == 0) {
      line.erase(0, sizeof AnnounceLine - 1);
      std::cerr << "ZigbeeBulbDriver received announce for '" << line << "'"
                << std::endl;
      // Fill out protocol buffer.
      ZigBeeAnnounce announce_pb;
      int device_id = stoi(line, 0, 16);
      std::cerr << line << " => " << device_id << std::endl;
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
