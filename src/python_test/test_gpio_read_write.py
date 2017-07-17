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
# pip install pyzmq protobuf tornado matrix_io-proto

import os
import zmq
import time
from matrix_io.proto.malos.v1 import driver_pb2
from matrix_io.proto.malos.v1 import io_pb2

from multiprocessing import Process
from zmq.eventloop import ioloop, zmqstream

from utils import driver_keep_alive, register_data_callback, register_error_callback

creator_ip = os.environ.get('CREATOR_IP', '127.0.0.1')
creator_gpio_base_port = 20013 + 36


def config_gpio_write(pin, value):
    """This function sets up a pin for use as an output pin"""

    # Create a new driver config
    config = driver_pb2.DriverConfig()

    # set the pin in the config provided from the function params
    config.gpio.pin = pin

    # Set pin mode to output
    config.gpio.mode = io_pb2.GpioParams.OUTPUT

    # Set the output of the pin initially
    config.gpio.value = value

    # Send configuration to malOS using global sconfig
    sconfig.send(config.SerializeToString())


def config_gpio_read(pin):
    """This function sets up a pin for use as an input pin"""

    # Create a new driver config
    config = driver_pb2.DriverConfig()

    # Set 250 miliseconds between updates.
    config.delay_between_updates = 0.5

    # Stop sending updates 2 seconds after pings.
    config.timeout_after_last_ping = 3.5

    # Set the pin to the value provided by the function param
    config.gpio.pin = pin

    # Set the pin mode to input
    config.gpio.mode = io_pb2.GpioParams.INPUT

    # Send configuration to malOS using global sconfig
    sconfig.send(config.SerializeToString())


def gpio_callback(msg):
    """Captures an error message and prints it to stdout"""
    data = io_pb2.GpioParams().FromString(msg[0])
    print('Received gpio register: {0}'.format(data))


def task_gpio_write(pin):
    """This flips the bit from high to low to high to low..."""

    # Set initial pin value LOW
    pin_value = 0

    # Start the everlasting loop
    while True:
        # Bit shift the pin_value
        pin_value ^= 1

        # Invoke the config and write function
        config_gpio_write(pin, pin_value)

        # Print some debug stuff
        print ('GPIO:{0}=>{1}'.format(pin, pin_value))

        # Sleep for 1 second before continuing the loop
        time.sleep(1)

if __name__ == "__main__":
    # Instantiate an ioloop
    ioloop.install()

    #Grab zmq context
    context = zmq.Context()

    # Create sconfig object that will be used by all subsequent functions
    # Essentially a gloabl
    sconfig = context.socket(zmq.PUSH)
    sconfig.connect('tcp://{0}:{1}'.format(creator_ip, creator_gpio_base_port))

    # pin 0 in output mode, value 0
    config_gpio_write(0, 0)

    # pin 1 in input mode
    config_gpio_read(1)

    # Start the process of shifting the bit on pin 0
    Process(target=task_gpio_write, args=(0, )).start()

    # Start up a process to keep the driver alive and sending updates
    Process(target=driver_keep_alive, args=(creator_ip, creator_gpio_base_port, 1)).start()

    # Register the callback to send data from the read pin
    Process(target=register_data_callback, args=(gpio_callback, creator_ip, creator_gpio_base_port)).start()
