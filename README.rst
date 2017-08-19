MALOS
=====

.. _MATRIX Creator: https://creator.matrix.one
.. _matrix-protos: https://www.npmjs.com/package/matrix-protos
.. _matrix_io-proto: https://pypi.python.org/pypi/matrix-io-proto/0.0.10
.. _MATRIX protos: https://github.com/matrix-io/protocol-buffers
.. _zmq: https://www.npmjs.com/package/zmq
.. _Protocol buffers: https://developers.google.com/protocol-buffers/docs/proto3

Hardware abstraction layer for MATRIX Creator usable via 0MQ. `Protocol buffers`_
are used for data exchange.

You can also use MALOS to query sensors of the `MATRIX Creator`_ and to control the MATRIX
Creator from any language that supports protocol buffers (version 3.X)
and 0MQ, Connections to MALOS can be made both from localhost
(127.0.0.1) and from remote computers that are in the same network.

Pre-Requisites
--------------

::

    echo "deb http://packages.matrix.one/matrix-creator/ ./" | sudo tee --append /etc/apt/sources.list;
    sudo apt-get update;
    sudo apt-get upgrade;
    sudo apt-get install libzmq3-dev xc3sprog matrix-creator-openocd wiringpi cmake g++ git;

Install
-------

::

    sudo apt-get install matrix-creator-init matrix-creator-malos
    sudo reboot

**Note:** At this point, on next start, ``malos`` will be running as a
service.

Upgrade
-------

::

    sudo apt-get update && sudo apt-get upgrade
    sudo reboot

Starting manually
-----------------

::

    # MALOS runs as a service, but to stop it run:
    sudo pkill -9 malos

    # to run manually, just type `malos`
    malos

Protocol
--------

All the MALOS drivers use the same protocol to interact with other
programs. Drivers allow read-only, write-only or read-write modes.

MALOS uses 0MQ to transfer information from and to the client
interfacing with it. Each MALOS driver has a base 0MQ port. This is the
current list of base ports (This is printed by MALOS when it is started
from the console):

::

    Registered driver IMU with port 20013.
    Registered driver Humidity with port 20017.
    Registered driver Everloop with port 20021.
    Registered driver Pressure with port 20025.
    Registered driver UV with port 20029.
    Registered driver ZigbeeBulb with port 20033.
    Registered driver MicArray_Alsa with port 20037.
    Registered driver Lirc with port 20041.

Each port reserves a range of 4 ports that are used for a driver. They
are described in the following sections.

Base port
~~~~~~~~~

This is the first port of the driver and the one used to denote the
``driver port``. It is used to configure the device and it makes sense
for the devices that support configuration. It is a 0MQ PULL port. To
send a configuration you need to send a valid message (serialized to a
string) for the given driver. For instance, the Everloop driver (LED
array) uses a configuration message to set the LEDs.

The message is named EverloopImage and it is in the file
`driver.proto <https://github.com/matrix-io/protocol-buffers/blob/master/malos/driver.proto>`__.
The message follows:

::

    message LedValue {
      uint32 red = 1;
      uint32 green = 2;
      uint32 blue = 3;
      uint32 white = 4;
    }

    // The led array.
    message EverloopImage {
      repeated LedValue led = 1;
    }

In order to turn all the LEDs RED you need to add 35 messages of type
LedValue to the EverloopImage message. Each of the LedValue messages
would need to have the following values:

-  red: 10
-  green: 0
-  blue: 0
-  white: 0

The valid values for led intensities range from 0 to 255, but we set red
to 10 because this value is bright enough.

Once the message of type EverloopImage is filled out it needs to be
serialized as a string and sent to the 0MQ configuration port.

If invalid values are used for the LED values of the number of LED
values inside of EverloopImage is not 35 the configuration will be
discarded and an error message will be generated and sent to the error
channel described below.

Error port
~~~~~~~~~~

Programs can subscribe to the 0MQ error port. It is a PUSH port. The
port number is obtained by adding 2 to the base port (Also known as
driver port). The errors are returned as strings but there's a pending
task to change the error messages to a protocol buffer (`track
issue <https://github.com/matrix-io/matrix-creator-malos/issues/21>`__).
Please do not depend on errors reported as strings as we will change the
errors to protocol buffers soon.

Keep-alive port
~~~~~~~~~~~~~~~

The port number is obtained by adding 1 to the base port (Also known as
driver port). It is a PUSH port.

