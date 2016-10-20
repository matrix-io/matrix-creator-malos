
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

### Writing Custom MALOS Integrations
##### ZeroMQ ports
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

##### Connecting to ZeroMQ with NodeJS

**Note:** You'll need `protobufjs` and `zmq` npm packages.
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

var creator_ip = '127.0.0.1'
var creator_everloop_base_port = 20013 + 8 // port for Everloop driver.

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_everloop_base_port /* config */)
```

##### Available Protobufs
Protobufs are the interfaces used
```
driver.proto // drivers & configuration 
hal.proto // sensors
```
