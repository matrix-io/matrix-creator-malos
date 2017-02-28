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

creator_ip = '192.168.1.114' # or local ip of MATRIX creator
creator_everloop_base_port = 20013 + 8

context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect('tcp://' + creator_ip + ':' + str(creator_everloop_base_port)) 

def setEverloopColor(red=0, green=0, blue=0, white=0):
    config = driver_proto.DriverConfig()
    image = []
    for led in range (35):
        ledValue = driver_proto.LedValue()
        ledValue.blue = blue
        ledValue.red = red
        ledValue.green = green
        ledValue.white = white
        image.append(ledValue)

    config.image.led.extend(image) 
    socket.send(config.SerializeToString())

setEverloopColor(5,10,2,0)

