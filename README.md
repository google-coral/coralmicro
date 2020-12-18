# Valiant source code repository

- Valiant is a device based on RT1176.
- This repository contains a CMake-based build system for the device.
- At the moment, all samples target the RT1170-EVK.

## CMake setup
It's recommended to have CMake put all artifacts into a build directory to not dirty the tree.
```
mkdir -p build
cd build
cmake ..
make
```

## Deploying an app to the RT1170-EVK
- The simplest way to deploy to the EVK is with the mass storage device.
- Ensure that the micro-USB port for debug (this is on the same side as the power connector) is attached to your workstation.
- A mass storage device should enumerate, note the location it is mounted to.
- We must convert the application to the ihex format, as well.
```
arm-none-eabi-objcopy -O ihex /path/to/build/apps/HelloWorld/HelloWorld /path/to/build/image.hex
cp /path/to/build/image.hex /path/to/mass-storage
sync
umount /path/to/mass-storage
```

- Hopefully the LED near the debug port does a good bit of blinking.
- Afterwards, the mass-storage device should re-appear, and no longer contain image.hex.
- If FAIL.TXT is present, something went wrong. It occasionally flakes, so try again at least once.
- To see output, open the ttyACM that the debug port exposes. You likely will have to press the reset button on the EVK to start the program.
