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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <string.h>

#include <string>
#include <iostream>

#include "./tcp_client.h"

// Following
// http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleclient

namespace {
static void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
}  // namespace

namespace matrix_malos {

TcpClient::~TcpClient() {
  if (sock_ != -1) {
    close(sock_);
  }
}

bool TcpClient::Connect(const std::string &address, int port) {
  // FIXME: Use better variable names.

  if (sock_ != -1) {
    close(sock_);
  }

  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  // Create socket

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints,
                        &servinfo)) != 0) {
    msg_error_ = gai_strerror(rv);
    return false;
  }

  // loop through all the results and connect to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sock_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sock_, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock_);
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL) {
    msg_error_ = "client: failed to connect";
    return false;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
            sizeof s);

  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo);  // all done with this structure

  msg_error_ = "Connected";
  return true;
}

bool TcpClient::Send(const std::string &data) {
  msg_error_ = "";

  // FIXME: Use SendAll.

  if (send(sock_, data.c_str(), strlen(data.c_str()), 0) < 0) {
    msg_error_ = "Send failed!";
    return false;
  }

  return true;
}

// FIXME: Better variable names. Improve select logic.

bool TcpClient::GetLine(std::string *line) {
  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = 0;
  // FIXME: Use 0 here.
  tv.tv_usec = 500000;

  FD_ZERO(&readfds);
  FD_SET(sock_, &readfds);

  bool bytes_to_read = true;

  // don't care about writefds and exceptfds:
  if (select(sock_ + 1, &readfds, NULL, NULL, &tv) == -1) {
    bytes_to_read = false;
  }

  if (bytes_to_read) {
    int nbytes = 0;
    char buf[512];

    if ((nbytes = recv(sock_, buf, sizeof buf, 0)) <= 0) {
      if (nbytes == 0) {
        std::cerr << "Gateway socket hung up" << std::endl;
      } else {
        perror("recv");
      }
      close(sock_);
      sock_ = -1;
    } else if (FD_ISSET(sock_, &readfds)) {
      buffer_.append(buf, nbytes);
    }
  }

  int ltrim_count = 0;
  for (int i = 0; i < static_cast<int>(buffer_.size()); ++i) {
    if (buffer_.at(i) == '\n' || buffer_.at(i) == '\r') {
      ++ltrim_count;
    }
  }

  std::string ret;
  bool can_return = false;

  for (int i = ltrim_count; i < static_cast<int>(buffer_.size()); ++i) {
    if (buffer_.at(i) == '\n' || buffer_.at(i) == '\r') {
      if (ret.size() > 0) {
        can_return = true;
      }
      break;
    }
    ret.append(1, buffer_.at(i));
    ++ltrim_count;
  }

  if (can_return) {
    buffer_.erase(0, ltrim_count);
    line->assign(ret);
    return true;
  }
  return false;
}

// FIXME: This function will be replace to manage partial reads.

#if 0
std::string TcpClient::receive(int size = 512) {
  char buffer[size];

  if (recv(sock_, buffer, sizeof(buffer), 0) < 0) {
    msg_error_ = "recv failed";
  } else {
    msg_error_ = "";
  }

  return std::string(buffer);

}
#endif

}  // namespace matrix_malos
