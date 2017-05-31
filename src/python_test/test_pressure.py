# This is how we connect to the creator. IP and port.
# The IP is the IP I'm using and you need to edit it.
# By default, MALOS has its 0MQ ports open to the world.
#
# (see README file for more details)

import zmq
import time
import driver_pb2 as driver_proto

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import driver_keep_alive, register_data_callback, register_error_callback

ioloop.install()

creator_ip = '127.0.0.1'  # or local ip of MATRIX creator
pressure_port = 20013 + 12


def pressure_data_callback(data):
    pressure_info = driver_proto.Pressure().FromString(data[0])
    print('{0}'.format(pressure_info))


def pressure_error_callback(error):
    print('{0}'.format(error))


if __name__ == '__main__':
    Process(target=register_data_callback, args=(
        pressure_data_callback, creator_ip, pressure_port)).start()
    Process(target=register_error_callback, args=(
        pressure_error_callback, creator_ip, pressure_port)).start()
    Process(target=driver_keep_alive, args=(creator_ip, pressure_port)).start()
