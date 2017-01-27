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

#include "./driver_zigbee.h"

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

bool ZigbeeDriver::ProcessConfig(const DriverConfig& config) {
  // TODO: Validate all the data that comes from the protos
  ZigBeeMsg zigbee_msg(config.zigbee_message());

  std::string command;
  bool command_error = false;

  if (zigbee_msg.type() == ZigBeeMsg::ZCL) {
    command = "zcl ";

    if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::ON_OFF) {
      command += "on-off ";
      if (zigbee_msg.zcl_cmd().onoff_cmd().type() ==
          ZigBeeMsg::ZCLCmd::OnOffCmd::ON) {
        command += " on";
      } else if (zigbee_msg.zcl_cmd().onoff_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::OnOffCmd::OFF) {
        command += " off";
      } else if (zigbee_msg.zcl_cmd().onoff_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::OnOffCmd::TOGGLE) {
        command += " toggle";
      } else {
        command_error = true;
      }
    } else if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::LEVEL) {
      command += "level-control ";
      if (zigbee_msg.zcl_cmd().level_cmd().type() ==
          ZigBeeMsg::ZCLCmd::LevelCmd::MOVE_TO_LEVEL) {
        char buf[128];
        std::snprintf(
            buf, sizeof buf, "mv-to-level 0x%02X 0x%04X",
            zigbee_msg.zcl_cmd().level_cmd().move_to_level_params().level(),
            zigbee_msg.zcl_cmd()
                .level_cmd()
                .move_to_level_params()
                .transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().level_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::LevelCmd::MOVE) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "move 0x%02X 0x%02X",
                      zigbee_msg.zcl_cmd().level_cmd().move_params().mode(),
                      zigbee_msg.zcl_cmd().level_cmd().move_params().rate());
        command += buf;
      } else {
        command_error = true;
      }
    } else if (zigbee_msg.zcl_cmd().type() ==
               ZigBeeMsg::ZCLCmd::COLOR_CONTROL) {
      command += "color-control ";
      if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() ==
          ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOHUE) {
        char buf[128];
        std::snprintf(
            buf, sizeof buf, "movetohue 0x%02X 0x%02X 0x%04X",
            zigbee_msg.zcl_cmd().colorcontrol_cmd().movetohue_params().hue(),
            zigbee_msg.zcl_cmd()
                .colorcontrol_cmd()
                .movetohue_params()
                .direction(),
            zigbee_msg.zcl_cmd()
                .colorcontrol_cmd()
                .movetohue_params()
                .transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOSAT) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "movetosat 0x%02X 0x%04X",
                      zigbee_msg.zcl_cmd()
                          .colorcontrol_cmd()
                          .movetosat_params()
                          .saturation(),
                      zigbee_msg.zcl_cmd()
                          .colorcontrol_cmd()
                          .movetosat_params()
                          .transition_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().colorcontrol_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::ColorControlCmd::MOVETOHUEANDSAT) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "movetohueandsat 0x%02X 0x%02X 0x%04X",
                      zigbee_msg.zcl_cmd()
                          .colorcontrol_cmd()
                          .movetohueandsat_params()
                          .hue(),
                      zigbee_msg.zcl_cmd()
                          .colorcontrol_cmd()
                          .movetohueandsat_params()
                          .saturation(),
                      zigbee_msg.zcl_cmd()
                          .colorcontrol_cmd()
                          .movetohueandsat_params()
                          .transition_time());
        command += buf;
      } else {
        command_error = true;
      }
    } else if (zigbee_msg.zcl_cmd().type() == ZigBeeMsg::ZCLCmd::IDENTIFY) {
      command += "identify ";
      if (zigbee_msg.zcl_cmd().identify_cmd().type() ==
          ZigBeeMsg::ZCLCmd::IdentifyCmd::IDENTIFY_ON) {
        char buf[128];
        std::snprintf(
            buf, sizeof buf, "on 0x%02X 0x%04X",
            zigbee_msg.zcl_cmd().identify_cmd().identify_on_params().endpoint(),
            zigbee_msg.zcl_cmd()
                .identify_cmd()
                .identify_on_params()
                .identify_time());
        command += buf;
      } else if (zigbee_msg.zcl_cmd().identify_cmd().type() ==
                 ZigBeeMsg::ZCLCmd::IdentifyCmd::IDENTIFY_OFF) {
        char buf[128];
        std::snprintf(buf, sizeof buf, "off 0x%04X", zigbee_msg.zcl_cmd()
                                                         .identify_cmd()
                                                         .identify_off_params()
                                                         .identify_time());
        command += buf;
      } else {
        command_error = true;
      }
    } else {
      command_error = true;
    }

    if (command_error) {
      std::cerr << "zcl command error" << std::endl;
      zmq_push_error_->Send("zcl command error");
      return false;
    } else if (tcp_client_.get() != nullptr) {  // Sending the messsage
      tcp_client_->Send(command + "\n");
      char buf[128];
      std::snprintf(buf, sizeof buf, "send 0x%04X 0 0x%02X\n",
                    zigbee_msg.zcl_cmd().node_id(),
                    zigbee_msg.zcl_cmd().endpoint_index());
      command = buf;
      tcp_client_->Send(command);
    }
  } else if (zigbee_msg.type() == ZigBeeMsg::ZLL) {
    std::cerr << "zzl command not implemented" << std::endl;
    zmq_push_error_->Send("zzl command not implemented");
    return false;
  } else if (zigbee_msg.type() == ZigBeeMsg::NETWORK_MGMT) {
    if (zigbee_msg.network_mgmt_cmd().type() ==
        ZigBeeMsg::NetworkMgmtCmd::CREATE_NWK) {
      command = "network find unused";
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::LEAVE_NWK) {
      command = "network leave";
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::NODE_LEAVE_NWK) {
      char buf[128];
      std::snprintf(
          buf, sizeof buf, "zdo leave  0x%04X 0 0",
          zigbee_msg.network_mgmt_cmd().node_leave_params().node_id());
      command = buf;
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::PERMIT_JOIN) {
      char buf[128];
      std::snprintf(buf, sizeof buf, "network pjoin %d",
                    zigbee_msg.network_mgmt_cmd().permit_join_params().time());
      command = buf;
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::DISCOVERY_INFO) {
      command = "plugin device-database print-all";
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::RESET_PROXY) {
      // Kill the ZigBeeGateway app
      system("sudo pkill ZigBeeGateway");

      // Starting the app
      system(
          "sudo /usr/share/admobilize/matrix-creator/blob/ZigBeeGateway -n 1 "
          "-p ttyS0 -v &");

      tcp_client_.reset(new TcpClient());

      std::cerr << "Connecting to the ZigbeeGateway" << std::endl;
      zmq_push_error_->Send("Connecting to the ZigbeeGateway");

      // Wait a little bit for the ZigbeeGateway to start
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      if (tcp_client_->Connect(gateway_ip, gateway_port)) {
        std::cerr << "Connected to the Gateway" << std::endl;
        zmq_push_error_->Send("Connected to the Gateway");
        return true;
      } else {
        std::cerr << "Can not connect to the ZigbeeGateway" << std::endl
                  << std::flush;
        zmq_push_error_->Send("Can not connect to the ZigbeeGateway");
        return false;
      }

      std::cerr.flush();

    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::IS_PROXY_ACTIVE) {
      if (tcp_client_->GetErrorMessage().compare("Connected") == 0) {
        zigbee_msg.mutable_network_mgmt_cmd()->set_is_proxy_active(true);
      } else {
        zigbee_msg.mutable_network_mgmt_cmd()->set_is_proxy_active(false);
      }

      std::string buffer;
      zigbee_msg.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);

      return true;
    } else if (zigbee_msg.network_mgmt_cmd().type() ==
               ZigBeeMsg::NetworkMgmtCmd::NETWORK_STATUS) {
      command = "info";
    }

    if (tcp_client_.get() != nullptr) {
      tcp_client_->Send(command + "\n");
    }
  } else {
    std::cerr << "zigbee command error" << std::endl;
    zmq_push_error_->Send("zigbee command error");

    return false;
  }
  return true;
}

