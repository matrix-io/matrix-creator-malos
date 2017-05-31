import zmq
import time

from zmq.eventloop import ioloop, zmqstream


def register_data_callback(callback, creator_ip, sensor_port):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    data_port = sensor_port + 3
    socket.connect('tcp://{0}:{1}'.format(creator_ip, data_port))
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    stream = zmqstream.ZMQStream(socket)
    stream.on_recv(callback)
    print('Connected to data publisher with port {0}'.format(data_port))
    ioloop.IOLoop.instance().start()
    print('Worker has stopped processing messages.')


def register_error_callback(callback, creator_ip, sensor_port):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    error_port = sensor_port + 2
    socket.connect('tcp://{0}:{1}'.format(creator_ip, error_port))
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    stream = zmqstream.ZMQStream(socket)
    stream.on_recv(callback)
    print('Connected to error publisher with port {0}'.format(error_port))
    ioloop.IOLoop.instance().start()
    print('Worker has stopped processing messages.')


def driver_keep_alive(creator_ip, sensor_port, ping=5):
    context = zmq.Context()
    sping = context.socket(zmq.PUSH)
    keep_alive_port = sensor_port + 1
    sping.connect('tcp://{0}:{1}'.format(creator_ip, keep_alive_port))
    while True:
        sping.send_string('')
        time.sleep(ping)
