// Warning! This is returning 0's.
// Missing low level logic. We're on it.

// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.

// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.

var creator_ip = process.env.CREATOR_IP || '127.0.0.1'
var creator_micarray_base_port = 20037

var zmq = require('zmq')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io


// ********** Start error management.
var errorSocket = zmq.socket('sub')
errorSocket.connect('tcp://' + creator_ip + ':' + (creator_micarray_base_port + 2))
errorSocket.subscribe('')
errorSocket.on('message', (error_message) => {
  console.log('Message received: DOA error: ', error_message.toString('utf8'))
});
// ********** End error management.


// ********** Start configuration.
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_micarray_base_port)
// Send driver configuration.
var config = matrix_io.malos.v1.driver.DriverConfig.create({
  delayBetweenUpdates: 1.0, // 1 seconds between updates
  timeoutAfterLastPing: 6.0, // Stop sending updates 6 seconds after pings
});
configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())
// ********** End configuration.

// ********** Start updates - Here is where they are received.
var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (creator_micarray_base_port + 3))
updateSocket.subscribe('')
updateSocket.on('message', (buffer) => {
  var data = matrix_io.malos.v1.io.MicArrayParams.decode(buffer)
  console.log('azimutal_angle (degrees) = ', data.azimutalAngle*180.0/Math.PI);
  console.log('   polar_angle (degrees) = ', data.polarAngle*180.0/Math.PI); 
});
// ********** End updates

// ********** Ping the driver
var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (creator_micarray_base_port + 1))
console.log("Sending pings every 5 seconds");
pingSocket.send(''); // Ping the first time.
setInterval(() => {
  pingSocket.send('');
}, 5000);
// ********** Ping the driver ends
