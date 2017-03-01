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
# pip install pyzmq protobuf tornado

# and then compile protos like this:
# export SRC_DIR=../../protocol-buffers/malos
# protoc -I=$SRC_DIR --python_out=./ $SRC_DIR/driver.proto

import zmq
import time
import driver_pb2 as driver_proto
from  multiprocessing import Process
from zmq.eventloop import ioloop, zmqstream
ioloop.install()

creator_ip = '127.0.0.1' # or local ip of MATRIX creator

creator_gpio_base_port = 20013 + 36

def config_gpio_write(pin,value):
    config = driver_proto.DriverConfig()
    config.gpio.pin = pin
    config.gpio.mode = driver_proto.GpioParams.OUTPUT 
    config.gpio.value = value
    sconfig.send(config.SerializeToString())

def config_gpio_read(pin):
    config = driver_proto.DriverConfig()
    # 250 miliseconds between updates.
    config.delay_between_updates = 0.5
    # Stop sending updates 2 seconds after pings.
    config.timeout_after_last_ping = 3.5
    config.gpio.pin = pin
    config.gpio.mode = driver_proto.GpioParams.INPUT
    sconfig.send(config.SerializeToString())

def gpio_callback(msg):
    data = driver_proto.GpioParams().ParseFromString(msg[0])
    print "Received gpio register: %s" % data

def register_gpio_callback():
    ssub = context.socket(zmq.SUB)
    ssub_port = str(creator_gpio_base_port+3)
    ssub.connect('tcp://' + creator_ip + ':' + ssub_port) 
    ssub.setsockopt(zmq.SUBSCRIBE,"")
    stream = zmqstream.ZMQStream(ssub)
    stream.on_recv(gpio_callback)
    print "Connected to publisher with port %s" % ssub_port
    ioloop.IOLoop.instance().start()
    print "Worker has stopped processing messages."

def task_gpio_write(pin):
    pin_value = 0
    while True:
        pin_value ^= 1
        config_gpio_write(pin,pin_value)
        print ('GPIO:'+str(pin)+' => '+str(pin_value))
        time.sleep(1)

def task_driver_ping():
    context = zmq.Context()
    sping = context.socket(zmq.PUSH)
    sping.connect('tcp://' + creator_ip + ':' + str(creator_gpio_base_port + 1))
    while True:
        sping.send('')
        time.sleep(1)

if __name__ == "__main__":
    context = zmq.Context()
    sconfig = context.socket(zmq.PUSH)
    sconfig.connect('tcp://' + creator_ip + ':' + str(creator_gpio_base_port))

    config_gpio_write(0,0) # pin 0 in output mode, value 0
    config_gpio_read(1)    # pin 1 in input mode

    Process(target = task_gpio_write, args = (0, )).start()
    Process(target = task_driver_ping).start()

    register_gpio_callback()

