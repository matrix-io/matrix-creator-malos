// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = process.env.CREATOR_IP || '127.0.0.1';
var creator_info_base_port = 20012

var zmq = require('zmq')
var requestSocket = zmq.socket('req')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io


requestSocket.on('message', (buffer) => {
    var msg_inst = matrix_io.malos.v1.driver.MalosDriverInfo.decode(buffer)
    console.log(msg_inst)
    process.exit(0)
});

requestSocket.connect('tcp://' + creator_ip + ':' + creator_info_base_port)
requestSocket.send('');
