# Everloop

The Everloop driver controls the LED array of the MATRIX Creator.
It follows the [MALOS protocol](../README.md#protocol).

### 0MQ Port
```
20021
```

### Protocol buffers

```
message EverloopImage {
  repeated LedValue led = 1;
}
```

The message `EverloopImage` needs to have exactly 35 messages of type `LedValue` in the repeated field `led`,
corresponding to each of the LEDs present in the Creator. The LEDs are counted starting from the left, clock-wise
as shown in the picture.

![Everloop leds](creator-front-everloop-leds.png)

The message LedValue holds the color values for each LED and each value is in the range [0, 255].

```
message LedValue {
  uint32 red = 1;
  uint32 green = 2;
  uint32 blue = 3;
  uint32 white = 4;
}
```

### Keep-alives

This driver doesn't need keepalive messages as it doesn't send data to the subscribed programs.

### Errors

This driver report errors when an invalid configuration is sent.

### Read

This driver doesn't send any data to a subscribed program.

### JS example

Enhanced description of the [sample source code](../src/js_test/test_everloop.js).


First define the address of the MATRIX Creator. In this case we make it be 127.0.0.1
because we're connecting from the local host but it needs to be different if we
connect from a different computer. There's also the base port reserved by MALOS for
the Everloop driver.

```
var creator_ip = '127.0.0.1'
var creator_everloop_base_port = 20013 + 8 // base port for Everloop driver.
```

Load the protocol buffers used in the example.

```
var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")
```

Create the configuration 0MQ socket.

```
var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_everloop_base_port /* config */)
```

var max_intensity = 50
var intensity_value = max_intensity

function setEverloop() {
    var config = new matrixMalosBuilder.DriverConfig
    config.image = new matrixMalosBuilder.EverloopImage
    for (var j = 0; j < 35; ++j) {
      var ledValue = new matrixMalosBuilder.LedValue;
      ledValue.setRed(0);
      ledValue.setGreen(intensity_value);
      ledValue.setBlue(0);
      ledValue.setWhite(0);
      config.image.led.push(ledValue)
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
