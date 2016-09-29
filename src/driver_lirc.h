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

#ifndef SRC_DRIVER_LIRC_H_
#define SRC_DRIVER_LIRC_H_

const char kLircDriverName[] = "Lirc";

namespace matrix_malos {

class LircDriver : public MalosBase {
 public:
  LircDriver() : MalosBase(kLircDriverName) {
    SetProvidesUpdates(false);
    SetNeedsKeepalives(false);
    SetNotesForHuman(
        "Write-read. Get list remotes and send commands via IR. In development"
    );
  }

  // Read configuration of IR command (from the outside world).
  bool ProcessConfig(const DriverConfig& config) override;

};

}  // namespace matrix_malos

#endif  // SRC_DRIVER_LIRC_H_
