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

# NOTE:
# THIS NEEDS TO BE TESTED! I DON'T HAVE A SERVO TO TEST ON!

import zmq
import time
import driver_pb2 as driver_proto


def send_servo_command(pin=4):
    # or local ip of MATRIX creator
    creator_ip = '127.0.0.1'
    creator_servo_base_port = 20013 + 32

    count = 0

    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.connect('tcp://{0}:{1}'.format(creator_ip, creator_servo_base_port))

    servo_config = driver_proto.DriverConfig()
    servo_config.servo.pin = pin

    while True:
        angle = count % 180
        #print('Angle: {0}'.format(angle))
        servo_config.servo.angle = angle
        # since python isn't IO bound like node this will simulate
        # the event loop, updating the angle all the time but
        # only updating the servo every 1500000 'ticks'
        if count % 1500000 == 0:
            print('Sending new config to servo...{0}'.format(count))
            socket.send(servo_config.SerializeToString())
        count += 1


if __name__ == '__main__':
    send_servo_command()
