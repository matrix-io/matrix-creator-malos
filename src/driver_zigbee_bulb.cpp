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

// using matrix_malos::ZigBeeMsg;

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
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::DISCOVERY_INFO) {
      command = "plugin device-database print-all";
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::RESET_PROXY) {
      
      // Kill the ZigBeeGateway app 
      system("sudo pkill ZigBeeGateway"); 

      // Starting the app
      system("sudo /usr/share/admobilize/matrix-creator/blob/ZigBeeGateway -n 1 -p ttyS0 -v &");

      int i = 0;
      const int count = 10; 
      tcp_client_.reset(new TcpClient());

      std::cerr << "Trying to connect with the Gateway ...." << std::endl;
      zmq_push_error_->Send("Trying to connect with the Gateway ....");
      
      for (i = 0; i < count; ++i)
      {
        // Sleep 0.5 sec to wait for 
        std::this_thread::sleep_for(
          std::chrono::milliseconds(500));
    
        if (tcp_client_->Connect(gateway_ip, gateway_port)) {
          std::cerr << "Connected to the Gateway." << std::endl;
          zmq_push_error_->Send("Connected to the Gateway.");
          break;
        } else {
          if (i == count - 1){
            std::cerr << "No connection to the Gateway" << std::endl << std::flush;
            zmq_push_error_->Send("No connection to the Gateway");
          }
          std::cerr << "Trying to connect with the Gateway ...." << std::endl;
          zmq_push_error_->Send("Trying to connect with the Gateway ....");
        }
      }

      std::cerr.flush();

    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::IS_PROXY_ACTIVE) {
      command = ""; // No need to send message to Gateway
      if( tcp_client_->GetErrorMessage().compare("Connected") == 0) { 
        zigbee_msg.mutable_network_mgmt_cmd()->set_is_proxy_active(true);
      }else{
        zigbee_msg.mutable_network_mgmt_cmd()->set_is_proxy_active(false);
      }

      std::string buffer;
      zigbee_msg.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);
      
    } else if (zigbee_msg.network_mgmt_cmd().type() == ZigBeeMsg::NetworkMgmtCmd::NETWORK_STATUS) {
      command = "info";
    }
  }

  // Sending the messsage 
  if (command != ""){
    if (tcp_client_.get() != nullptr) { 
      tcp_client_->Send(command + "\n");
    }
  }

  return true;
}

ZigBeeMsg zigbee_msg; 
bool bulding_discovery_result = false;


bool ZigbeeBulbDriver::SendUpdate() {
  std::string line;
  while (tcp_client_->GetLine(&line)) { 
    
    line = Trim(line);
    std::cerr << ">>>" <<line << std::endl;

    const char network_state_line[] = "network state";
    std::size_t found = line.find(network_state_line);
    if (found != std::string::npos){

      int network_type = stoi(line.substr(found + sizeof network_state_line + 1 , 2),0,10);

      zigbee_msg.set_type(ZigBeeMsg::NETWORK_MGMT);
      zigbee_msg.mutable_network_mgmt_cmd()->set_type(
        ZigBeeMsg::NetworkMgmtCmd::NETWORK_STATUS);

      switch (network_type)
      {
        // NO_NETWORK
        case 0: zigbee_msg.mutable_network_mgmt_cmd()->mutable_network_status()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::NO_NETWORK);
        break;
        // JOINING_NETWORK 
        case 1:zigbee_msg.mutable_network_mgmt_cmd()->mutable_network_status()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINING_NETWORK);
        break;
        // JOINED_NETWORK 
        case 2: zigbee_msg.mutable_network_mgmt_cmd()->mutable_network_status()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINED_NETWORK);
        break;
        // JOINED_NETWORK_NO_PARENT 
        case 3: zigbee_msg.mutable_network_mgmt_cmd()->mutable_network_status()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINED_NETWORK_NO_PARENT);
        break;
        // LEAVING_NETWORK 
        case 4: zigbee_msg.mutable_network_mgmt_cmd()->mutable_network_status()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::LEAVING_NETWORK);
        break;
      }
       // Send the serialized proto.
      std::string buffer;
      zigbee_msg.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);
      zmq_push_error_->Send(buffer);

      continue;
    } 

    const char discovery_start_line[] = "Discovery Database";
    found = line.find(discovery_start_line);
    if (found != std::string::npos){
      bulding_discovery_result =  true;

      zigbee_msg.set_type(ZigBeeMsg::NETWORK_MGMT);
      zigbee_msg.mutable_network_mgmt_cmd()->set_type(
        ZigBeeMsg::NetworkMgmtCmd::DISCOVERY_INFO);
      zigbee_msg.mutable_network_mgmt_cmd()->clear_connected_nodes();

      continue;
    }

    const char discovery_end_line[] = "devices in database";
    found = line.find(discovery_end_line);
    if (found != std::string::npos) {
      bulding_discovery_result =  false;
      continue;
    }

    const char node_id_line[] = "NodeId";
    found = line.find(node_id_line);
    if (found != std::string::npos && bulding_discovery_result) {
      int node_id = stoi(line.substr(found + sizeof node_id_line + 1, 6), 0, 16);
      ZigBeeMsg::NetworkMgmtCmd::NodeDescription* new_node =  
        zigbee_msg.mutable_network_mgmt_cmd()->add_connected_nodes();
      new_node->set_nodeid(node_id);
      continue;
    }

    const char euid64_line[] = "Eui64";
    found = line.find(euid64_line);
    if (found != std::string::npos && bulding_discovery_result){
      std::string euid64_str = line.substr(found + sizeof euid64_line + 1, 18);
      unsigned long long euid64 = strtoull(euid64_str.c_str(), 0, 16);
      if (zigbee_msg.mutable_network_mgmt_cmd()->connected_nodes_size() > 0) {
          zigbee_msg.mutable_network_mgmt_cmd()->mutable_connected_nodes(
            zigbee_msg.mutable_network_mgmt_cmd()->connected_nodes_size()-1)->set_eui64(euid64);
      }
      continue;
    }



  }
  return true;
}

}  // namespace matrix_malos

