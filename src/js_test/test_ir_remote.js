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
var creator_lirc_base_port = 20013 + 28 // port for Lirc driver.

var zmq = require('zmq')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io


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
 * Sets Lirc remote control configuration from a lirc config file
 *
 * @param {string} [configPath] config file path
 * @param {function} [callback] callback function
 */
function configRemote(configPath, callback) {
  console.log('Setting remote to config to %s', configPath)
  fs.readFile(configPath, 'utf8', (err, data) => {
    if (err) { return console.log(err); }
    var ir_cfg_cmd = matrix_io.malos.v1.comm.LircParams.create({
      config: data
    })
    sendIRConfigProto(ir_cfg_cmd) 
    callback()
  });
}

/**
 * Sends an IR command
 * Please see *.conf content for details
 *
 * @param {string} [device] LIRC remote device name i.e Sony_RM-AAU014
 * @param {string} [command] LIRC remote command i.e BTN_MUTING
 */
function sendIrCommand(device, command) {
  var ir_cfg_cmd = matrix_io.malos.v1.comm.LircParams.create({
    device: device,
    command: command
  })
  console.log('sending IR command: %s -> %s', device, command)
  sendIRConfigProto(ir_cfg_cmd) 
}

/**
 * Build DriverConfig proto message and send it
 *
 * @param {ProtoMessage} [ir_cfg] LircParams proto message
 */
function sendIRConfigProto(ir_cfg){
  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    lirc: ir_cfg
  })
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())
}

/**
 * Loops command a command to infinity for testing
 */
function loopRemoteCommand(){
  setInterval(() => {
    sendIrCommand('SONY_RM-AAU014','BTN_MUTING') // check LED on MATRIX
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

function download(url, dest, cb) {

  if (fs.existsSync(dest)) {
    console.log('Config found locally ...')
    process.nextTick(() => {
      cb(dest)
    })
    return
  }

  console.log('Downloading remote config ...')
  var file = fs.createWriteStream(dest);
  var request = http.get(url, (response) => {
    response.pipe(file);
    file.on('finish', () => {
      file.close(cb(dest));
    });
  });
}

/**************
 **** MAIN ****/

// download remote configuration and send continous MUTE commands
download(remote_url, remote_output, (configPath) => {
  configRemote(configPath, loopRemoteCommand)
})

