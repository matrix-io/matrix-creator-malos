
# Pre-Requisites
```
echo "deb http://packages.matrix.one/matrix-creator/ ./" | sudo tee --append /etc/apt/sources.list;
sudo apt-get update;
sudo apt-get upgrade;
sudo apt-get install libzmq3-dev xc3sprog matrix-creator-openocd wiringpi cmake g++ git;
```

# MALOS

Hardware abstraction layer for MATRIX Creator usable via 0MQ. Protocol buffers are used for data exchange.

### Install MALOS
```
sudo apt-get install matrix-creator-init matrix-creator-malos
sudo shutdown -r now
```

### Upgrade MALOS
```
sudo apt-get update && sudo apt-get upgrade
sudo shutdown -r now
```

### Test MALOS

MALOS starts as a service, there is no need to start it by hand.

### Clone and test examples
Note: pre-requisite is NodeJS. Don't use the one shipped with raspbian because it's a bit old. If you don't have it, please check a recipe included below.
```
git clone https://github.com/matrix-io/matrix-creator-malos.git && cd matrix-creator-malos
git submodule init && git submodule update
cd src/js_test

// humidity, temperatre
node test_humidity.js 

// inertial measurement unit
node test_imu.js 

// pressure, altitude
node test_pressure.js 

// uv index, uv range
node test_uv.js
```
-------------------------

### NodeJS Dependency

For instance (in the Raspberry):

```
# Install npm (doesn't really matter what version, apt-get node is v0.10...)
sudo apt-get install npm

# n is a node version manager
sudo npm install -g n

# node 6.5 is the latest target node version, also installs new npm
n 6.5

# check version
node -v
```

### Connecting to MALOS
##### ZeroMQ ports
MALOS uses ZeroMQ to transfer information to and from the client interfacing with it. Below are a list of available ports MALOS allows you to interface with.
```
Registered driver IMU with port 20013.
Registered driver Humidity with port 20017.
Registered driver Everloop with port 20021.
Registered driver Pressure with port 20025.
Registered driver UV with port 20029.
Registered driver ZigbeeBulb with port 20033.
Registered driver MicArray_Alsa with port 20037.
Registered driver Lirc with port 20041.
```
### MALOS Examples
##### Connecting to MALOS with NodeJS
**Note:** You'll need `protobufjs` and `zmq` npm packages, and protobuf as a submodule. See the [Everloop Example](https://github.com/matrix-io/matrix-creator-malos/blob/master/src/js_test/test_everloop.js) for the full implementation of the code below.
```
// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var protoBuf = require("protobufjs");
var zmq = require('zmq');
var configSocket = zmq.socket('push')

var creator_ip = '127.0.0.1'
var creator_everloop_base_port = 20013 + 8 // port for Everloop driver.

// relative to where you have the protobufs
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')

// malos interface
var matrixMalosBuilder = protoBuilder.build("matrix_malos")
configSocket.connect('tcp://' + creator_ip + ':' + creator_everloop_base_port /* config */)

...
```
##### Passing Commands to MALOS
Below is an example of some NodeJS interfacing with the Everloop via MALOS. See the [Everloop Example](https://github.com/matrix-io/matrix-creator-malos/blob/master/src/js_test/test_everloop.js) for the full implementation of the code below.
```
...

var max_intensity = 50
var intensity_value = max_intensity

function setEverloop() {
    var config = new matrixMalosBuilder.DriverConfig
    config.image = new matrixMalosBuilder.EverloopImage
    
    // Iteration over all 35 Everloop LEDs to turn them green.
    for (var j = 0; j < 35; ++j) {
      var ledValue = new matrixMalosBuilder.LedValue;

      // set brightness/intensity of the color for each rgb LED.
      ledValue.setRed(0);
      ledValue.setGreen(intensity_value);
      ledValue.setBlue(0);
      ledValue.setWhite(0);
      config.image.led.push(ledValue);
    }
    configSocket.send(config.encode().toBuffer());
}

setEverloop(intensity_value)
setInterval(function() {
  intensity_value -= 1
  if (intensity_value < 0)
    intensity_value = max_intensity
  setEverloop()
}, 10);
```
##### Reading from MALOS
Below is a simple implementation via NodeJS to read a `humidity` from MALOS via ZeroMQ. See [Humidty Example](https://github.com/matrix-io/matrix-creator-malos/blob/master/src/js_test/test_humidity.js) for the full example.
```
// Start configuration for refresh rate, and heartbeat timeouts
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_humidity_base_port)
// Send driver configuration.
var driverConfigProto = new matrixMalosBuilder.DriverConfig
// 2 seconds between updates.
driverConfigProto.delay_between_updates = 2.0
// Stop sending updates 6 seconds after pings.
driverConfigProto.timeout_after_last_ping = 6.0
configSocket.send(driverConfigProto.encode().toBuffer())
// ********** End configuration.

// ********** Start updates - Here is where they are received.
var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (creator_humidity_base_port + 3))
updateSocket.subscribe('')
updateSocket.on('message', function(buffer) {
  var data = new matrixMalosBuilder.Humidity.decode(buffer)
  console.log(data)
});
// ********** End updates
```


##### Available Protobufs
```
driver.proto // drivers
hal.proto // sensors & controllers
```
