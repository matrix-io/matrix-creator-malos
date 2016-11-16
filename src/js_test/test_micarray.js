// Example based in this: https://www.npmjs.com/package/mic
//
// Here we open mic_channel8. You can open more mics, from
// mic_channel0 to mic_channel8. From mic_channel0 to mic_channel7
// are the real microphones, mic_channel8  are the beamformed
// microphone. 
//
// *You can open more than one microphone.
//

var mic = require('mic');
var fs = require('fs');


// This is how we connect to the creator. IP and port.
// The IP is the IP I'm using and you need to edit it.
// By default, MALOS has its 0MQ ports open to the world.
//
// Every device is identified by a base port. Then the mapping works
// as follows:
// BasePort     => Configuration port. Used to config the device.
// BasePort + 1 => Keepalive port. Send pings to this port.
// BasePort + 2 => Error port. Receive errros from device.
// BasePort + 3 => Data port. Receive data from device.


var creator_ip = '127.0.0.1'
var creator_micarray_base_port = 20037 

//
// Micarray setup with MALOS
//
var protoBuf = require("protobufjs");
var protoBuilder = protoBuf.loadProtoFile('../../protocol-buffers/malos/driver.proto')
var matrixMalosBuilder = protoBuilder.build("matrix_malos")

var zmq = require('zmq')
var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_micarray_base_port /* config */)

function setupMicarray() {
  var micarray_cfg = new matrixMalosBuilder.MicArrayParams
 
  // setup gain for all microphones 
  micarray_cfg.set_gain(8)

  // setup a sound source perpendicular to the MATRIX Creator
  micarray_cfg.set_azimutal_angle(0)
  micarray_cfg.set_polar_angle(0)
  micarray_cfg.set_radial_distance_mm(1000)
  micarray_cfg.set_sound_speed_mmseg(340.3 * 1000)

  var config = new matrixMalosBuilder.DriverConfig
  config.set_micarray(micarray_cfg)
  configSocket.send(config.encode().toBuffer());
}

setupMicarray()


//
// ALSA recorder
//
// mic_channel8 is a beamformed channel using the delay and sum method
var micInstance = mic({ 'device':'mic_channel8','rate': '16000', 'channels': '1', 'debug': true, 'exitOnSilence': 6 });
var micInputStream = micInstance.getAudioStream();
var outputFileStream = fs.WriteStream('output.wav');

micInputStream.pipe(outputFileStream);

micInputStream.on('data', function(data) {
    console.log("Recieved Input Stream: " + data.length);
});


micInputStream.on('error', function(err) {
    cosole.log("Error in Input Stream: " + err);
});


micInputStream.on('startComplete', function() {
        console.log("Got SIGNAL startComplete");
        setTimeout(function() {
                micInstance.pause();
            }, 5000);
    });


micInputStream.on('stopComplete', function() {
        console.log("Got SIGNAL stopComplete");
    });


micInputStream.on('pauseComplete', function() {
        console.log("Got SIGNAL pauseComplete");
        setTimeout(function() {
                micInstance.resume();
            }, 5000);
    });


micInputStream.on('resumeComplete', function() {
        console.log("Got SIGNAL resumeComplete");
        setTimeout(function() {
                micInstance.stop();
            }, 5000);
    });


micInputStream.on('silence', function() {
        console.log("Got SIGNAL silence");
    });


micInputStream.on('processExitComplete', function() {
        console.log("Got SIGNAL processExitComplete");
    });


micInstance.start();
