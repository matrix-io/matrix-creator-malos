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

#include <chrono>
#include <thread>
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
  // TODO: Validate all the data that comes from the protos
  ZigBeeMsg zigbee_msg(config.zigbee_message());

  std::string command;

  if (zigbee_msg.type() == ZigBeeMsg::ZCL) {
    command = "zcl ";
    if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::ON_OFF) {
      command += "on-off ";
      if (zigbee_msg.zcl_cmd().onoff_cmd().type() ==  ZigBeeMsg::ZCLCmd::OnOffCmd::ON)  {
        command += " on";
      } else if(zigbee_msg.zcl_cmd().onoff_cmd().type() ==  ZigBeeMsg::ZCLCmd::OnOffCmd::OFF){
        command += " off";
      } else if(zigbee_msg.zcl_cmd().onoff_cmd().type() ==  ZigBeeMsg::ZCLCmd::OnOffCmd::TOGGLE){
        command += " toggle";
      }
    } else if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::LEVEL) {
      command += "level-control ";
      if (zigbee_msg.zcl_cmd().level_cmd().type() == ZigBeeMsg::ZCLCmd::LevelCmd::MOVE_TO_LEVEL) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "mv-to-level 0x%02x 0x%04x",
                    zigbee_msg.zcl_cmd().level_cmd().move_to_level_params().level(),
                    zigbee_msg.zcl_cmd().level_cmd().move_to_level_params().transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().level_cmd().type() == ZigBeeMsg::ZCLCmd::LevelCmd::MOVE) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "move 0x%02x 0x%02x",
                    zigbee_msg.zcl_cmd().level_cmd().move_params().mode(),
                    zigbee_msg.zcl_cmd().level_cmd().move_params().rate());
        command += buf;
      }
    } else if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::COLOR_CONTROL) {
      command += "color-control ";
      if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() == ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOHUE) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "movetohue 0x%02x 0x%02x 0x%04x",
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohue_params().hue(),
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohue_params().direction(),
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohue_params().transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() == ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOSAT) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "movetosat 0x%02x 0x%04x",
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetosat_params().saturation(),
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetosat_params().transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() == ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOHUEANDSAT) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "movetohue 0x%02x 0x%02x 0x%04x",
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohueandsat_params().hue(),
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohueandsat_params().saturation(),
                    zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohueandsat_params().transition_time());
        command += buf;
      }
    } else if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::IDENTIFY) {
      command += "identify ";
      if (zigbee_msg.zcl_cmd().identify_cmd().type() == ZigBeeMsg::ZCLCmd::IdentifyCmd::IDENTIFY_ON) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "on 0x%02x 0x%04x",
                    zigbee_msg.zcl_cmd().identify_cmd().identify_on_params().endpoint(),
                    zigbee_msg.zcl_cmd().identify_cmd().identify_on_params().identify_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().identify_cmd().type() == ZigBeeMsg::ZCLCmd::IdentifyCmd::IDENTIFY_OFF) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "off 0x%04x",
                    zigbee_msg.zcl_cmd().identify_cmd().identify_off_params().identify_time());
        command += buf;
      }
    }

  } else if (zigbee_msg.type() == ZigBeeMsg::ZLL) {

  } else if (zigbee_msg.type() == ZigBeeMsg::NETWORK_MGMT) {

    if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::CREATE_NWK) {
      command = "network find unused";
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::LEAVE_NWK) {
      command = "network leave";
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::NODE_LEAVE_NWK) {
      char buf[128];
      std::snprintf(buf, sizeof buf, "zdo leave  0x%04x 0 0",
                    zigbee_msg.network_mgmt_cmd().node_leave_params().node_id());
      command = buf;
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::PERMIT_JOIN) {
      char buf[128];
      std::snprintf(buf, sizeof buf, "network pjoin %3d",
                    zigbee_msg.network_mgmt_cmd().permit_join_params().time());
      command = buf;
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::NODE_INFO) {
      command = "info";
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::DISCOVERY_INFO) {
      command = "plugin device-database print-all";
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::RESET_PROXY) {
      
      // Kill the ZigBeeGateway app 
      system("sudo pkill ZigBeeGateway");

      // Starting the app
      system("sudo /usr/share/admobilize/matrix-creator/blob/ZigBeeGateway -n 1 -p ttyS0 -v &");

      int i = 0;
      const int count = 10;
      for (i = 0; i < count; ++i)
      {
        // Sleep 0.5 sec to wait for 
        std::this_thread::sleep_for(
          std::chrono::milliseconds(500));
    
        tcp_client_.reset(new TcpClient());
        if (tcp_client_->Connect(gateway_ip, gateway_port)) {
          std::cerr << "Couldn't connect to ZigBee Gateway at " << gateway_ip << ":" << std::to_string(gateway_port) << std::endl << std::flush;
          break;
        } else {
          if (i == count - 1){
            std::cerr << "NOT connected." << std::endl;
            zmq_push_error_->Send("Couldn't connect to ZigBee Gateway at " +
              gateway_ip + ":" + std::to_string(gateway_port));
          } else {
            std::cerr << "Trying to connect with ZigBee Gateway..." << std::endl;
            zmq_push_error_->Send("Trying to connect with ZigBee Gateway at " +
              gateway_ip + ":" + std::to_string(gateway_port));
          }
        }
      }

      std::cerr.flush();

    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::IS_PROXY_ACTIVE) {
      /* Check if ZigBeeGateway is running*/
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::NETWORK_STATUS) {
      command = "info";
    }
  }

  // Sending the messsage 
  if (tcp_client_.get() == nullptr) {
    zmq_push_error_->Send("Gateway app not connected.");
  }
  else{
    tcp_client_->Send(command + "\n");
  }

  return true;
}

