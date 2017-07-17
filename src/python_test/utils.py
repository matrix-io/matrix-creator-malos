import zmq
import time

from zmq.eventloop import ioloop, zmqstream

"""
This module contains some utility functions to help when using the lower level
zmq messaging protocol malOS exposes.  There are three functions below, one to
register a data callback, one to register an error callback, and another to
register a keep alive.  The main difference between these functions is the port
I conntect to since there are tcp ports for errors, data, and keep alive.
"""


def register_data_callback(callback, creator_ip, sensor_port):
    """Accepts a function to run when malOS zqm driver pushes an update"""

    # Grab a zmq context, as per usual, connect to it, but make it a SUBSCRIPTION this time
    context = zmq.Context()
    socket = context.socket(zmq.SUB)

    # Connect to the base sensor port provided in the args + 3 for the data port
    data_port = sensor_port + 3

    # Connect to the data socket
    socket.connect('tcp://{0}:{1}'.format(creator_ip, data_port))

    # Set socket options to subscribe and send off en empty string to let it know we're ready
    socket.setsockopt(zmq.SUBSCRIBE, b'')

    # Create the stream to listen to
    stream = zmqstream.ZMQStream(socket)

    # When data comes across the stream, execute the callback with it's contents as parameters
    stream.on_recv(callback)

    # Print some debug information
    print('Connected to data publisher with port {0}'.format(data_port))

    # Start a global IO loop from tornado
    ioloop.IOLoop.instance().start()
    print('Worker has stopped processing messages.')


def register_error_callback(callback, creator_ip, sensor_port):
    """Accepts a function to run when the malOS zqm driver pushes an error"""

    # Grab a zmq context, as per usual, connect to it, but make it a SUBSCRIPTION this time
    context = zmq.Context()
    socket = context.socket(zmq.SUB)

    # Connect to the base sensor port provided in the args + 2 for the error port
    error_port = sensor_port + 2

    # Connect to the data socket
    socket.connect('tcp://{0}:{1}'.format(creator_ip, error_port))

    # Set socket options to subscribe and send off en empty string to let it know we're ready
    socket.setsockopt(zmq.SUBSCRIBE, b'')

    # Create a stream to listen to
    stream = zmqstream.ZMQStream(socket)

    # When data comes across the stream, execute the callback with it's contents as parameters
    stream.on_recv(callback)

    # Print some debug information
    print('Connected to error publisher with port {0}'.format(error_port))

    #Start a global IO loop from tornado
    ioloop.IOLoop.instance().start()
    print('Worker has stopped processing messages.')


def driver_keep_alive(creator_ip, sensor_port, ping=5):
    """
    This doesn't take a callback function as it's purpose is very specific.
    This will ping the driver every n seconds to keep the driver alive and sending updates
    """

    # Grab zmq context
    context = zmq.Context()

    # Set up socket as a push
    sping = context.socket(zmq.PUSH)

    # Set the keep alive port to the sensor port from the function args + 1
    keep_alive_port = sensor_port + 1

    # Connect to the socket
    sping.connect('tcp://{0}:{1}'.format(creator_ip, keep_alive_port))

    # Start a forever loop
    while True:
        # Ping with empty string to let the drive know we're still listening
        sping.send_string('')

        # Delay between next ping
        time.sleep(ping)
