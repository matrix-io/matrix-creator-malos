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

// Print the errors that the driver sends.
var errorSocket = zmq.socket('sub')
errorSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 2))
errorSocket.subscribe('')
errorSocket.on('message', function(error_message) {
  process.stdout.write('Message received: ' + error_message.toString('utf8') + "\n")
});

// ------------ Create the socket for sending data to the driver ----------
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + create_zigbee_base_port /* config */)

var config = new matrixMalosBuilder.DriverConfig;

// ------- Setting the delay_between_updates and set_timeout_after_last_ping ---------
config.set_delay_between_updates(1)
config.set_timeout_after_last_ping(1)

// // ------------ Reseting the Gateway App -----------------------
config = new matrixMalosBuilder.DriverConfig;
var zig_msg = new matrixMalosBuilder.ZigBeeMsg;
zig_msg.set_type(matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT);
zig_msg.network_mgmt_cmd = new matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd;
zig_msg.network_mgmt_cmd.set_type(
  matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.RESET_PROXY)

config.set_zigbee_message(zig_msg)
configSocket.send(config.encode().toBuffer());

// ------------ Starting to ping the driver -----------------------
var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 1))
console.log('Sending pings every 5 seconds');
pingSocket.send(''); // Ping the first time.
setInterval(function(){
  pingSocket.send('');
// console.log(' ... ping');
}, 5000);

// ------------ Checking if a Zigbee Network exist ------ ---------------
var zig_msg = new matrixMalosBuilder.ZigBeeMsg;
zig_msg.set_type(matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT);
zig_msg.network_mgmt_cmd = new matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd;
zig_msg.network_mgmt_cmd.set_type(
matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NETWORK_STATUS)
config.set_zigbee_message(zig_msg)
configSocket.send(config.encode().toBuffer());


// // ------------ Creating a ZigBee Network -------------------------------
// var zig_msg = new matrixMalosBuilder.ZigBeeMsg;
// zig_msg.set_type(matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT);
// zig_msg.network_mgmt_cmd = new matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd;
// zig_msg.network_mgmt_cmd.set_type(
// matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NODE_INFO)
// config.set_zigbee_message(zig_msg)
// configSocket.send(config.encode().toBuffer());

// ------------ Making the Network joinable -----------------------------
// ------------ Waiting for new devices ---------------------------------
// ------------ Identify devices on the Network -------------------------
// ------------ Selecting one device and change toggle ------------------


// var subSocket = zmq.socket('sub')
// subSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 3))
// subSocket.subscribe('')
// subSocket.on('message', function(buffer) {

//   console.log('console : Protobuf Message received');

//   var zigBeeMsg = new matrixMalosBuilder.ZigBeeMsg.decode(buffer)

//   if(zigBeeMsg.type)

  // if (!knownBulbs.has(zigBeeMsg.short_id)) {
  //     setInterval(function() {
  //       var bulbCmd = new matrixMalosBuilder.ZigBeeBulbCmd
  //       bulbCmd.short_id = zigBeeMsg.short_id
  //       // Check the message ZigBeeBulbCmd for the available commands.
  //       // https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto
  //       bulbCmd.command = matrixMalosBuilder.ZigBeeBulbCmd.EnumCommands.TOGGLE
  //       console.log('toggle')
  //       var bulb_cfg_cmd = new matrixMalosBuilder.ZigbeeBulbConfig
  //       bulb_cfg_cmd.set_address('')
  //       bulb_cfg_cmd.set_port(-1)
  //       bulb_cfg_cmd.set_command(bulbCmd)

  //       var config = new matrixMalosBuilder.DriverConfig
  //       config.set_delay_between_updates(0.2)
  //       config.set_zigbee_bulb(bulb_cfg_cmd)
  //       configSocket.send(config.encode().toBuffer());
  //     }, 2000);
  // }

//   knownBulbs.add(zigBeeMsg.short_id)
// });
