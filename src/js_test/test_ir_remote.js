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
var creator_lirc_base_port = 20013 + 28 // port for Lirc driver.

var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_lirc_base_port /* config */)

var http = require('http');
var fs = require('fs');

/**
 * LIRC remote database on admobilize assets.
 * Orginal source on: https://sourceforge.net/p/lirc-remotes/code/ci/master/tree/
 * Example remote device (SONY control RM-AAU014) for this test:
 */
var remote_url="http://assets.admobilize.com/lirc-remotes/sony/RM-AAU014.lircd.conf"
var remote_output="RM-AAU014.lircd.conf"

/**
 * configRemote: Main function, set configuration from 
 * remote.conf file downloaded
 *
 * Params:
 * config: file downloaded path
 */

function configRemote(config){
  process.stdout.write('set remote to config..')
  fs.readFile(config, 'utf8', function (err,data) {
    if (err) { return console.log(err); }
    var ir_cfg_cmd = new matrixMalosBuilder.LircParams
    ir_cfg_cmd.set_config(data)
    sendIRConfigProto(ir_cfg_cmd) 
    console.log('done.')
    // sending commands
    continousSendRemoteCommand()
  });
}

/**
 * sendIrCommand: send once command.
 * Please see *.conf content for details
 *
 * Params:
 * device: LIRC remote device name
 * command: LIRC remote command
 */

function sendIrCommand(device, command) {
  var ir_cfg_cmd = new matrixMalosBuilder.LircParams
  ir_cfg_cmd.set_device(device)
  ir_cfg_cmd.set_command(command)
  sendIRConfigProto(ir_cfg_cmd) 
}

/**
 * sendIRConfigProto: build Proto message 
 * and send it.
 *
 * Params:
 * ir_cfg: LircParams proto message
 */

function sendIRConfigProto(ir_cfg){
  var config = new matrixMalosBuilder.DriverConfig
  config.set_lirc(ir_cfg)
  configSocket.send(config.encode().toBuffer());
}

/**
 * continousSendRemoteCommand: loop for test
 * of sending IR commands
 */

function continousSendRemoteCommand(){
  setInterval(function() {
    process.stdout.write('sending IR command: BTN_MUTING to SONY_RM device..')
    sendIrCommand('SONY_RM-AAU014','BTN_MUTING') // check LED on MATRIX
    console.log('done.')
  }, 3000);
}

/**
 * download: Basic http get downloader
 *
 * Params:
 * url: url target
 * dest: file path destination
 * cb: callback on finish
 */

function download (url, dest, cb) {
  process.stdout.write('downloading remote config..')
  var file = fs.createWriteStream(dest);
  var request = http.get(url, function(response) {
    response.pipe(file);
    file.on('finish', function() {
      console.log('done.')
      file.close(cb(dest));
    });
  });
}

/**************
 **** MAIN ****/

// download remote configuration and send continous MUTE commands
download(remote_url,remote_output,configRemote)