bool ZigbeeDriver::SendUpdate() {
  std::string line;

  while (tcp_client_->GetLine(&line)) {
    line = Trim(line);
    const char network_state_line[] = "network state";
    std::size_t found = line.find(network_state_line);

    if (found != std::string::npos) {
      int network_type;
      try {
        network_type =
            stoi(line.substr(found + sizeof network_state_line + 1, 2), 0, 10);
      } catch (...) {
        std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
        zmq_push_error_->Send("Not valid value from ZigBeeGateway");
        return false;
      }

      zigbee_msg.set_type(ZigBeeMsg::NETWORK_MGMT);
      zigbee_msg.mutable_network_mgmt_cmd()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::NETWORK_STATUS);

      switch (network_type) {
        // NO_NETWORK
        case 0:
          zigbee_msg.mutable_network_mgmt_cmd()
              ->mutable_network_status()
              ->set_type(ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::NO_NETWORK);
          break;
        // JOINING_NETWORK
        case 1:
          zigbee_msg.mutable_network_mgmt_cmd()
              ->mutable_network_status()
              ->set_type(
                  ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINING_NETWORK);
          break;
        // JOINED_NETWORK
        case 2:
          zigbee_msg.mutable_network_mgmt_cmd()
              ->mutable_network_status()
              ->set_type(
                  ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::JOINED_NETWORK);
          break;
        // JOINED_NETWORK_NO_PARENT
        case 3:
          zigbee_msg.mutable_network_mgmt_cmd()
              ->mutable_network_status()
              ->set_type(ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::
                             JOINED_NETWORK_NO_PARENT);
          break;
        // LEAVING_NETWORK
        case 4:
          zigbee_msg.mutable_network_mgmt_cmd()
              ->mutable_network_status()
              ->set_type(
                  ZigBeeMsg::NetworkMgmtCmd::NetworkStatus::LEAVING_NETWORK);
          break;
        default:
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
          break;
      }
      // Send the serialized proto.
      std::string buffer;
      zigbee_msg.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);

      continue;
    }

    // Detect EMBER_NETWORK_UP & EMBER_NETWORK_DOWN
    const char network_up_line[] = "EMBER_NETWORK_";
    found = line.find(network_up_line);
    if (found != std::string::npos) {
      if (tcp_client_.get() != nullptr) {
        tcp_client_->Send("info\n");
      }

      continue;
    }

    const char discovery_start_line[] = "Discovery Database";
    found = line.find(discovery_start_line);
    if (found != std::string::npos) {
      bulding_discovery_result = true;

      zigbee_msg.set_type(ZigBeeMsg::NETWORK_MGMT);
      zigbee_msg.mutable_network_mgmt_cmd()->set_type(
          ZigBeeMsg::NetworkMgmtCmd::DISCOVERY_INFO);
      zigbee_msg.mutable_network_mgmt_cmd()->clear_connected_nodes();

      continue;
    }

    const char discovery_end_line[] = "devices in database";
    found = line.find(discovery_end_line);
    if (found != std::string::npos) {
      bulding_discovery_result = false;

      // Send the serialized proto.
      std::string buffer;
      zigbee_msg.SerializeToString(&buffer);
      zqm_push_update_->Send(buffer);

      continue;
    }

    if (bulding_discovery_result) {
      const char node_index_line[] = "Node Index";
      found = line.find(node_index_line);
      if (found != std::string::npos) {
        zigbee_msg.mutable_network_mgmt_cmd()->add_connected_nodes();
        continue;
      }

      ZigBeeMsg::NetworkMgmtCmd::NodeDescription* last_node;
      const int nodes_size =
          zigbee_msg.mutable_network_mgmt_cmd()->connected_nodes_size();
      if (nodes_size > 0) {
        last_node =
            zigbee_msg.mutable_network_mgmt_cmd()->mutable_connected_nodes(
                nodes_size - 1);
      } else {
        continue;
      }

      const char node_id_line[] = "NodeId";
      found = line.find(node_id_line);
      if (found != std::string::npos) {
        try {
          const int node_id =
              stoi(line.substr(found + sizeof node_id_line + 1, 6), 0, 16);
          last_node->set_node_id(node_id);
        } catch (...) {
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
        }

        continue;
      }

      const char euid64_line[] = "Eui64";
      found = line.find(euid64_line);
      if (found != std::string::npos) {
        const unsigned long long euid64 = strtoull(
            line.substr(found + sizeof euid64_line + 1, 18).c_str(), 0, 16);
        last_node->set_eui64(euid64);
        continue;
      }

      const char endpoint_line[] = "EP";
      found = line.find(endpoint_line);
      if (found != std::string::npos) {
        ZigBeeMsg::NetworkMgmtCmd::EndPointDescription* new_endpoint =
            last_node->add_endpoints();

        try {
          const int endpoint_index = stoi(
              line.substr(found + sizeof endpoint_line + 1, line.length() - 1),
              0, 10);
          new_endpoint->set_endpoint_index(endpoint_index);
        } catch (...) {
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
        }

        continue;
      }

      ZigBeeMsg::NetworkMgmtCmd::EndPointDescription* last_endpoint;
      const int endpoints_size = last_node->endpoints_size();
      if (endpoints_size > 0) {
        last_endpoint = last_node->mutable_endpoints(endpoints_size - 1);
      } else {
        continue;
      }

      const char profile_id_line[] = "Profile ID";
      found = line.find(profile_id_line);
      if (found != std::string::npos) {
        try {
          const int profile_id =
              stoi(line.substr(found + sizeof profile_id_line + 1, 6), 0, 16);
          last_endpoint->set_profile_id(profile_id);
        } catch (...) {
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
        }

        continue;
      }

      const char device_id_line[] = "Device ID";
      found = line.find(device_id_line);
      if (found != std::string::npos) {
        try {
          const int device_id =
              stoi(line.substr(found + sizeof device_id_line + 1, 6), 0, 16);
          last_endpoint->set_device_id(device_id);
        } catch (...) {
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
        }

        continue;
      }

      const char cluster_line[] = "Cluster:";
      found = line.find(cluster_line);
      if (found != std::string::npos) {
        ZigBeeMsg::NetworkMgmtCmd::ClusterDescription* last_cluster =
            last_endpoint->add_clusters();

        try {
          const int cluster_id =
              stoi(line.substr(found + sizeof cluster_line, 6), 0, 16);
          last_cluster->set_cluster_id(cluster_id);
        } catch (...) {
          std::cerr << "Not valid value from ZigBeeGateway" << std::endl;
          zmq_push_error_->Send("Not valid value from ZigBeeGateway");
          return false;
        }

        found = line.find("Server");
        if (found == std::string::npos) {
          last_cluster->set_type(
              ZigBeeMsg::NetworkMgmtCmd::ClusterDescription::CLIENT_OUT);
        } else {
          last_cluster->set_type(
              ZigBeeMsg::NetworkMgmtCmd::ClusterDescription::SERVER_IN);
        }

        continue;
      }
    }
  }
  return true;
}
}
