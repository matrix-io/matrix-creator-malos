# This is how we connect to the creator. IP and port.
# You can define the CREATOR_IP env var to use a different host
# By default, MALOS has its 0MQ ports open to the world.
#
# (see README file for more details)

import os
from matrix_io.proto.malos.v1 import sense_pb2

from multiprocessing import Process
from zmq.eventloop import ioloop

from utils import driver_keep_alive, register_data_callback, register_error_callback

# remote or local ip of MATRIX creator
creator_ip = os.environ.get('CREATOR_IP', '127.0.0.1')
imu_port = 20013


def imu_data_callback(data):
    """Callback that prints new data updates to stdout"""
    imu_info = sense_pb2.Imu().FromString(data[0])
    print('{0}'.format(imu_info))


def imu_error_callback(error):
    """Callback that prints error messages to stdout"""
    print('{0}'.format(error))


if __name__ == '__main__':
    # Instantiate ioloop
    ioloop.install()

    # Start a process to get the data
    Process(target=register_data_callback, args=(imu_data_callback, creator_ip, imu_port)).start()

    # Start a process to get the error messages
    Process(target=register_error_callback, args=(imu_error_callback, creator_ip, imu_port)).start()

    # Start a process to keep the driver alive
    Process(target=driver_keep_alive, args=(creator_ip, imu_port)).start()
