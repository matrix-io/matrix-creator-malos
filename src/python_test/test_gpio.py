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
# (see README file for more details)

# NOTE:
# before run this example please execute:
# pip install pyzmq protobuf matrix_io-proto

import zmq
import time
from matrix_io.proto.malos.v1 import driver_pb2
from matrix_io.proto.malos.v1 import io_pb2

# or local ip of MATRIX creator
creator_ip = '127.0.0.1'
creator_gpio_base_port = 20013 + 36

# Grab a zmq context
context = zmq.Context()

# Create a zmq push socket
socket = context.socket(zmq.PUSH)

# Connect to push socket
socket.connect('tcp://{0}:{1}'.format(creator_ip, creator_gpio_base_port))

# Create a new driver config
config = driver_pb2.DriverConfig()

# Set pin number to control
config.gpio.pin = 15

# Set pin 15 to output mode
config.gpio.mode = io_pb2.GpioParams.OUTPUT

# Start the unescapable loop!
while True:

    # Bit shift the value on the pin
    # from high to low to high to low...
    config.gpio.value ^= 1

    # Print some debug statements
    print ('GPIO{0}={1}'.format(config.gpio.pin, config.gpio.value))

    # Serialize the configuration we created
    # and send it to the socket
    socket.send(config.SerializeToString())

    # Nap time
    time.sleep(1)
