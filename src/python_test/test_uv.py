# This is how we connect to the creator. IP and port.
# The IP is the IP I'm using and you need to edit it.
# By default, MALOS has its 0MQ ports open to the world.
#
# (see README file for more details)

import os
import zmq
import time
from matrix_io.proto.malos.v1 import sense_pb2

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import driver_keep_alive, register_data_callback, register_error_callback

# local ip of MATRIX creator
creator_ip = os.environ.get('CREATOR_IP', '127.0.0.1')
uv_port = 20013 + 16


def uv_data_callback(data):
    """Capture data and print it to stdout"""
    uv_info = sense_pb2.UV().FromString(data[0])
    print('{0}'.format(uv_info))


def uv_error_callback(error):
    """Capture error and print it to stdout"""
    print('{0}'.format(error))


if __name__ == '__main__':
    # Instiate a new ioloop
    ioloop.install()

    # Register the data callback in a new process
    Process(target=register_data_callback, args=(uv_data_callback, creator_ip, uv_port)).start()

    # Register the error callback in a new process
    Process(target=register_error_callback, args=(uv_error_callback, creator_ip, uv_port)).start()

    # Register the keep alive driver in a new process
    Process(target=driver_keep_alive, args=(creator_ip, uv_port)).start()
