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

#ifndef SRC_DRIVER_TCP_CLIENT_H_
#define SRC_DRIVER_TCP_CLIENT_H_

#include <string>

namespace matrix_malos {

/**
    TCP Client class
*/
class TcpClient {
 public:
  TcpClient() : sock_(-1), msg_error_("") {}
  ~TcpClient();
  bool Connect(const std::string& address, int port);
  bool Send(const std::string& data);
  bool GetLine(std::string* line);
  std::string GetErrorMessage() { return msg_error_; }

 private:
  int sock_;
  std::string msg_error_;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_TCP_CLIENT_H_
