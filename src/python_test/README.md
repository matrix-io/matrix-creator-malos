# GPIO python example

On this example, we will connect to MALOS GPIO driver for controlling a single pin. GPIO driver on MATRIX creator supports:<a href="https://github.com/matrix-io/matrix-creator-malos/blob/master/docs/gpio_diagram.jpg"><img src="https://github.com/matrix-io/matrix-creator-malos/blob/master/docs/gpio_diagram.jpg" align="right" width="420" ></a>

* GPIO pin input
* GPIO pin output
* GPIO updates (state from all pins)

The MALOS driver follows the [MALOS protocol](../README.md#protocol).

### Python prerequisites

 install python packages dependences:
 ``` bash
 pip install pyzmq protobuf
 ```

 build protocol buffer source for create driver messages:
 ``` bash
 export SRC_DIR=../../protocol-buffers/malos
 protoc -I=$SRC_DIR --python_out=./ $SRC_DIR/driver.proto
 ```

### Running sample

``` bash
$ python test_gpio.py 
GPIO15=0
GPIO15=1
GPIO15=0

```
(on this example: pin 15 toggle value 0 and 1)

### Example details

Enhanced description of the [sample source code](../src/python_test/test_gpio.py).

First, define the address of the MATRIX Creator. In this case we make it be `127.0.0.1`
because we are connecting from the local host but it needs to be different if we
connect from another computer. There is also the base port reserved by MALOS for
the Pressure driver.

``` python
import zmq
import time
import driver_pb2 as driver_proto # proto buffer precompiled

creator_ip = '127.0.0.1' # or local ip of MATRIX creator
creator_gpio_base_port = 20013 + 36

# connection to device
context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect('tcp://' + creator_ip + ':' + str(creator_gpio_base_port)) 

# instance for config driver message
config = driver_proto.DriverConfig()
config.gpio.pin = 15  # pin on board
config.gpio.mode = driver_proto.GpioParams.OUTPUT # pin mode OUTPUT/INPUT

while True:
    config.gpio.value ^= 1 # toggle value
    print ('GPIO'+str(config.gpio.pin)+'='+str(config.gpio.value))
    socket.send(config.SerializeToString()) # send proto message
    time.sleep(1)
```


## Driver details

### 0MQ Port
```
20049
```
### Protocol buffers

``` javascript
// GPIO handler params
message GpioParams {
  // GPIO to config
  uint32 pin = 1;

  // GPIO mode input/output
  enum EnumMode {
    INPUT = 0;
    OUTPUT = 1;
  }
  EnumMode mode = 2;

  // GPIO value
  uint32 value = 3;

  // GPIO all values
  uint32 values = 4;
}
```
The message is defined in [driver.proto](https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto).

### Keep-alives

This driver needs keep-alive messages [as specified in the MALOS protocol](https:////github.com/matrix-io/matrix-creator-malos/blob/master/README.md#keep-alive-port).
If you start sending keep-alive messages it will start returning data every second so you can omit the configuration for this device.


### Errors

This driver reports errors when an invalid configuration is sent.


### Write

All pins on matrix creator start as inputs. For change to outputs the driver need message for each pin on OUTPUT mode.


### Read

The driver will send a serialized message of integer *values* which reprensets of state from all GPIO pins [see figure 1](https://github.com/matrix-io/matrix-creator-malos/blob/av/doc_gpio/docs/gpio_diagram.jpg). For example: *values=5* represents *101* (pin 0 on 1, pin 1 on 0 and pin 2 on 1).



