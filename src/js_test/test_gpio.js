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
var creator_gpio_base_port = 20013 + 36 // port for Gpio driver.
var toggle = false

var zmq = require('zmq')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io


var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_gpio_base_port /* config */)

// ********** Start configuration ******************
configGpioOutput() // GPIO 0 to output mode. (toggle value only for demo)
configGpioInput()  // GPIO 1 to input mode. (arrive updates on "sub" callback)  

// ***** Register update callback:
var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (creator_gpio_base_port + 3))
updateSocket.subscribe('')

updateSocket.on('message', (gpio_buffer) => {
  var gpioData = matrix_io.malos.v1.io.GpioParams.decode(gpio_buffer)
  // output in binary format all 15 pins of GPIO
  console.log('<== GPIO pins register: ', dec2bin(gpioData.values))
});
// ***** End updates.
// ********** End configuration ******************

// ********** Ping the GPIO driver
var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (creator_gpio_base_port + 1))
// Ping the first time.
pingSocket.send(''); 
// Ping every 2 seconds (same to delay between updates)
setInterval(() => {
  pingSocket.send(''); // ping for new updates
  configGpioOutput()   // for demo: toggle pin 
}, 2000);
// ********** Ping the driver ends

// ************************************************
// ********** GPIO config functions ***************
// ************************************************
function configGpioOutput() {
  // Set GPIO 0 to output mode
  var gpio_cfg_cmd = matrix_io.malos.v1.io.GpioParams.create({
    pin: 0,
    mode: matrix_io.malos.v1.io.GpioParams.EnumMode.OUTPUT
  })

  // For demo: toggle function for output value
  process.nextTick(() => { toggle =! toggle });
  console.log('==> pin 0 set to:', toggle)
  if (toggle) gpio_cfg_cmd.value = 1;
  else gpio_cfg_cmd.value = 0;

  // build and sending proto
  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    gpio: gpio_cfg_cmd
  })
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())
}

function configGpioInput() { 
  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    delayBetweenUpdates: 0.250, // 250 miliseconds between updates.
    timeoutAfterLastPing: 2.0,  // Stop sending updates 2 seconds after pings.
    gpio: matrix_io.malos.v1.io.GpioParams.create({
      pin: 1, // GPIO params: set pin 1 to input mode
      mode: matrix_io.malos.v1.io.GpioParams.EnumMode.INPUT
    })
  })
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())
}

// *********** Tools functions *************
function dec2bin(dec){
  return (dec >>> 0).toString(2);
}

