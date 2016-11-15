// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = '127.0.0.1'
var creator_micarray_base_port = 20037 // port for Lirc driver.

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_micarray_base_port /* config */)

function setupMicarray() {
  var micarray_cfg = new matrixMalosBuilder.MicArrayParams
  
  micarray_cfg.set_gain(8)
  micarray_cfg.set_azimutal_angle(0)
  micarray_cfg.set_polar_angle(0)
  micarray_cfg.set_radial_distance_mm(1000)
  micarray_cfg.set_sound_speed_mmseg(340.3 * 1000)

  var config = new matrixMalosBuilder.DriverConfig
  config.set_micarray(micarray_cfg)
  configSocket.send(config.encode().toBuffer());
}

setupMicarray()

process.exit(0)
