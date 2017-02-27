# This is how we connect to the creator. IP and port.
# The IP is the IP I'm using and you need to edit it.
# By default, MALOS has its 0MQ ports open to the world.
#
# Every device is identified by a base port. Then the mapping works
# as follows:
# BasePort     => Configuration port. Used to config the device.
# BasePort + 1 => Keepalive port. Send pings to this port.
# BasePort + 2 => Error port. Receive errros from device.
# BasePort + 3 => Data port. Receive data from device.

# NOTE: 
# before run this example please execute:
# pip install pyzmq protobuf 

# and then compile protos like this:
# export SRC_DIR=../../protocol-buffers/malos
# protoc -I=$SRC_DIR --python_out=./ $SRC_DIR/driver.proto

import zmq
import time
import driver_pb2 as driver_proto

creator_ip = '10.0.0.198' # or local ip of MATRIX creator
creator_gpio_base_port = 20013 + 36

context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect('tcp://' + creator_ip + ':' + str(creator_gpio_base_port)) 

config = driver_proto.DriverConfig()
config.gpio.pin = 15
config.gpio.mode = driver_proto.GpioParams.OUTPUT 

while True:
    config.gpio.value ^= 1
    socket.send(config.SerializeToString())
    time.sleep(1)

