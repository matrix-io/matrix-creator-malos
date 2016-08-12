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

#include "./zmq_push.h"

namespace matrix_malos {

bool ZmqPusher::Init(int port, int n_threads, int high_water_mark,
                     const std::string &bind_scope) {
  // Get the context first.
  context_.reset(new zmq::context_t(n_threads));

  // Now get the socket.
  socket_.reset(new zmq::socket_t(*(context_.get()), ZMQ_PUB));

  // Set high water mark.
  socket_->setsockopt(ZMQ_SNDHWM, (void *)&high_water_mark,
                      sizeof(high_water_mark));

  // Bind the socket.
  socket_->bind("tcp://" + bind_scope + ":" + std::to_string(port));
  // Assume things went well.
  return true;
}

bool ZmqPusher::Send(const std::string &message_str) {
  zmq::message_t message(message_str.size());
  memcpy(message.data(), message_str.data(), message_str.size());
  return socket_->send(message);
}

}  // namespace  matrix_malos
