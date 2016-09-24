// NOTE: This file could be better. We know.

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
var create_zigbee_base_port = 20013 + 20 // port for Zigbee bulb driver.

var protoBuf = require("protobufjs");
var zmq = require('zmq')

var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

// Let's keep track of known bulbs so that we don't start background toggles
// more than once for a given bulb.
var knownBulbs = new Set()

// Print the errors that the driver sends.
var errorSocket = zmq.socket('sub')
errorSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 2))
errorSocket.subscribe('')
errorSocket.on('message', function(error_message) {
  process.stdout.write('Message received: Zigbee bulb error: ' + error_message.toString('utf8') + "\n")
});

var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + create_zigbee_base_port /* config */)

var config = new matrixMalosBuilder.DriverConfig
var bulb_cfg = new matrixMalosBuilder.ZigbeeBulbConfig
bulb_cfg.set_address('127.0.0.1')
bulb_cfg.set_port(31558)
config.set_zigbee_bulb(bulb_cfg)
config.set_delay_between_updates(0.2)
configSocket.send(config.encode().toBuffer());

var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 1))
process.stdout.write("Sending pings every 3 seconds");
pingSocket.send(''); // Ping the first time.
setInterval(function(){
  pingSocket.send('');
}, 3000);


var updateSocket = zmq.socket('sub')
updateSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 3))
updateSocket.subscribe('')
updateSocket.on('message', function(buffer) {
  var data = new matrixMalosBuilder.ZigBeeAnnounce.decode(buffer)

  console.log(data)
  // { short_id: 6058, cluster_id: 0, zdo_command: 0, zdo_status: 0 }

  if (!knownBulbs.has(data.short_id)) {
      setInterval(function() {
        var bulbCmd = new matrixMalosBuilder.ZigBeeBulbCmd
        bulbCmd.short_id = data.short_id
        // Check the message ZigBeeBulbCmd for the available commands. At the moment,
        // only ON, OFF and TOGGLE are supported. This should change soon.
        // https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto
        bulbCmd.command = matrixMalosBuilder.ZigBeeBulbCmd.EnumCommands.TOGGLE
        console.log('toggle')
        var bulb_cfg_cmd = new matrixMalosBuilder.ZigbeeBulbConfig
        bulb_cfg_cmd.set_address('')
        bulb_cfg_cmd.set_port(-1)
        bulb_cfg_cmd.set_command(bulbCmd)

        var config = new matrixMalosBuilder.DriverConfig
        config.set_delay_between_updates(0.2)
        config.set_zigbee_bulb(bulb_cfg_cmd)
        configSocket.send(config.encode().toBuffer());
      }, 2000);
  }

  knownBulbs.add(data.short_id)
});
