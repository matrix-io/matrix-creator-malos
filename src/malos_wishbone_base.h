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

#ifndef SRC_MALOS_WISHBONE_BASE_H_
#define SRC_MALOS_WISHBONE_BASE_H_

#include <memory>
#include <string>

#include "./zmq_push.h"
#include "./zmq_pull.h"

#include "matrix_hal/wishbone_bus.h"
#include "./src/malos.pb.h"

namespace matrix_malos {

class MalosWishboneBase {
 public:
  explicit MalosWishboneBase(const std::string& driver_name)
      : is_active_(false),
        delay_between_updates_(1000),
        timeut_after_last_ping_(5000),
        driver_name_(driver_name) {}

  // Base port for all the 0MQ  channels.
  // base_port => config channel (pull)
  // base_port + 1 => keepalive channel (pull)
  // base_port + 2 => error channel (push)
  // base_port + 3 => send update channel (push)
  // Bind_scope is the IP to listen on. Use "*" to listen on every interfface.
  bool Init(int base_port, const std::string& bind_scope);

  // Thread that waits for configuration changes.
  void ConfigThread();
  // Thread that send updates to subscribers.
  void UpdateThread();
  // Thead that receives the keepalives.
  void KeepAliveThread();

  // This function should be overridden by drivers that use the wishbone bus.
  virtual void SetupWishboneBus(matrix_hal::WishboneBus*) {}

  // This function should be overridden by drivers. Here is where they send
  // updates.
  virtual bool SendUpdate() { return false; }

  virtual bool ProcessConfig(const DriverConfig&) { return true; }

 private:
  // Is the driver active and sending updates?
  bool is_active_;
  // Delay between updates in milliseconds.
  int delay_between_updates_;
  // Timeout after last ping.
  int timeut_after_last_ping_;
  // Driver name.
  std::string driver_name_;
  // ZMQ channel where configuration is received.
  std::unique_ptr<ZmqPuller> zmq_pull_config_;
  // ZMQ channel where keepalives are received.
  std::unique_ptr<ZmqPuller> zmq_pull_keepalive_;

 protected:
  // ZMQ channel where errors are sent.
  std::unique_ptr<ZmqPusher> zmq_push_error_;
  // ZMQ channel where actual data updates are sent.
  std::unique_ptr<ZmqPusher> zqm_push_update_;
};

}  // namespace matrix_malos

#endif  // SRC_MALOS_WISHBONE_BASE_H_
