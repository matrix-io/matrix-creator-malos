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

#ifndef SRC_DRIVER_ZIGBEE_H_
#define SRC_DRIVER_ZIGBEE_H_

#include <memory>

#include "./malos_wishbone_base.h"
#include "./tcp_client.h"

const char kZigbeeDriverName[] = "ZigbeeBulb";

namespace matrix_malos {

// FIXME: inherit from malos_base.h

class ZigbeeDriver : public MalosWishboneBase {
 public:
  ZigbeeDriver() : MalosWishboneBase(kZigbeeDriverName) {
    SetNeedsKeepalives(true);
    SetMandatoryConfiguration(true);
    SetNotesForHuman("Zigbee bulb driver. In development");
  }

  // Read configuration of LEDs (from the outside world).
  bool ProcessConfig(const DriverConfig& config) override;

  // Send updates. Checks for new messages from Zigbee.
  bool SendUpdate() override;

 private:
  // Tcp client.
  std::unique_ptr<TcpClient> tcp_client_;
  const std::string gateway_ip = "127.0.0.1";
  const int gateway_port = 4901;
};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_ZIGBEE_H_
