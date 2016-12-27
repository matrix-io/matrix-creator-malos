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
var sleep = require('sleep')
var time = require('time')


var zigbee_network_up = false
var gateway_up = false
var attemps = 10
var device_detected = false 


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

// ------------ Starting to ping the driver -----------------------
var pingSocket = zmq.socket('push')
pingSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 1))
// console.log('Sending pings every 5 seconds');
pingSocket.send(''); // Ping the first time.
setInterval(function(){
  pingSocket.send('');
}, 1000);


var subSocket = zmq.socket('sub')
subSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 3))
subSocket.subscribe('')
subSocket.on('message', function(buffer) {

  var zig_msg = new matrixMalosBuilder.ZigBeeMsg.decode(buffer)

  if(zig_msg.type == matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.ZCL){

  } else if (matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT){
    
    switch(zig_msg.network_mgmt_cmd.type){
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.CREATE_NWK:
        console.log('CREATE_NWK');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.LEAVE_NWK:
        console.log('LEAVE_NWK');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NODE_LEAVE_NWK:
        console.log('NODE_LEAVE_NWK');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.PERMIT_JOIN:
        console.log('PERMIT_JOIN');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NODE_INFO:
        console.log('NODE_INFO');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.DISCOVERY_INFO:
        console.log('DISCOVERY_INFO');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.RESET_PROXY:
        console.log('RESET_PROXY');
      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.IS_PROXY_ACTIVE:
        gateway_up = true; //zig_msg.network_mgmt_cmd.is_proxy_activee;
        console.log('Gateway connected');
        if(zigbee_network_up == false) {
          console.log('Requesting ZigBee Network Status');
          config.zigbee_message.network_mgmt_cmd.set_type(
            matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NETWORK_STATUS)
          configSocket.send(config.encode().toBuffer());
        }

        setTimeout(function(){
          if (!zigbee_network_up) {
            console.log('Zigbee Network not working.');
            process.exit(1);  
          }
        },1000);

      break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NETWORK_STATUS:
        console.log('NETWORK_STATUS');

        
      break;
    }  
  } else if (matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NETWORK_MGMT){
    
  }

});

ResetGateway();






function ResetGateway(){
  // ------- Setting the delay_between_updates and set_timeout_after_last_ping ---------
  var config = new matrixMalosBuilder.DriverConfig
  console.log('Setting the Zigbee Driver');
  config.set_delay_between_updates(1)
  config.set_timeout_after_last_ping(1)
  configSocket.send(config.encode().toBuffer());

  // ------------ Creating the base proto zigbee message -----------------------
  config = new matrixMalosBuilder.DriverConfig
  var zig_msg = new matrixMalosBuilder.ZigBeeMsg
  var network_mgmt_cmd = new matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd
  zig_msg.set_network_mgmt_cmd(network_mgmt_cmd)
  config.set_zigbee_message(zig_msg)

  // ------------ Reseting the Gateway App -----------------------
  console.log('Reseting the Gateway App');
  config.zigbee_message.set_type(matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT);
  config.zigbee_message.network_mgmt_cmd.set_type(
    matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.RESET_PROXY)
  configSocket.send(config.encode().toBuffer());

  // ------------ Checking connection with the Gateway ----------------------
  console.log('Checking connection with the Gateway');
  config.zigbee_message.network_mgmt_cmd.set_type(
    matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.IS_PROXY_ACTIVE)
  configSocket.send(config.encode().toBuffer());

  // ------------- Exit if not connected ----------------------------
  setTimeout(function(){
    if (!gateway_up) {
      console.log('Gateway not connected');
      process.exit(1);  
    }
  },1000);

}

// if(!zigbee_network_up){

//   // ------------ Creating a ZigBee Network -------------------------------
//   console.log('Create a ZigBee Network');
//   config.zigbee_message.network_mgmt_cmd.set_type(
//   matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.CREATE_NWK)
//   configSocket.send(config.encode().toBuffer());
// }

// sleep.sleep(2);
// // ------------ Checking if a Zigbee Network exist ------ ---------------
// console.log('Requesting ZigBee NETWORK_STATUS');
// config.zigbee_message.network_mgmt_cmd.set_type(
//   matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.NETWORK_STATUS)
// configSocket.send(config.encode().toBuffer());

// console.log('Waiting 2 sec for Network Status ..');
// sleep.sleep(2);

// if(!zigbee_network_up){
//   console.log('Error creating ZigBee Network ');
//   process.exit(1);  
// }

// console.log('ZigBee Network Up');

// // ------------ Making the Network joinable -----------------------------
// console.log('Open the Network for new devices to Join for 30 sec');
// config.zigbee_message.network_mgmt_cmd.set_type(
//   matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes.PERMIT_JOIN)
// configSocket.send(config.encode().toBuffer());

// var i = 10 
// while(!device_detected && (i-- > 0)){
//   console.log('Waiting for devices ... please turn one the device');
//   sleep.sleep(3);
// }
// if(i == 0){
//   console.log('No devices found .');
//   process.exit(1);  
// }
//   console.log('No devices found .');




// ------------ Waiting for new devices ---------------------------------
// ------------ Identify devices on the Network -------------------------
// ------------ Selecting one device and change toggle ------------------

