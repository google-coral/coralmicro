# Valiant source code repository

- Valiant is a device based on RT1176.
- This repository contains a CMake-based build system for the device.

## Initialize submodules
Before you try and build anything, be sure to initialize the submodules!

```
git submodule update --init
```

## CMake setup
It's recommended to have CMake put all artifacts into a build directory to not dirty the tree. By default, this will build for P0 in Release mode.
```
mkdir -p build
pushd build; cmake ..; popd
make -C build
```

## Loading code to the P0 device (uses HelloWorldFreeRTOS as example)
flashtool_p0 only operates (correctly) on srec files. These are generated for you automatically by the build system.

### Install prerequisites
Before running the script for the first time, be sure to install the required Python modules:
```
python3 -m pip install -r scripts/requirements.txt
sudo apt-get install libhidapi-hidraw0 python-dev libusb-1.0-0-dev libudev-dev
```

Also, be sure to setup udev rules, or the scripts will have permission and path issues:
```
sudo cp scripts/99-valiant.rules scripts/50-cmsis-dap.rules scripts/99-secure-provisioning.rules /etc/udev/rules.d
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Loading code to RAM
This is not persistent, and will be lost on a power cycle.
```
python3 scripts/flashtool_p0.py --srec build/apps/HelloWorldFreeRTOS/image.srec --ram
```

### Loading code AND filesystem to NAND
This is persistent. To boot from NAND, the switch on the underside of the board must be set in the position that is nearer to the center of the board.
This will load both the code and the filesystem to the NAND. As it loads the filesystem, you should always do this step once on a fresh board (or if you change the filesystem contents).
```
python3 scripts/flashtool_p0.py --srec build/apps/HelloWorldFreeRTOS/image.srec
```

### Loading code to NAND (no filesystem)
Same as above, but skips the filesystem. Much faster if you didn't make any changes to the filesystem.
```
python3 scripts/flashtool_p0.py --srec build/apps/HelloWorldFreeRTOS/image.srec --nofs
```

## Recovering P0 from a bad state
If you run code on P0 that puts it in a bad state (e.g. crashes at startup), it can be recovered by putting the device into serial download mode and loading new code. To do so, set the switch on the underside of the board to the position that is nearer the edge of the board, and press the reset button.

It will boot in serial download mode, and the above flash instructions can be used to put working code on the device.
