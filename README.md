# MALOS

Hardware abstraction layer for MATRIX Creator usable via 0MQ. Protocol buffers are used for data exchange.

### Install MALOS
```
sudo apt-get install matrix-creator-init matrix-creator-malos cmake g++ git
sudo shutdown -r now
```

### Upgrade MALOS
```
sudo apt-get update && sudo apt-get upgrade
sudo shutdown -r now
```

### Test MALOS
Start it as a background process.
```
malos > /dev/null 2>&1 &
```

### Clone and test examples
Note: pre-requisite is NodeJS. Don't use the one shipped with raspbian because it's a bit old. If you don't have it, please check a recipe included below.
```
git clone https://github.com/matrix-io/matrix-creator-malos.git
cd src/js_test

// humidity, temperatre
node test_humidity.js 

// inertial measurement unit
node test_imu.js 

// pressure, altitude
node test_pressure.js 

// uv index, uv range
node test_uv.js
```

### Kill MALOS
```
// Kill malos process
pkill -9 malos
```

-------------------------

### NodeJS Dependency (Testing Only)

You can use NodeJS from another computer (not the Raspberry) or from the raspberry itself.
Check for the most recent version of Node.js, but today it is: https://nodejs.org/dist/v4.5.0/node-v4.5.0-linux-armv7l.tar.xz

You can download it, uncompress it and then change the default `PATH`.

For instance (in the Raspberry):

    mkdir nodejs && cd nodejs
    wget -c https://nodejs.org/dist/v4.5.0/node-v4.5.0-linux-armv7l.tar.xz
    xz -d node-v4.5.0-linux-armv7l.tar.xz
    tar xvf node-v4.5.0-linux-armv7l.tar
    export PATH=/home/pi/nodejs/node-v4.5.0-linux-armv7l/bin:$PATH

In this version the binary is called `node` and not `nodejs`. The IPs used in the examples are 127.0.0.1. Remember to edit them if you're accessing the Creator from another host and not from the Raspberry itself.
