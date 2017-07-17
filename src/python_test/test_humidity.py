# This is how we connect to the creator. IP and port.
# The IP is the IP I'm using and you need to edit it.
# By default, MALOS has its 0MQ ports open to the world.
#
# (see README file for more details)

import os
import zmq
import time
from matrix_io.proto.malos.v1 import driver_pb2
from matrix_io.proto.malos.v1 import sense_pb2

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import driver_keep_alive, register_data_callback, register_error_callback

# or local ip of MATRIX creator
creator_ip = os.environ.get('CREATOR_IP', '127.0.0.1')

humidity_port = 20013 + 4


def config_socket():
    """Configure and calibrate the humidity driver"""

    # Grab the zmq context and set it to push, then connect to it
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.connect('tcp://{0}:{1}'.format(creator_ip, humidity_port))

    # Create a new driver config
    driver_config_proto = driver_pb2.DriverConfig()

    # Set the delay between updates that the driver returns
    driver_config_proto.delay_between_updates = 2.0

    # Stop sending updates if there is no ping for 6 seconds
    driver_config_proto.timeout_after_last_ping = 6.0

    # Calibrate the temperature by taking a real world
    # measurment from a thermometer and enter it in here
    # in degrees celcius
    driver_config_proto.humidity.current_temperature = 23

    # Serialize the config and send it to the config socket
    socket.send(driver_config_proto.SerializeToString())


def humidity_data_callback(data):
    """Capture any data and print them to stdout"""
    humidity_info = sense_pb2.Humidity().FromString(data[0])
    print('{0}'.format(humidity_info))


def humidity_error_callback(error):
    """Capture any errors and send them to stdout"""
    print('{0}'.format(error))


if __name__ == '__main__':
    # Instiate ioloop
    ioloop.install()

    # Run the configure socket function to  calibrate the sensor
    config_socket()

    # Start a new process that captures and prints data to stdout
    Process(target=register_data_callback, args=(humidity_data_callback, creator_ip, humidity_port)).start()

    # Start another new process to capture errors to stdout
    Process(target=register_error_callback, args=(humidity_error_callback, creator_ip, humidity_port)).start()

    # Start one more new process to keep the driver alive
    # If this thread in particular is killed, 6 seconds later
    # the driver will stop sending updates due to out config
    Process(target=driver_keep_alive, args=(creator_ip, humidity_port)).start()
