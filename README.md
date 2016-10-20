
# Pre-Requisites
```
echo "deb http://packages.matrix.one/matrix-creator/ ./" | sudo tee --append /etc/apt/sources.list;
sudo apt-get update;
sudo apt-get upgrade;
sudo apt-get install libzmq3-dev xc3sprog matrix-creator-openocd wiringpi cmake g++ git;
```

# MALOS

Hardware abstraction layer for MATRIX Creator usable via 0MQ. Protocol buffers are used for data exchange.

### Install MALOS
```
sudo apt-get install matrix-creator-init matrix-creator-malos
sudo shutdown -r now
```

### Upgrade MALOS
```
sudo apt-get update && sudo apt-get upgrade
sudo shutdown -r now
```

### Test MALOS

MALOS starts as a service, there is no need to start it by hand.

### Clone and test examples
Note: pre-requisite is NodeJS. Don't use the one shipped with raspbian because it's a bit old. If you don't have it, please check a recipe included below.
```
git clone https://github.com/matrix-io/matrix-creator-malos.git && cd matrix-creator-malos
git submodule init && git submodule update
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
-------------------------

### NodeJS Dependency

For instance (in the Raspberry):

```
# Install npm (doesn't really matter what version, apt-get node is v0.10...)
sudo apt-get install npm

# n is a node version manager
sudo npm install -g n

# node 6.5 is the latest target node version, also installs new npm
n 6.5

# check version
node -v
```