In order to save CPU power and other resources some drivers require
applications to send pings to it in order to keep them alive. For most
driveres it means that the driver keeps sending updates as fast as they
have been configured to do so. The Everloop driver doesn't require
keep-alive messages. The IMU driver does.

Drivers that need keep-alive messages can be configured using the
message that is used for all the configurations. The way to do it is set
relevant field while doing other driver specific configuration (if this
is required).

::

    message DriverConfig {
      // Delay between updates. In seconds.
      float delay_between_updates = 1;
      // Timeout after last ping.
      float timeout_after_last_ping = 2;

      // More fields here -- Omitted.
    }

The field timeout\_after\_last\_ping defaults to 5 seconds and it can be
set during driver configuration. If a driver doesn't receive Clive
messages after ``timeout_after_last_ping`` seconds it will stop sending
updates. This field is ignored by drivers that do not require keep alive
messages. For instance, the Everloop driver ignores this setting.

After the setting is done (or not if the default value of 5 seconds is
OK) you can start sending keepalive messages to the driver by sending
messages to the respective 0MQ port. Any message that is sent to this
port will be discarded, so the empty string "" makes for a good
keep-alive message.

Data update port
~~~~~~~~~~~~~~~~

The port number is obtained by adding 3 to the base port (Also known as
driver port). This port is used by drivers that send data (for instance
Humidity and UV). Each driver uses a different message to report data to
programs that subscribe expecting updates.

Let's use the UV driver as an example. The relevant message:

::

    // Basic UV radiation lecture.
    message UV{
      // UV index.
      float uv_index = 1;

      // Risk of harm from unprotected sun exposure, for the average adult.
      // According to the OMS table. https://www.epa.gov/sunsafety/uv-index-scale-0
      string oms_risk = 2;
    }

Drivers that subscribe to updates via 0MQ will receive a string with
serialized messages of type UV (within the matrix\_malos namespace).
Then this message needs to be deserialized and the values can be used.

Workflow
~~~~~~~~

Wrapping up the protocol section, a program that talks to MALOS can:

-  Configure a driver if needed. Or configure it many times if the
   drivers needs it.
-  Subscribe to error messages if it is interested in them.
-  Sends keep-alive messages if those are needed by the driver to remain
   active.
-  Subscribe to updates from the driver if the drivers produces them.

Interfaces
----------

-  `Everloop <docs/everloop.md>`__
-  `Humidity <docs/humidity.md>`__
-  `IMU <docs/imu.md>`__
-  `IR <docs/lirc.md>`__
-  `Microphone Array <docs/microphone.md>`__
-  `Pressure <docs/pressure.md>`__
-  `UV <docs/uv.md>`__
-  `Zigbee <docs/zigbee.md>`__
-  `NFC <docs/nfc.md>`__
-  `Z-wave <docs/zwave.md>`__
-  `GPIO <docs/gpio.md>`__

Examples
--------

**Note:** pre-requisite is NodeJS. Don't use the one shipped with
Raspbian because it's a bit old. If you don't have it, please check a
recipe included below.

::

    git clone https://github.com/matrix-io/matrix-creator-malos.git && cd matrix-creator-malos
    git submodule init && git submodule update
    cd src/js_test

    // humidity, temperature
    node test_humidity.js

    // inertial measurement unit
    node test_imu.js

    // pressure, altitude
    node test_pressure.js

    // uv index, uv range
    node test_uv.js

--------------

NodeJS Dependency
-----------------

For instance (in the Raspberry):

::

    # Install npm (doesn't really matter what version, apt-get node is v0.10...)
    sudo apt-get install npm

    # n is a node version manager
    sudo npm install -g n

    # node 6.5 is the latest target node version, also installs new npm
    n 6.5

    # check version
    node -v

Using MALOS
===========

MALOS Examples
--------------

Connecting to MALOS with NodeJS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Note:** You'll need `matrix-protos`_ and `zmq`_ npm packages, and
protobuf as a submodule. See the `Everloop Example <src/js_test/test_everloop.js>`__
for the full implementation of the code below.

