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

import zmq
import time
import driver_pb2 as driver_proto

# or local ip of MATRIX creator
creator_ip = '192.168.1.154'
creator_base_port = 20012

# grab zmq context
context = zmq.Context()

# get a request socket from the context
req_socket = context.socket(zmq.REQ)

# connect to the socket using the cretor_ip and creator_base_port
req_socket.connect('tcp://{0}:{1}'.format(creator_ip, creator_base_port))

# send literally anything to get a response, this is a blocking operation
req_socket.send_string('')

# capture the message from the socket
message = req_socket.recv()

# parse data using the protocol buffers you compiled
proto_data = driver_proto.MalosDriverInfo().FromString(message)

# print the prased proto data to stdout
print(proto_data)
