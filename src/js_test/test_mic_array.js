//  Example taken from: https://www.npmjs.com/package/mic
//  Here we open mic_channel0. You can open more mics, from
//  mic_channel0 to mic_channel7. You can open more than one
//  microphone.


var mic = require('mic');

var fs = require('fs');



var micInstance = mic({ 'device':'mic_channel0','rate': '16000', 'channels': '1', 'debug': true, 'exitOnSilence': 6 });


var micInputStream = micInstance.getAudioStream();


var outputFileStream = fs.WriteStream('output.raw');


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
