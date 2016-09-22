/*
 * Copyright 2016 <Admobilize>
 */

#include <chrono>
#include <iostream>
#include <thread>

#include "./malos_base.h"

namespace matrix_malos {

// How many threads for a given 0MQ context. Just one.
const int kOneThread = 1;

// Hight water mark. How many samples to queue (outgoing).
const int kSmallHighWaterMark = 4;

// Default delay when inactive.
const int kDefaultDelayWhenInactive = 100;

bool MalosBase::Init(int base_port, const std::string &bind_scope) {
  base_port_ = base_port;

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
  std::thread config_thread(&MalosBase::ConfigThread, this);
  config_thread.detach();
  // Receive pings.
  std::thread keepalive_thread(&MalosBase::KeepAliveThread, this);
  keepalive_thread.detach();
  // Send update to clients.
  std::thread update_thread(&MalosBase::UpdateThread, this);
  update_thread.detach();

  std::cerr << "Registered driver " << driver_name_ << " with port "
            << std::to_string(base_port) << "." << std::endl;

  return true;
}

void MalosBase::ConfigThread() {
  // TODO: Fill out key/value pairs and make them readable by
  // derived classes.
  while (true) {
    if (zmq_pull_config_->Poll(ZmqPuller::WAIT_FOREVER)) {
      std::lock_guard<std::mutex> lock(config_mutex_);
      DriverConfig config;
      // Can we parse a configuration?
      if (!config.ParseFromString(zmq_pull_config_->Read())) {
        std::cerr << "Invalid configuration for " << driver_name_ << " driver."
                  << std::endl;
        zmq_push_error_->Send("0, Invalid configuration for " + driver_name_ +
                              " driver. Could not parse protobuf.");
        has_been_configured_ = false;
        continue;
      }
      // Derived classes receive the config. For instance, in the case of the
      // Everloop driver there is data for the driver to read. In the case of
      // malos-eye,
      // the camera and the detectors need to be configured.
      if (!ProcessConfig(config)) {
        std::cerr << "Specific config for " << driver_name_ << " failed.";
        zmq_push_error_->Send("0, Invalid specific configuration for " +
                              driver_name_ + " driver.");
        has_been_configured_ = false;
        continue;
      }

      // Shall we set a new delay between updates?
      if (config.delay_between_updates() > 0) {
        std::cerr << "New delay between updates for " << driver_name_ << " is "
                  << delay_between_updates_ << " ms." << std::endl;
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
      has_been_configured_ = true;
    }
  }
}

void MalosBase::UpdateThread() {
  while (true) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    // If the device needs mandatory configuration, do not send updates until a
    // valid
    // configuration has been received.
    if ((mandatory_configuration_ && !has_been_configured_) || !is_active_) {
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

void MalosBase::KeepAliveThread() {
  while (true) {
    is_active_ = zmq_pull_keepalive_->Poll(timeout_after_last_ping_);
    if (is_active_) {
      // Discard anything that was received. Just a ping, man.
      zmq_pull_keepalive_->Read();
    }
  }
}

void MalosBase::FillOutDriverInfo(DriverInfo *driver_info) const {
  driver_info->set_driver_name(driver_name_);
  driver_info->set_base_port(base_port_);
  driver_info->set_provides_updates(provides_updates_);
  driver_info->set_delay_between_updates(delay_between_updates_);
  driver_info->set_needs_pings(needs_pings_);
  driver_info->set_timeout_after_last_ping(timeout_after_last_ping_);
  driver_info->set_notes_for_human(notes_for_human_);
}

}  // namespace matrix_malos
