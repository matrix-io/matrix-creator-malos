// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = '127.0.0.1'
var creator_info_base_port = 20012

var protoBuf = require("protobufjs")


// Parse proto file
var protoBuilder = protoBuf.loadProtoFile('../../protos/malos.proto')
// Parse matrix_malos package (namespace).
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')

var requestSocket = zmq.socket('req')

requestSocket.on("message", function(reply) {
    var proto_data = new matrixMalosBuilder.MalosDriverInfo.decode(reply)
    console.log(proto_data)
    process.exit(0)
});

requestSocket.connect('tcp://' + creator_ip + ':' + creator_info_base_port)
requestSocket.send('');
