# I2C example

This pair of applications demonstate the Dev Board Micro's I2C hardware.

NOTE: Executing this example successfully requires two devices, and the devices
need headers soldered to facilitate connecting the board I2C lines to each other.

## i2c_target

i2c_target listens on the I2C bus for a transaction request, on address 0x42.
It will reply with up to sixteen bytes of data, containing the device's serial number.
The device is listening on I2C6.

## i2c_controller

i2c_controller executes a transaction on the I2C bus, where it attempts to read
sixteen bytes from a device at address 0x42.
The device is speaking on I2C1.

## Device setup and flashing

Using a wire, connect I2C1_SDA (SDA1) of one device to I2C6_SDA (SDA6) on the other.
Using a wire, connect I2C1_SCL (SCL1) of one device to I2C6_SCL (SCL6) on the other.

On the device that you have used the I2C6 pins, flash i2c_target:

```
python3 scripts/flashtool.py -e i2c --subapp i2c_target
```

On the device that you have used the I2C1 pins, flash i2c_controller:

```
python3 scripts/flashtool.py -e i2c --subapp i2c_controller
```

## Running the example

Power the device with i2c_target first -- it needs to be listening or the i2c_controller will fail.
Power up the device with i2c_controller, and observe the console. You should be presented with output like:
```
Writing our serial number to the remote device...
Reading back our serial number from the remote device...
Readback of data from target device matches written data!
```
