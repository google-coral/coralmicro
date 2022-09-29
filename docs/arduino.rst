Arduino APIs
=====================


The Coral Dev Board Micro is compatible with the `core Arduino
programming langauge <https://www.arduino.cc/reference/en/>`_. So a lot of
simple Arduino projects can work on the Dev Board Micro without any code
changes.

This page describes additional APIs that provide Arduino-style programming for
Dev Board Micro hardware, such as the camera and
microphone, plus descriptions for features that use standard Arduino
libraries, such as GPIOs and the filesystem.

Even the APIs that have custom implementations for the Dev Board Micro
are designed to allow code-reuse with other Arduino
projects that use similar libraries whenever possible.

To try programming with Arduino for the Dev Board Micro, start by following the
guide to `Build apps with Arduino </docs/dev-board-micro/arduino/>`_.


Camera
----------

The Dev Board Micro includes an on-board camera module with 324 x 324 px
resolution. To use the camera with Arduino, you need to use the global instance
of the :cpp:any:`~coralmicro::arduino::CameraClass` object called ``Camera``.

To get started, you just need to call
:cpp:any:`~coralmicro::arduino::CameraClass::begin` and specify the image format
you want, such as the image resolution and rotation.

When you want to capture an image, call
:cpp:any:`~coralmicro::arduino::CameraClass::grab`
and pass it a buffer where the image should be stored.

When you're done, call
:cpp:any:`~coralmicro::arduino::CameraClass::end()` to turn the camera off.

**Example**:

.. literalinclude:: ../arduino/libraries/Sensors/examples/Camera/Camera.ino
   :start-after: [start-snippet:ardu-camera]
   :end-before: [end-snippet:ardu-camera]

`[coralmicro_camera.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/CoralMicro_Camera/src/coralmicro_camera.h>`_

.. doxygenfile:: CoralMicro_Camera/src/coralmicro_camera.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var


Microphone
-----------------

The Dev Board Micro includes one on-board pulse-density modulation (PDM)
microphone. To use the microphone with Arduino, you need to use the global
instance of the global instance of the :cpp:any:`~coralmicro::arduino::PDMClass`
object called ``Mic``.

This API is designed to be code-compatible with projects that use the
`Arduino PDM library <https://docs.arduino.cc/learn/built-in-libraries/pdm>`_,
but it does not support ``setGain()`` and ``setBufferSize()``.

To get started, specify a
callback function with :cpp:any:`~coralmicro::arduino::PDMClass::onReceive`,
to be notified whenever the microphone has new data to read.

Then call :cpp:any:`~coralmicro::arduino::PDMClass::begin` to start recording
with the microphone. Inside the function you passed to
:cpp:any:`~coralmicro::arduino::PDMClass::onReceive`, read the available audio
data with :cpp:any:`~coralmicro::arduino::PDMClass::read`.

The microphone remains powered and
processes input whenever there is an active audio callback.

When you're done with the microphone, call
:cpp:any:`~coralmicro::arduino::PDMClass::end()` to disable the micrphone.


**Example**:

This code listens to input from the microphone and saves data into a
``currentSamples`` buffer.

.. literalinclude:: ../arduino/libraries/Sensors/examples/Microphone/Microphone.ino
   :start-after: [start-snippet:ardu-pdm]
   :end-before: [end-snippet:ardu-pdm]

`[PDM.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/PDM/src/PDM.h>`_

.. doxygenfile:: PDM/src/PDM.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var


Filesystem
-----------------

This API allows you to create, read, and write files on the Dev Board Micro
flash storage.

This API is designed to be code-compatible with projects that use the
`Arduino SD library <https://www.arduino.cc/reference/en/libraries/sd/>`_.
However, on the Dev Board Micro, these APIs enable reading and writing files on
the on-board flash memory (not an SD card).

`[coralmicro_SD.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/CoralMicro_SD/src/coralmicro_SD.h>`_

.. doxygenfile:: CoralMicro_SD/src/coralmicro_SD.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var


I/O pins & LEDs
------------------------------

You can interact with GPIO pins (digital or analog input/output pins)
on the Dev Board Micro 12-pin headers with Arduino the same as you would on
other Arduino boards, using Arduino APIs such as
`digitalRead() <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/>`_,
`digitalWrite() <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/>`_,
`analogRead() <https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/>`_, and
`analogWrite() <https://www.arduino.cc/reference/en/language/functions/analog-io/analogwrite/>`_.
To address these GPIO pins, use the
Arduino pin names shown in figure 1.

Likewise, you can use
`digitalWrite() <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/>`_
to toggle the on-board LEDs and
`digitalRead() <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/>`_
to listen for User button presses.

.. raw:: html

   <figure id="figure1">
      <img src="images/micro-pinout.png" alt="" />
      <figcaption><b>Figure 1.</b> Pinout for the 12-pin headers, LEDs and User button</figcaption>
   </figure>

Once booting is complete, all digital/analog pins are set to a high-Z (floating)
state, except for the I2C pins, which default to high. So
be sure to call `pinMode()
<https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/>`_
before reading or writing values.

.. note::
   All GPIO pins are powered by the 1.8 V power rail, and provide a max current
   of approximately 6 mA on most pins.

**Example**:

This code toggles the on-board User LED when you press the on-board User button.

