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

creator_ip = '192.168.1.154'  # or local ip of MATRIX creator
humidity_port = 20013 + 4


def config_socket():
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.connect('tcp://{0}:{1}'.format(creator_ip, humidity_port))

    driver_config_proto = driver_proto.DriverConfig()
    driver_config_proto.delay_between_updates = 2.0
    driver_config_proto.timeout_after_last_ping = 6.0

    driver_config_proto.humidity.current_temperature = 23

    socket.send(driver_config_proto.SerializeToString())


def humidity_data_callback(data):
    humidity_info = driver_proto.Humidity().FromString(data[0])
    print('{0}'.format(humidity_info))


def humidity_error_callback(error):
    print('{0}'.format(error))


if __name__ == '__main__':
    config_socket()
    Process(target=register_data_callback, args=(
        humidity_data_callback, creator_ip, humidity_port)).start()
    Process(target=register_error_callback, args=(
        humidity_error_callback, creator_ip, humidity_port)).start()
    Process(target=driver_keep_alive, args=(creator_ip, humidity_port)).start()
