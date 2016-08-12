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

#ifndef SRC_ZMQ_PUSH_H_
#define SRC_ZMQ_PUSH_H_

#include <memory>
#include <string>

#include "third_party/zmq.hpp"

namespace matrix_malos {

// This class is used to push ZMQ messages to subscribers.
// A small wrapper to make code cleaner in upper layers.
class ZmqPusher {
 public:
  ZmqPusher() {}

  // Initialize the pusher in a given port and host_name. Use
  // '*' for host_name if you want to listen in all the available
  // interfaces.
  bool Init(int port, int nthreads, int high_water_mark,
            const std::string &bind_scope);

  // Push message.
  bool Send(const std::string &message_str);

 private:
  std::unique_ptr<zmq::context_t> context_;
  std::unique_ptr<zmq::socket_t> socket_;
};

}  // namespace matrix_malos

#endif  // SRC_ZMQ_PUSH_H_
