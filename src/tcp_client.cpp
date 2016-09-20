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

#include "./tcp_client.h"
#include <string.h>

namespace matrix_malos {

TcpClient::TcpClient() : sock_(-1), msg_error_("")
{
}

bool TcpClient::Connect(std::string address, int port) {
  msg_error_ = "";

  if (sock_ == -1) {
    // Create socket
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == -1) {
      msg_error_ += "Could not create socketi\n";
    }
  }

  if (inet_addr(address.c_str()) == -1) {
    struct hostent *he;
    struct in_addr **addr_list;

    // resolve the hostname, its not an ip address
    if ((he = gethostbyname(address.c_str())) == NULL) {
      herror("gethostbyname");
      msg_error_ += "Failed to resolve hostname\n";
      return false;
    }

    addr_list = (struct in_addr **)he->h_addr_list;

    for (int i = 0; addr_list[i] != NULL; i++) {
      server_.sin_addr = *addr_list[i];

      break;
    }
  } else {
    server_.sin_addr.s_addr = inet_addr(address.c_str());
  }

  server_.sin_family = AF_INET;
  server_.sin_port = htons(port);

  if (connect(sock_, (struct sockaddr *)&server_, sizeof(server_)) < 0) {
    msg_error_ += "Connectect failed.";
    return false;
  }

  return true;
}

bool TcpClient::SendData(std::string data) {
  msg_error_ = "";

  if (send(sock_, data.c_str(), strlen(data.c_str()), 0) < 0) {
    msg_error_ =  "Send failed!";
    return false;
  }

  return true;
}

/**
    Receive data from the Connectected host
*/
std::string TcpClient::receive(int size = 512) {
  char buffer[size];

  if (recv(sock_, buffer, sizeof(buffer), 0) < 0) {
    msg_error_ = "recv failed";
  } else {
    msg_error_ = "";
  }

  return std::string(buffer);
}

}  // namespace matrix_malos
