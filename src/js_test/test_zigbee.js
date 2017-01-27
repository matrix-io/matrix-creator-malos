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

var creator_ip = '127.0.0.1';
var create_zigbee_base_port = 20013 + 20;
var protoBuf = require("protobufjs");
var zmq = require('zmq');

var zigbee_network_up = false;
var gateway_up = false;
var attemps = 10;
var device_detected = false;

// status
var none = 0;
var status = none;
var waiting_for_devices = 1;
var waiting_for_network_status = 2;
var nodes_discovered = 3;
var nodes_id = [];
var endpoints_index = [];

var protoBuilder =
    protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto');
var matrixMalosBuilder = protoBuilder.build("matrix_malos");

// ------------------------- ZigBee --------------------------
var config = new matrixMalosBuilder.DriverConfig;

//-----  Print the errors that the ZigBee driver sends ------------
var errorSocket = zmq.socket('sub');
errorSocket.connect('tcp://' + creator_ip + ':' +
                    (create_zigbee_base_port + 2));
errorSocket.subscribe('');
errorSocket.on('message', function(error_message) {
  process.stdout.write('Message received: ' + error_message.toString('utf8') +
                       "\n");
});

// ----- Create the socket for sending data to the ZigBee driver ----

var configSocket = zmq.socket('push');
configSocket.connect('tcp://' + creator_ip + ':' + create_zigbee_base_port);

// ------------ Starting to ping the driver -----------------------

var pingSocket = zmq.socket('push');
pingSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 1));
pingSocket.send('');  // Ping the first time.

setInterval(function() { pingSocket.send(''); }, 1000);

// -------------- Receive Data from Driver  ----------------------

var subSocket = zmq.socket('sub');
subSocket.connect('tcp://' + creator_ip + ':' + (create_zigbee_base_port + 3));

subSocket.subscribe('');
subSocket.on('message', function(buffer) {

  var zig_msg = new matrixMalosBuilder.ZigBeeMsg.decode(buffer);

  if (zig_msg.type == matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.ZCL) {
  } else if (matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT) {
    switch (zig_msg.network_mgmt_cmd.type) {
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .CREATE_NWK:
        // console.log('CREATE_NWK');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .LEAVE_NWK:
        // console.log('LEAVE_NWK');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .NODE_LEAVE_NWK:
        // console.log('NODE_LEAVE_NWK');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .PERMIT_JOIN:
        // console.log('PERMIT_JOIN');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .NODE_INFO:
        // console.log('NODE_INFO');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .DISCOVERY_INFO:

        if (status == waiting_for_devices) {
          var zig_msg = new matrixMalosBuilder.ZigBeeMsg.decode(buffer).toRaw();
          // Looking  for nodes that have an on-off cluster
          console.log('Device(s) found!!!');
          console.log('Looking for nodes that have an on-off cluster.');
          for (var i = 0; i < zig_msg.network_mgmt_cmd.connected_nodes.length;
               i++) {
            for (var j = 0;
                 j <
                 zig_msg.network_mgmt_cmd.connected_nodes[i].endpoints.length;
                 j++) {
              for (var k = 0; k < zig_msg.network_mgmt_cmd.connected_nodes[i]
                                      .endpoints[j]
                                      .clusters.length;
                   k++) {
                // Adding just nodes with  On/Off cluster
                if (zig_msg.network_mgmt_cmd.connected_nodes[i]
                        .endpoints[j]
                        .clusters[k]
                        .cluster_id == 6) {
                  // saving the node_id
                  nodes_id
                      .push(zig_msg.network_mgmt_cmd.connected_nodes[i].node_id)
                      // saving the endpoint_index
                      endpoints_index.push(
                          zig_msg.network_mgmt_cmd.connected_nodes[i]
                              .endpoints[j]
                              .endpoint_index);
                  continue;
                }
              }
            }
          }

          if (nodes_id.length > 0) {
            status = nodes_discovered;
            process.stdout.write(nodes_id.length +
                                 ' nodes found with on-off cluster\n');
          } else {
            status = none;
            console.log('No devices found !');
            process.exit(1);
          }
          // Start toggling the nodes
          ToggleNodes();
        }

        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .RESET_PROXY:
        // console.log('RESET_PROXY');
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .IS_PROXY_ACTIVE:

        if (zig_msg.network_mgmt_cmd.is_proxy_active) {
          console.log('Gateway connected');
          gateway_up = true;  // zig_msg.network_mgmt_cmd.is_proxy_activee;
        } else {
          console.log('Gateway Reset Failed.');
          process.exit(1);
        }
        console.log('Requesting ZigBee Network Status');
        config.zigbee_message.network_mgmt_cmd
            .set_type(matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd
                          .NetworkMgmtCmdTypes.NETWORK_STATUS)
                configSocket.send(config.encode().toBuffer());
        status = waiting_for_network_status;
        break;
      case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .NETWORK_STATUS:

        if (status != waiting_for_network_status) {
          break;
        }

        process.stdout.write('NETWORK_STATUS: ')

            status = none;

        switch (zig_msg.network_mgmt_cmd.network_status.type) {
          case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkStatus.Status
              .NO_NETWORK:
            console.log('NO_NETWORK');
            console.log('Creating a ZigBee Network');
            config.zigbee_message.network_mgmt_cmd
                .set_type(matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd
                              .NetworkMgmtCmdTypes.CREATE_NWK)
                    configSocket.send(config.encode().toBuffer());
            status = waiting_for_network_status;
            break;
          case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkStatus.Status
              .JOINING_NETWORK:
            console.log('JOINING_NETWORK');
            break;
          case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkStatus.Status
              .JOINED_NETWORK:
            console.log('JOINED_NETWORK');
            config.zigbee_message.network_mgmt_cmd.set_type(
                matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
                    .PERMIT_JOIN);

            // Send a permit join commnad
            var permit_join_params = new matrixMalosBuilder.ZigBeeMsg
                                         .NetworkMgmtCmd.PermitJoinParams;
            permit_join_params.setTime(60);
            config.zigbee_message.network_mgmt_cmd.set_permit_join_params(
                permit_join_params);

            configSocket.send(config.encode().toBuffer());

            console.log('Please reset your zigbee devices');
            console.log('... Waiting 60 sec for new devices');
            status = waiting_for_devices;
            break;
          case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkStatus.Status
              .JOINED_NETWORK_NO_PARENT:
            console.log('JOINED_NETWORK_NO_PARENT');
            break;
          case matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkStatus.Status
              .LEAVING_NETWORK:
            console.log('LEAVING_NETWORK');
            break;
        }

        break;
    }
  } else if (matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
                 .NETWORK_MGMT) {
  }

});

