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
var creator_gpio_base_port = 20013 + 36 // port for Gpio driver.

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_gpio_base_port /* config */)

// ********** Start updates - Here is where they are received.
var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (creator_gpio_base_port + 3))
updateSocket.subscribe('')

updateSocket.on('message', function(gpio_buffer) {
  var imuData = new matrixMalosBuilder.GpioParams.decode(gpio_buffer)
  console.log('on pin 1 receive: ',imuData.value)
});

var toggle=false

function sendGpioValue() {
  var gpio_cfg_cmd = new matrixMalosBuilder.GpioParams
  gpio_cfg_cmd.set_pin(0)
  gpio_cfg_cmd.set_mode(matrixMalosBuilder.GpioParams.EnumMode.OUTPUT)

  process.nextTick(function() {toggle=!toggle});
  console.log('on pin 0 sending:',toggle)
  if(toggle)gpio_cfg_cmd.set_value(1);
  else gpio_cfg_cmd.set_value(0);

  var config = new matrixMalosBuilder.DriverConfig
  config.set_gpio(gpio_cfg_cmd)
  configSocket.send(config.encode().toBuffer())
}

function receiveGpioValue() {
  var gpio_cfg_cmd = new matrixMalosBuilder.GpioParams
  gpio_cfg_cmd.set_pin(1)
  gpio_cfg_cmd.set_mode(matrixMalosBuilder.GpioParams.EnumMode.INPUT)

  var config = new matrixMalosBuilder.DriverConfig
  config.set_gpio(gpio_cfg_cmd)
  configSocket.send(config.encode().toBuffer())
}

setInterval(function() {
  sendGpioValue()
  receiveGpioValue();
}, 2000);
