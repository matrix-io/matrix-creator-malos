# GPIO

The Servo driver on current version supports:

<a href="https://github.com/matrix-io/matrix-creator-malos/blob/av/doc_servo/docs/servo_diagram.jpg"><img src="https://github.com/matrix-io/matrix-creator-malos/blob/av/doc_servo/docs/servo_diagram.jpg" align="right" width="420" ></a>

<a href="https://github.com/matrix-io/matrix-creator-malos/blob/master/docs/gpio_diagram.jpg"><img src="https://github.com/matrix-io/matrix-creator-malos/blob/master/docs/gpio_diagram.jpg" align="right" width="420" ></a>

* Handle Servo via GPIO pin output
* Set Servo angle (only 180ø servos)

The driver follows the [MALOS protocol](../README.md#protocol).

### GPIO electrical characteristics

* GPIO voltage: 0.60-4.10 VDC ([details](https://github.com/matrix-io/matrix-creator-quickstart/wiki/Data-Sheets))
* current 10mA max
* all GPIO pins need pullups
* requiere external source for servo

### 0MQ Port
```
20045
```
### Protocol buffers

```
// Servo handler params
message ServoParams {
  // GPIO to config
  uint32 pin = 1;

  // Servo mode
  uint32 angle = 2; 
}
```
The message is defined in [driver.proto](https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto).

### Errors

This driver reports errors when an invalid configuration is sent.


### Write

All pins on matrix creator start as inputs. You need to change the default settings. (see Javascript example below)


This is a sample output given by the example described below.

```
$ node test_servo.js 
angle: 0
angle: 10
angle: 20
angle: 30
angle: 40
```

(The servo motor will change by the different angles)


### JavaScript example

Enhanced description of the [sample source code](../src/js_test/test_servo.js).

First, define the address of the MATRIX Creator. In this case we make it be `127.0.0.1`
because we are connecting from the local host but it needs to be different if we
connect from another computer. There is also the base port reserved by MALOS for
the Pressure driver.

```
var creator_ip = '127.0.0.1'
var creator_pressure_base_port = 20013 + 32
```

#### Load the protocol buffers used in the example.

```
var protoBuf = require("protobufjs");
// parse proto file
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
// Parse matrix_malos package (namespace).
var matrixMalosBuilder = protoBuilder.build("matrix_malos")
```

#### Connection to servo driver
```
var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_servo_base_port /* config */)
```

#### Configure Servo GPIO pin and send angle:
All the drivers are configured using the message `driverconfig` (see [driver.proto](https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto)).
```
function sendServoCommand() {
  var servo_cfg_cmd = new matrixMalosBuilder.ServoParams;
  servo_cfg_cmd.set_pin(4);

  process.nextTick(function() {count=count+10});
  var angle=count%180;
  console.log('angle:',angle);
  servo_cfg_cmd.set_angle(angle);
  
  var config = new matrixMalosBuilder.DriverConfig;
  config.set_servo(servo_cfg_cmd);
  configSocket.send(config.encode().toBuffer());
}
```

#### Send continous configuration:

```
sendServoCommand()
setInterval(function() {
  sendServoCommand()
}, 3000);
```

