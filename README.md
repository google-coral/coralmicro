# Coral Dev Board Micro source code repository

- Dev Board Micro is a device based on RT1176.
- This repository contains a CMake-based build system for the device.

## Initialize submodules
Before you try and build anything, be sure to initialize the submodules!

```
git submodule update --init --recursive
```

## Building the code
```
bash build.sh
```

By default, this will build in a folder called `build`. If you wish to build into a different directory, please pass it with the `-b` flag.

## Loading code to the device (uses HelloWorldFreeRTOS as example)
### Install prerequisites
Before running the script for the first time, be sure to install the required Python modules:
```
python3 -m pip install -r scripts/requirements.txt
sudo apt-get update
sudo apt-get install -y \
         cmake \
         git-core \
         libusb-1.0-0-dev \
         libudev-dev \
         libhidapi-hidraw0 \
         ninja-build \
         protobuf-compiler \
         python3-dev \
         python3-pip \
         python3-protobuf \
         vim
```

Also, be sure to setup udev rules, or the scripts will have permission and path issues:
```
sudo cp scripts/99-coral-micro.rules scripts/50-cmsis-dap.rules scripts/99-secure-provisioning.rules /etc/udev/rules.d
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Loading code to RAM
This is not persistent, and will be lost on a power cycle.
```
python3 scripts/flashtool.py --build_dir build --app HelloWorldFreeRTOS --ram
```

### Loading code AND filesystem to NAND
This is persistent. To boot from NAND, the switch on the underside of the board must be set in the position that is nearer to the center of the board.
This will load both the code and the filesystem to the NAND. As it loads the filesystem, you should always do this step once on a fresh board (or if you change the filesystem contents).
```
python3 scripts/flashtool.py --build_dir build --app HelloWorldFreeRTOS
```

## Recovering EVT+ from a bad state
To reset the EVT+ boards to a known good state, hold the button in the center of the board (near TPU), press and release the reset button (side of the device), and release the center button. It will boot in serial download mode, and the above flash instructions can be used to put working code on the device.
