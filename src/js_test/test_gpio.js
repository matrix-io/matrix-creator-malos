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
var toggle=false

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_gpio_base_port /* config */)

// ********** Start configuration ******************
configGpioOutput() // GPIO 0 to output mode. (toggle value only for demo)
configGpioInput()  // GPIO 1 to input mode. (arrive updates on "sub" callback)  

// ***** Register update callback:
var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (creator_gpio_base_port + 3))
updateSocket.subscribe('')

updateSocket.on('message', function(gpio_buffer) {
  var gpioData = new matrixMalosBuilder.GpioParams.decode(gpio_buffer)
  // output in binary format all 15 pins of GPIO
  console.log('<== GPIO pins register: ',dec2bin(gpioData.values))
});
// ***** End updates.
// ********** End configuration ******************

// ********** Ping the GPIO driver
var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (creator_gpio_base_port + 1))
// Ping the first time.
pingSocket.send(''); 
// Ping every 2 seconds (same to delay between updates)
setInterval(function() {
  pingSocket.send(''); // ping for new updates
  configGpioOutput()   // for demo: toggle pin 
}, 2000);
// ********** Ping the driver ends

// ************************************************
// ********** GPIO config functions ***************
// ************************************************
function configGpioOutput() {
  // Set GPIO 0 to output mode
  var gpio_cfg_cmd = new matrixMalosBuilder.GpioParams
  gpio_cfg_cmd.set_pin(0)
  gpio_cfg_cmd.set_mode(matrixMalosBuilder.GpioParams.EnumMode.OUTPUT)
  // For demo: toggle function for output value
  process.nextTick(function() {toggle=!toggle});
  console.log('==> pin 0 set to:',toggle)
  if(toggle)gpio_cfg_cmd.set_value(1);
  else gpio_cfg_cmd.set_value(0);
  // build and sending proto
  var config = new matrixMalosBuilder.DriverConfig
  config.set_gpio(gpio_cfg_cmd)
  configSocket.send(config.encode().toBuffer())
}

function configGpioInput() { 
  var config = new matrixMalosBuilder.DriverConfig
  // 250 miliseconds between updates.
  config.delay_between_updates = .250
  // Stop sending updates 2 seconds after pings.
  config.timeout_after_last_ping = 2.0
  // GPIO params: set pin 1 to input mode
  var gpio_cfg_cmd = new matrixMalosBuilder.GpioParams
  gpio_cfg_cmd.set_pin(1)
  gpio_cfg_cmd.set_mode(matrixMalosBuilder.GpioParams.EnumMode.INPUT)
  // build and sending proto
  config.set_gpio(gpio_cfg_cmd)
  configSocket.send(config.encode().toBuffer())
}

// *********** Tools functions *************
function dec2bin(dec){
  return (dec >>> 0).toString(2);
}

