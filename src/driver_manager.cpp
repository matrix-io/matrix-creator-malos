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

#include <string>
#include <vector>
#include "third_party/zmq.hpp"

#include "./src/malos.pb.h"

#include "./driver_manager.h"

namespace matrix_malos {

void DriverManager::ServeInfoRequestsForEver() {
  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REP);
  socket.bind("tcp://" + bind_scope_ + ":" + std::to_string(driver_info_port_));

  zmq::message_t request;
  MalosDriverInfo all_driver_info;

  while (true) {
    all_driver_info.Clear();
    // Blocking operation. Ignore request data, just reply.
    socket.recv(&request);

    // Gather info for all drivers.
    for (const MalosWishboneBase* driver : drivers_) {
      DriverInfo* new_info = all_driver_info.add_info();
      driver->FillOutDriverInfo(new_info);
    }

    // TODO: In herit from zmq_push.cpp to avoid duplicating code.
    // Maybe?
    std::string buffer;
    all_driver_info.SerializeToString(&buffer);
    zmq::message_t message(buffer.size());
    memcpy(message.data(), buffer.data(), buffer.size());
    socket.send(message);
  }
}

}  // namespace matrix_malos
