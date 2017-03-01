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

import zmq
import time
import driver_pb2 as driver_proto
from  multiprocessing import Process
from zmq.eventloop import ioloop, zmqstream
ioloop.install()

creator_ip = '10.0.0.171' # or local ip of MATRIX creator

creator_base_port = 20013
pressure_port = creator_base_port + 12
uv_port = creator_base_port + 16
humidity_port = creator_base_port + 4

def humidity_callback(msg):
    data = driver_proto.Humidity().ParseFromString(msg[0])
    print "humidity: %s" % data

def pressure_callback(msg):
    data = driver_proto.Pressure().ParseFromString(msg[0])
    print "pressure: %s" % data

def uv_callback(msg):
    data = driver_proto.UV().ParseFromString(msg[0])
    print "uv: %s" % data

def register_callback(callback, sensor_port):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    ssub_port = str(sensor_port+3)
    socket.connect('tcp://' + creator_ip + ':' + ssub_port) 
    socket.setsockopt(zmq.SUBSCRIBE,"")
    stream = zmqstream.ZMQStream(socket)
    stream.on_recv(callback)
    print "Connected to publisher with port %s" % ssub_port
    ioloop.IOLoop.instance().start()
    print "Worker has stopped processing messages."

def task_driver_ping(sensor_port):
    context = zmq.Context()
    sping = context.socket(zmq.PUSH)
    sping.connect('tcp://' + creator_ip + ':' + str(sensor_port + 1))
    while True:
        sping.send('')
        time.sleep(3)

if __name__ == "__main__":
    Process(target = register_callback, args = (humidity_callback,humidity_port,)).start()
    Process(target = register_callback, args = (pressure_callback,pressure_port,)).start()
    Process(target = register_callback, args = (uv_callback,uv_port,)).start()
    Process(target = task_driver_ping, args = (humidity_port,)).start()
    Process(target = task_driver_ping, args = (pressure_port,)).start()
    Process(target = task_driver_ping, args = (uv_port,)).start()
