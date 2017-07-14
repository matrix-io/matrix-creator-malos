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
# pip install pyzmq protobuf matrix_io-proto

# NOTE:
# THIS NEEDS TO BE TESTED! I DON'T HAVE A SERVO TO TEST ON!

import zmq
from matrix_io.proto.malos.v1 import driver_pb2


def send_servo_command(pin=4):
    # or local ip of MATRIX creator
    creator_ip = '127.0.0.1'
    creator_servo_base_port = 20013 + 32

    # Set a base count of 0
    count = 0

    # Grab a zmq context and set it up as a push socket, then connect
    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.connect('tcp://{0}:{1}'.format(creator_ip, creator_servo_base_port))

    # Create a new driver configuration
    servo_config = driver_pb2.DriverConfig()

    # Set a pin that the servo will operate on
    servo_config.servo.pin = pin

    # Start the loop of forever
    while True:
        # count mod 180 will set the angle of the servo to move to
        # this will change as the count increments to values of half turns
        angle = count % 180

        # Print out the angle to stdout
        print('Angle: {0}'.format(angle))

        # Set the servo's angle in the config
        servo_config.servo.angle = angle

        # since python isn't IO bound like node this will simulate
        # the event loop, updating the angle all the time but
        # only updating the servo every 1500000 'ticks'
        if count % 1500000 == 0:
            # Print message to stdout
            print('Sending new config to servo...{0}'.format(count))

            # Serialize the config and send it to the driver
            socket.send(servo_config.SerializeToString())

        # Increment the counter
        count += 1


if __name__ == '__main__':
    # Run everything!
    send_servo_command()
