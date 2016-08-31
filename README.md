# matrix-creator-malos

Hardware abstraction layer for MATRIX Creator usable via 0MQ.

See src/js_test/ for a few Node.js examples.

Protocol buffers are used for data exchange.

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
Note: pre-requisite is NodeJS.
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