// const char AnnounceLine[] = "Device Announce: ";
const char NetworkStateLine[] = "network state";


bool ZigbeeBulbDriver::SendUpdate() {
  std::string line;
  while (tcp_client_->GetLine(&line)) { 
    
    line = Trim(line);
    
    // Detecting NetworkStateLine in the line
    std::size_t found = line.find(NetworkStateLine);
    if (found != std::string::npos){

      int network_type = stoi(line.substr(found + sizeof NetworkStateLine + 1 , 2),0,10);

      ZigBeeMsg zigbee_msg; 
      zigbee_msg.set_type(ZigBeeMsg::NETWORK_MGMT);
      zigbee_msg.network_mgmt_cmd().set_type(ZigBeeMsg::NetworkMgmtCmd::CREATE_NWK);

      // switch (network_type)
      // {
      //   // NO_NETWORK
      //   case 0: zigbee_msg.network_mgmt_cmd().set_network_status(
      //     ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::NO_NETWORK);
      //   break;
      //   // JOINING_NETWORK 
      //   case 1:zigbee_msg.network_mgmt_cmd().set_network_status(
      //     ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINING_NETWORK);
      //   break;
      //   // JOINED_NETWORK 
      //   case 2: zigbee_msg.network_mgmt_cmd().set_network_status(
      //     ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINED_NETWORK);
      //   break;
      //   // JOINED_NETWORK_NO_PARENT 
      //   case 3: zigbee_msg.network_mgmt_cmd().set_network_status(
      //     ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINED_NETWORK_NO_PARENT);
      //   break;
      //   // LEAVING_NETWORK 
      //   case 4: zigbee_msg.network_mgmt_cmd().set_network_status(
      //     ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::LEAVING_NETWORK);
      //   break;
        
      // }

      //  // Send the serialized proto.
      // std::string buffer;
      // zigbee_msg.SerializeToString(&buffer);
      // zqm_push_update_->Send(buffer);
      

    } else {
    }
  }
  return true;
}

}  // namespace matrix_malos
