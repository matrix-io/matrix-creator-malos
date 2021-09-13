## Documentation
Visit the MATRIX CORE [Getting Started](https://matrix-io.github.io/matrix-documentation/matrix-core/getting-started/) page for a better understanding and setting up Javascript & Python.

## Installation
Before starting, ensure you have access to the terminal of your Raspberry Pi via an <a href="https://www.raspberrypi.org/documentation/remote-access/ssh/" target="_blank">SSH-session</a> or a connected screen, mouse, and keyboard. Then insert and run the following commands into your Raspberry Pi's terminal, one at a time.
```language-bash
curl -L https://apt.matrix.one/doc/apt-key.gpg | sudo apt-key add -
echo "deb https://apt.matrix.one/raspbian $(lsb_release -sc) main" | sudo tee /etc/apt/sources.list.d/matrixlabs.list
sudo apt-get update
sudo apt-get upgrade 
```

The next command will install the MATRIX CORE packages.
```language-bash
sudo apt-get install matrixio-malos
```

After the MATRIX CORE packages are installed, use the command below to reboot your Raspberry Pi. MATRIX CORE will then be running as a service each time your Raspberry Pi boots up.
```language-bash
sudo reboot
```

These remaining commands will install <a href="http://zeromq.org/" target="_blank">ZeroMQ</a>.
```language-bash
echo "deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/Debian_9.0/ ./" | sudo tee /etc/apt/sources.list.d/zeromq.list
wget https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/Debian_9.0/Release.key -O- | sudo apt-key add
```