ResetGateway();

function ResetGateway() {
  // --- Setting the delay_between_updates and set_timeout_after_last_ping --
  console.log('Setting the Zigbee Driver');
  config.set_delay_between_updates(1);
  config.set_timeout_after_last_ping(1);
  configSocket.send(config.encode().toBuffer());

  // ---- Creating the base proto zigbee message --------------------
  config = new matrixMalosBuilder.DriverConfig;

  var zig_msg = new matrixMalosBuilder.ZigBeeMsg;
  var network_mgmt_cmd = new matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd;
  var zcl_cmd = new matrixMalosBuilder.ZigBeeMsg.ZCLCmd;
  var onoff_cmd = new matrixMalosBuilder.ZigBeeMsg.ZCLCmd.OnOffCmd;
  var colorcontrol_cmd =
      new matrixMalosBuilder.ZigBeeMsg.ZCLCmd.ColorControlCmd;
  var movetohueandsat_params = new matrixMalosBuilder.ZigBeeMsg.ZCLCmd
                                   .ColorControlCmd.MoveToHueAndSatCmdParams;


  zig_msg.set_zcl_cmd(zcl_cmd);
  zig_msg.zcl_cmd.set_onoff_cmd(onoff_cmd);
  zig_msg.zcl_cmd.set_colorcontrol_cmd(colorcontrol_cmd);
  zig_msg.zcl_cmd.colorcontrol_cmd.set_movetohueandsat_params(
      movetohueandsat_params);
  zig_msg.set_network_mgmt_cmd(network_mgmt_cmd);
  config.set_zigbee_message(zig_msg);

  // ------------ Reseting the Gateway App -----------------------
  console.log('Reseting the Gateway App');
  config.zigbee_message.set_type(
      matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.NETWORK_MGMT);
  config.zigbee_message.network_mgmt_cmd.set_type(
      matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
          .RESET_PROXY);
  configSocket.send(config.encode().toBuffer());

  // ------------ Checking connection with the Gateway ----------------------
  console.log('Checking connection with the Gateway');
  config.zigbee_message.network_mgmt_cmd
      .set_type(matrixMalosBuilder.ZigBeeMsg.NetworkMgmtCmd.NetworkMgmtCmdTypes
                    .IS_PROXY_ACTIVE)
          configSocket.send(config.encode().toBuffer());
}

function ToggleNodes() {
  if (!nodes_discovered) return;
  config.zigbee_message.set_type(
      matrixMalosBuilder.ZigBeeMsg.ZigBeeCmdType.ZCL);
  config.zigbee_message.zcl_cmd.set_type(
      matrixMalosBuilder.ZigBeeMsg.ZCLCmd.ZCLCmdType.ON_OFF);
  config.zigbee_message.zcl_cmd.onoff_cmd.set_type(
      matrixMalosBuilder.ZigBeeMsg.ZCLCmd.OnOffCmd.ZCLOnOffCmdType.TOGGLE);

  setInterval(function() {
    for (var i = 0; i < nodes_id.length; i++) {
      process.stdout.write('Sending toggle to Node: ')
          process.stdout.write(nodes_id[i] + "\n")
              config.zigbee_message.zcl_cmd.set_node_id(nodes_id[i]);
      config.zigbee_message.zcl_cmd.set_endpoint_index(endpoints_index[i]);
      configSocket.send(config.encode().toBuffer());
    }
  }, 2000);
}
