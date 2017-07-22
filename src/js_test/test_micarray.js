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


var creator_ip = process.env.CREATOR_IP || '127.0.0.1'
var creator_micarray_base_port = 20037 

var zmq = require('zmq')

// Import MATRIX Proto messages
var matrix_io = require('matrix-protos').matrix_io

//
// Micarray setup with MALOS
//

var configSocket = zmq.socket('push')
configSocket.connect('tcp://' + creator_ip + ':' + creator_micarray_base_port /* config */)

function setupMicarray() {
  var micarray_cfg = matrix_io.malos.v1.io.MicArrayParams.create({
    gain: 16,          // set gain for all microphones 
    azimutalAngle: 0,  // set a sound source perpendicular to the MATRIX Creator
    polarAngle: 0,
    radialDistanceMm: 1000,
    soundSpeedMmseg: 340.3 * 1000
  })
 
  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    micarray: micarray_cfg
  })
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())
}

setupMicarray()


//
// ALSA recorder
//
// mic_channel8 is a beamformed channel using the delay and sum method
var micInstance = mic({ 
  device: 'mic_channel8',
  rate: '16000', 
  channels: '1', 
  debug: true, 
  exitOnSilence: 6 
})
var micInputStream = micInstance.getAudioStream();
var outputFileStream = fs.WriteStream('output.wav');

micInputStream.pipe(outputFileStream);

micInputStream.on('data', (data) => {
  console.log("Recieved Input Stream: " + data.length);
});


micInputStream.on('error', (err) => {
  cosole.log("Error in Input Stream: " + err);
});


micInputStream.on('startComplete', () => {
  console.log("Got SIGNAL startComplete");
  setTimeout(() => {
    micInstance.pause();
  }, 5000);
});


micInputStream.on('stopComplete', () => {
  console.log("Got SIGNAL stopComplete");
});


micInputStream.on('pauseComplete', () => {
  console.log("Got SIGNAL pauseComplete");
  setTimeout(() => {
    micInstance.resume();
  }, 5000);
});


micInputStream.on('resumeComplete', () => {
  console.log("Got SIGNAL resumeComplete");
  setTimeout(() => {
    micInstance.stop();
  }, 5000);
});


micInputStream.on('silence', () => {
  console.log("Got SIGNAL silence");
});


micInputStream.on('processExitComplete', () => {
  console.log("Got SIGNAL processExitComplete");
});


micInstance.start();
