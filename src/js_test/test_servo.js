// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = process.env.CREATOR_IP || '127.0.0.1'
var creator_servo_base_port = 20013 + 32 // port for Servo driver.

var zmq = require('zmq')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io


var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_servo_base_port /* config */)

var count=0

function sendServoCommand() {
  var servo_cfg_cmd = matrix_io.malos.v1.io.ServoParams.create({
    pin: 4
  })

  process.nextTick(() => {count=count+10});
  var angle=count%180;
  console.log('angle: ', angle);
  servo_cfg_cmd.angle = angle;

  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    servo: servo_cfg_cmd
  })
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish());
}

sendServoCommand()
setInterval(() => {
  sendServoCommand()
}, 3000);
