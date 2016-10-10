// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = '192.168.2.128'
var creator_everloop_base_port = 20013 + 32 // port for Everloop driver.

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_everloop_base_port /* config */)

var count=0

function sendServoCommand() {
  var servo_cfg_cmd = new matrixMalosBuilder.ServoParams;
  servo_cfg_cmd.set_pin(4);

  process.nextTick(function() {count=count+10});
  servo_cfg_cmd.set_angle(count%180)

  var config = new matrixMalosBuilder.DriverConfig;
  config.set_servo(servo_cfg_cmd);
  configSocket.send(config.encode().toBuffer());
}

sendServoCommand()
setInterval(function() {
  sendServoCommand()
}, 3000);
