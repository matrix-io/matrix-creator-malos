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
# pip install pyzmq protobuf

# and then compile protos like this:
# export SRC_DIR=../../protocol-buffers/malos
# protoc -I=$SRC_DIR --python_out=./ $SRC_DIR/driver.proto

# NOTE:
# there seems to be a delay when running this at high framerates
# look into using queues to pass messages between threads

import zmq
import time
import driver_pb2 as driver_proto

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import register_error_callback

ioloop.install()

# or local ip of MATRIX creator
creator_ip = '127.0.0.1'

# port for everloop driver
creator_everloop_base_port = 20013 + 8


def led_error_callback(error):
    print('{0}'.format(error))


# sets all of the LEDS to a given rgbw value
def set_everloop_intensity(intensity):
    # grab zmq context
    context = zmq.Context()

    # get socket for config
    config_socket = context.socket(zmq.PUSH)
    config_socket.connect(
        'tcp://{0}:{1}'.format(creator_ip, creator_everloop_base_port))

    # create a new driver config strut
    config = driver_proto.DriverConfig()

    # initialize an empty list for the "image" or LEDS
    image = []

    # iterate over all 35 LEDS and set the rgbw value of each
    # then append it to the end of the list/image thing
    for led in range(35):
        ledValue = driver_proto.LedValue()
        ledValue.blue = 0
        ledValue.red = 0
        ledValue.green = intensity
        ledValue.white = 0
        image.append(ledValue)

    # add the "image" to the config driver
    config.image.led.extend(image)

    # send a serialized string of the driver config
    # to the config socket
    config_socket.send(config.SerializeToString())


def animation(max_intensity=50):
    intensity_value = max_intensity

    set_everloop_intensity(intensity_value)
    while True:
        intensity_value -= 1
        if intensity_value < 0:
            intensity_value = max_intensity
        set_everloop_intensity(intensity_value)
        print('Intensity: {0}'.format(intensity_value))
        time.sleep(0.5)

if __name__ == '__main__':
    Process(target=register_error_callback, args=(
        led_error_callback, creator_ip, creator_everloop_base_port)).start()
    Process(target=animation, args=(40,)).start()