.. literalinclude:: ../arduino/libraries/Basics/examples/ButtonLED/ButtonLED.ino
   :start-after: [start-snippet:ardu-led]
   :end-before: [end-snippet:ardu-led]

For more information about using digital and analog pins, see the
`Arduino GPIO documentation
<https://docs.arduino.cc/learn/starting-guide/getting-started-arduino#gpio--pin-management>`_.


I2C
------

You can interact with I2C devices on the Dev Board Micro using the `Arduino Wire
library
<https://www.arduino.cc/reference/en/language/functions/communication/wire/>`_.

Only one pair of I2C lines on the 12-pin headers is available in Arduino,
so you do not need to specify the pin names. It is assumed that you are
connected to ``D0`` and ``D3`` (shown in figure 1).

**Example**:

This code sends messages to an I2C device that's connected to ``D0`` and ``D3``.

.. literalinclude:: ../arduino/libraries/Basics/examples/I2CTarget/I2CTarget.ino
   :start-after: [start-snippet:ardu-i2c]
   :end-before: [end-snippet:ardu-i2c]



SPI
-----------------

You can interact with Serial Peripheral Interface (SPI) devices connected to
the SPI pins on the 12-pin headers (see figure 1), using the APIs described
below. The Dev Board Micro must be the controller.

Only one SPI bus available on the 12-pin headers, so you do not
need to specify the pin names.

.. note::
   This SPI is designed to be compatible with the `Arduino SPI library
   <https://www.arduino.cc/reference/en/language/functions/communication/spi/>`_,
   but our API is a little different because the Dev Board Micro does not
   support using SPI from interrupts. Thus, instead of ``beginTransaction()``,
   you must set the `SPISettings
   <https://www.arduino.cc/reference/en/language/functions/communication/spi/spisettings/>`_
   with :cpp:any:`~coralmicro::arduino::HardwareSPI::updateSettings` and then
   call :cpp:any:`~coralmicro::arduino::HardwareSPI::begin`. Also,
   legacy functions ``setBitOrder()`` and ``setClockDivider()`` are not
   supported because you should instead use
   :cpp:any:`~coralmicro::arduino::HardwareSPI::updateSettings`.

To learn more about using SPI with Arduino, read the `Arduino & SPI
documentation <https://docs.arduino.cc/learn/communication/spi>`_.

**Example** (from ``sketches/SPI/``):

.. literalinclude:: ../arduino/libraries/Basics/examples/SPI/SPI.ino
   :start-after: [start-snippet:ardu-spi]
   :end-before: [end-snippet:ardu-spi]


`[SPI.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/SPI/src/SPI.h>`_

.. doxygenfile:: SPI/src/SPI.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var



Network
-----------------

Sockets
++++++++++

These APIs define the basic interface for socket connections, whether using
Wi-Fi or Ethernet.

To open a socket as a client or server, you should use
the corresponding typedef aliases, corresponding to the type of network
connection you're using:
:cpp:any:`~coralmicro::arduino::WiFiClient`/:cpp:any:`~coralmicro::arduino::WiFiServer`
or
:cpp:any:`~coralmicro::arduino::EthernetClient`/:cpp:any:`~coralmicro::arduino::EthernetServer`.


.. doxygenfile:: SocketClient.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var

.. doxygenfile:: SocketServer.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var



Wi-Fi
+++++++

.. note::
   Requires the Coral Wireless Add-on board.

To use Wi-Fi on the Dev Board Micro:

1. `Connect the Wireless Add-on board </docs/dev-board-micro/wireless-addon/>`_.

2. Use :cpp:any:`~coralmicro::arduino::WiFiClass` to connect to a Wi-Fi network.

3. Use :cpp:any:`~coralmicro::arduino::WiFiClient` to connect to a server,
   or use :cpp:any:`~coralmicro::arduino::WiFiServer` to host a server on the
   board.


`[WiFi.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/WiFi/src/WiFi.h>`_

.. doxygenfile:: WiFi/src/WiFi.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var



Ethernet
+++++++++

.. note::
   Requires the Coral PoE Add-on board.

To use Ethernet on the Dev Board Micro:

1. `Connect the PoE Add-on board </docs/dev-board-micro/poe-addon/>`_.

2. Call :cpp:any:`coralmicro::arduino::EthernetClass::begin()` to enable the
   Ethernet connection.

3. Use :cpp:any:`~coralmicro::arduino::EthernetClient` to connect to a server,
   or use :cpp:any:`~coralmicro::arduino::EthernetServer` to host a server on
   the board.


`[Ethernet.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/Ethernet/src/Ethernet.h>`_

.. doxygenfile:: Ethernet/src/Ethernet.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var




TensorFlow Lite
-----------------

To run inference with TensorFlow Lite models in Arduino, you'll use the same
:doc:`TensorFlow Lite Micro APIs <tensorflow>` that are used with FreeRTOS apps.
Of course, the rest of your code can continue using the Arduino language and
other Arduino-style APIs such as `Camera <#camera>`_.


**Example**:

This code performs object detection with TensorFlow Lite using images from
the camera.

.. literalinclude:: ../arduino/libraries/TensorFlow/examples/DetectObjects/DetectObjects.ino
   :start-after: [start-snippet:ardu-detection]
   :end-before: [end-snippet:ardu-detection]
