# This is how we connect to the creator. IP and port.
# The IP is the IP I'm using and you need to edit it.
# By default, MALOS has its 0MQ ports open to the world.
#
# (see README file for more details)

import os
from matrix_io.proto.malos.v1 import sense_pb2

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import driver_keep_alive, register_data_callback, register_error_callback

# or local ip of MATRIX creator
creator_ip = os.environ.get('CREATOR_IP', '127.0.0.1')
pressure_port = 20013 + 12


def pressure_data_callback(data):
    """Callback that prints new data updates to stdout"""
    pressure_info = sense_pb2.Pressure().FromString(data[0])
    print('{0}'.format(pressure_info))


def pressure_error_callback(error):
    """Callback that prints error messages to stdout"""
    print('{0}'.format(error))


if __name__ == '__main__':
    # Instantiate ioloop
    ioloop.install()

    # Start a process to get the data
    Process(target=register_data_callback, args=(pressure_data_callback, creator_ip, pressure_port)).start()

    # Start a process to get the error messages
    Process(target=register_error_callback, args=(pressure_error_callback, creator_ip, pressure_port)).start()

    # Start a process to keep the driver alive
    Process(target=driver_keep_alive, args=(creator_ip, pressure_port)).start()
