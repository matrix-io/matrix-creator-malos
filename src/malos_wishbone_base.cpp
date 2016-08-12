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
#include <iostream>
#include <memory>
#include <thread>

#include "./malos_wishbone_base.h"

namespace matrix_malos {

// How many threads for a given 0MQ context. Just one.
const int kOneThread = 1;

// Hight water mark. How many samples to queue (outgoing).
const int kSmallHighWaterMark = 4;

// Default delay when inactive.
const int kDefaultDelayWhenInactive = 50;

bool MalosWishboneBase::Init(int base_port, const std::string& bind_scope) {
  zmq_pull_config_.reset(new ZmqPuller());
  if (!zmq_pull_config_->Init(base_port, kOneThread, bind_scope)) {
    return false;
  }

  zmq_pull_keepalive_.reset(new ZmqPuller());
  if (!zmq_pull_keepalive_->Init(base_port + 1, kOneThread, bind_scope)) {
    return false;
  }

  zmq_push_error_.reset(new ZmqPusher());
  if (!zmq_push_error_->Init(base_port + 2, kOneThread, kSmallHighWaterMark,
                             bind_scope)) {
    return false;
  }

  zqm_push_update_.reset(new ZmqPusher());
  if (!zqm_push_update_->Init(base_port + 3, kOneThread, kSmallHighWaterMark,
                              bind_scope)) {
    return false;
  }

  // Receive configurations.
  std::thread config_thread(&MalosWishboneBase::ConfigThread, this);
  config_thread.detach();
  // Receive pings.
  std::thread keepalive_thread(&MalosWishboneBase::KeepAliveThread, this);
  keepalive_thread.detach();
  // Send update to clients.
  std::thread update_thread(&MalosWishboneBase::UpdateThread, this);
  update_thread.detach();

  std::cerr << "Registered driver " << driver_name_ << " with port "
            << std::to_string(base_port) << "." << std::endl;

  return true;
}

void MalosWishboneBase::ConfigThread() {
  // TODO: Fill out key/value pairs and make them readable by
  // derived classes.
  while (true) {
    if (zmq_pull_config_->Poll(ZmqPuller::WAIT_FOREVER)) {
      DriverConfig config;
      if (config.ParseFromString(zmq_pull_config_->Read())) {
        // Derived classes receive the config. For instance, in the case of the
        // Everloop driver there is data for the driver to read.
        if (!ProcessConfig(config)) {
          std::cerr << "Specific config for " << driver_name_ << " failed.";
        }

        // Shall we set a new delay between updates?
        if (config.delay_between_updates() > 0) {
          std::cerr << "New delay between updates for " << driver_name_
                    << " is " << delay_between_updates_ << " ms." << std::endl;
          delay_between_updates_ =
              config.delay_between_updates() * static_cast<float>(1000);
        }

        // Shall we set a timeout after the last ping?
        if (config.timeout_after_last_ping() > 0) {
          timeout_after_last_ping_ =
              config.timeout_after_last_ping() * static_cast<float>(1000);
          std::cerr << "New timeout after last ping for " << driver_name_ << " "
                    << timeout_after_last_ping_ << " ms." << std::endl;
        }
      } else {
        std::cerr << "Invalid configuration for " << driver_name_ << " driver."
                  << std::endl;
        zmq_push_error_->Send("0, Invalid configuration for " + driver_name_ +
                              " driver.");
      }
    }
  }
}

void MalosWishboneBase::UpdateThread() {
  while (true) {
    if (!is_active_) {
      // We know this is not the best way to do this.
      // The thread should start when the driver is active and end when it's
      // not.
      // This introduces latency for the driver to start once it becomes active.
      std::this_thread::sleep_for(
          std::chrono::milliseconds(kDefaultDelayWhenInactive));
      continue;
    }
    if (!SendUpdate()) {
      zmq_push_error_->Send("1, Could not send update for " + driver_name_ +
                            " driver.");
    }
    std::this_thread::sleep_for(
        std::chrono::milliseconds(delay_between_updates_));
  }
}

void MalosWishboneBase::KeepAliveThread() {
  while (true) {
    is_active_ = zmq_pull_keepalive_->Poll(timeout_after_last_ping_);
    if (is_active_) {
      std::cerr << driver_name_ << " driver received ping." << std::endl;
      // Discard anything that was received. Just a ping, man.
      zmq_pull_keepalive_->Read();
    }
  }
}

}  // namespace matrix_malos