.. code-block:: javascript

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
  var creator_everloop_base_port = 20013 + 8 // port for Everloop driver.

  var zmq = require('zmq')

  // Import MATRIX Proto messages
  var matrix_io = require('matrix-protos').matrix_io


  // To trigger an error message you can send an invalid configuration to the driver.
  // For instance, set a number of leds != 35.
  var errorSocket = zmq.socket('sub')
  errorSocket.connect('tcp://' + creator_ip + ':' + (creator_everloop_base_port + 2))
  errorSocket.subscribe('')
  errorSocket.on('message', (error_message) => {
    console.log('Message received: Pressure error: ' + error_message.toString('utf8'))
  });

  var configSocket = zmq.socket('push')
  configSocket.connect('tcp://' + creator_ip + ':' + creator_everloop_base_port /* config */)


Passing Commands to MALOS
~~~~~~~~~~~~~~~~~~~~~~~~~

Below is an example of some NodeJS interfacing with the Everloop via
MALOS. See the `Everloop Example <src/js_test/test_everloop.js>`__
for the full implementation of the code below.

.. code-block:: javascript

  ...

  var max_intensity = 50
  var intensity_value = max_intensity

  function setEverloop(led_values) {
      var image = matrix_io.malos.v1.io.EverloopImage.create()
      for (var j = 0; j < 35; ++j) {
        var led_conf = matrix_io.malos.v1.io.LedValue.create(led_values);
        image.led.push(led_conf)
      }
      var config = matrix_io.malos.v1.driver.DriverConfig.create({
        image: image
      })
      configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish());
  }

  setInterval(() => {
    intensity_value -= 1
    if (intensity_value < 0)
      intensity_value = max_intensity
    setEverloop({
      red: 0,
      green: intensity_value,
      blue: 0,
      white: 0
    })
  }, 50);


Reading from MALOS
~~~~~~~~~~~~~~~~~~

Below is a simple implementation via NodeJS to read a ``humidity`` from
MALOS via 0MQ. See `Humidty Example <src/js_test/test_humidity.js>`__
for the full example.

.. code-block:: javascript

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
  var creator_humidity_base_port = 20013 + 4 // port for Humidity driver.

  var zmq = require('zmq')

  // Import MATRIX Proto messages
  var matrix_io = require('matrix-protos').matrix_io


  // ********** Start error management.
  var errorSocket = zmq.socket('sub')
  errorSocket.connect('tcp://' + creator_ip + ':' + (creator_humidity_base_port + 2))
  errorSocket.subscribe('')
  errorSocket.on('message', (error_message) => {
    console.log('Message received: Humidity error: ' + error_message.toString('utf8'))
  });
  // ********** End error management.


  // ********** Start configuration.
  var configSocket = zmq.socket('push')
  configSocket.connect('tcp://' + creator_ip + ':' + creator_humidity_base_port)

  var config = matrix_io.malos.v1.driver.DriverConfig.create({
    delayBetweenUpdates: 2.0,  // 2 seconds between updates
    timeoutAfterLastPing: 6.0, // Stop sending updates 6 seconds after pings.
    humidity: matrix_io.malos.v1.sense.HumidityParams.create({
      currentTemperature: 23   // Real current temperature [Celsius] for calibration 
    })
  })

  // Send driver configuration.
  configSocket.send(matrix_io.malos.v1.driver.DriverConfig.encode(config).finish())

  // ********** End configuration.

  // ********** Start updates - Here is where they are received.
  var updateSocket = zmq.socket('sub')
  updateSocket.connect('tcp://' + creator_ip + ':' + (creator_humidity_base_port + 3))
  updateSocket.subscribe('')
  updateSocket.on('message', (buffer) => {
    var data = matrix_io.malos.v1.sense.Humidity.decode(buffer)
    console.log(data)
  });
  // ********** End updates

  // ********** Ping the driver
  var pingSocket = zmq.socket('push')
  pingSocket.connect('tcp://' + creator_ip + ':' + (creator_humidity_base_port + 1))
  process.stdout.write("Sending pings every 5 seconds");
  pingSocket.send(''); // Ping the first time.
  setInterval(() => {
    pingSocket.send('');
  }, 5000);
  // ********** Ping the driver ends

Available Protobuf Packages
~~~~~~~~~~~~~~~~~~~~~~~~~~~

NodeJs
^^^^^^

The easiest way of using protocol buffers in NodeJS is by installing `matrix-protos`_ npm 
module: 

::

  $ npm install matrix-protos


Python
^^^^^^

The python equivalente is also available via the `matrix_io-proto`_ `pip`` package:

::

  $ pip install matrix_io-proto


Protobuf message documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can check the protocol message definition in the `MATRIX protos`_ repo. 
