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

**Example** (from ``sketches/Camera/``):

.. literalinclude:: ../arduino/libraries/examples/examples/Camera/Camera.ino
   :start-after: [start-snippet:ardu-camera]
   :end-before: [end-snippet:ardu-camera]

`[coralmicro_camera.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/CoralMicro_Camera/coralmicro_camera.h>`_

.. doxygenfile:: CoralMicro_Camera/coralmicro_camera.h
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


**Example** (from ``sketches/PDM/``):

This code listens to input from the microphone and saves data into a
``currentSamples`` buffer.

.. literalinclude:: ../arduino/libraries/examples/examples/PDM/PDM.ino
   :start-after: [start-snippet:ardu-pdm]
   :end-before: [end-snippet:ardu-pdm]

`[PDM.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/PDM/PDM.h>`_

.. doxygenfile:: PDM/PDM.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var


Filesystem
-----------------

This API allows you to create, read, and write files on the Dev Board Micro
flash storage.

This API is designed to be code-compatible with projects that use the
`Arduino SD library <https://www.arduino.cc/reference/en/libraries/sd/>`_.
However, on the Dev Board Micro, these APIs enable reading and writing files on
the on-board flash memory (not an SD card).

`[coralmicro_SD.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/CoralMicro_SD/coralmicro_SD.h>`_

.. doxygenfile:: CoralMicro_SD/coralmicro_SD.h
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

The Dev Board Micro also includes a few built-in LEDs that you can control with
`digitalWrite() <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/>`_. To address these LEDs and the
User button, use the pin names in table 2.

.. raw:: html

   <figure>
      <img src="images/micro-gpio-header.png" alt="" />
      <figcaption><b>Figure 1.</b> Primary functions and pin names for the 12-pin headers</figcaption>
   </figure>

Once booting is complete, all digital/analog pins are set to a high-Z (floating)
state, except for the I2C pins, which default to high. So
be sure to call `pinMode()
<https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/>`_
before reading or writing values.

.. note::
   All GPIO pins are powered by the 1.8 V power rail, and provide a max current
   of approximately 6 mA on most pins.

.. raw:: html

   <table class="style0" style="font-size:85%">
   <caption><strong>Table 2. </strong>Button and LED pin names </caption>
     <thead>
       <tr>
         <th class="style1"><strong>Arduino name</strong></th>
         <th class="style1"><strong>Function</strong></th>
       </tr>
    </thead>
     <tbody>
       <tr>
         <td class="style99">PIN_BTN</td>
         <td class="">User button (input)</td>
       </tr>
       <tr>
         <td class="style99">PIN_LED_USER</td>
         <td class="">User LED (output)</td>
       </tr>
       <tr>
         <td class="style99">PIN_LED_STATUS</td>
         <td class="">Status LED (output)</td>
       </tr>
       <tr>
         <td class="style99">PIN_LED_TPU</td>
         <td class="">Edge TPU LED (output); available only when the
            Edge TPU is enabled</td>
       </tr>
     </tbody>
  </table>

**Example** (from ``sketches/ButtonLED/``):

This code toggles the on-board User LED when you press the on-board User button.

.. literalinclude:: ../arduino/libraries/examples/examples/ButtonLED/ButtonLED.ino
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

**Example** (from ``sketches/I2CM-Writer/``):

This code sends messages to an I2C device that's connected to ``D0`` and ``D3``.

.. literalinclude:: ../arduino/libraries/examples/examples/I2CM-Writer/I2CM-Writer.ino
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

.. literalinclude:: ../arduino/libraries/examples/examples/SPI/SPI.ino
   :start-after: [start-snippet:ardu-spi]
   :end-before: [end-snippet:ardu-spi]


`[SPI.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/SPI/SPI.h>`_

.. doxygenfile:: SPI/SPI.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var



Wi-Fi
-----------------

**Requires the Coral Wireless Add-on.**

These APIs allow you to get online and perform basic Wi-Fi tasks (when the board
is attached to the Coral Wireless Add-on), such as scan for Wi-Fi networks,
connect to a Wi-Fi network, and read network details.

This API is designed to be code-compatible with projects that use the ``WiFi``
class from the `Arduino WiFi
library <https://www.arduino.cc/reference/en/libraries/wifi/>`_. For other
networking features such as hosting local servers on the Dev Board Micro, you
should use the `coralmicro Network APIs </docs/reference/micro/network/>`_.

**Example** (from ``sketches/WiFiConnect/``):

This code connects to a Wi-Fi network and prints network details.

.. literalinclude:: ../arduino/libraries/WiFiExamples/examples/WiFiConnect/WiFiConnect.ino
   :start-after: [start-snippet:ardu-wifi]
   :end-before: [end-snippet:ardu-wifi]

`[WiFi.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/WiFi/WiFi.h>`_

.. doxygenfile:: WiFi/WiFi.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var



Ethernet
-----------------

**Requires the Coral PoE Add-on.**

These APIs allow you to get online via Ethernet and get network
details (when the board is attached to the Coral PoE Add-on).

For networking features such as hosting local servers on the Dev Board Micro,
you should use the `coralmicro Network APIs </docs/reference/micro/network/>`_.

**Example** (from ``sketches/EthernetClient/``):

This code enables the Ethernet connection and .

.. literalinclude:: ../arduino/libraries/PoEExamples/examples/EthernetClient/EthernetClient.ino
   :start-after: [start-snippet:ardu-ethernet-client]
   :end-before: [end-snippet:ardu-ethernet-client]

`[WiFi.h source] <https://github.com/google-coral/coralmicro/blob/main/arduino/libraries/Ethernet/Ethernet.h>`_

.. doxygenfile:: Ethernet/Ethernet.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib public-type enum typedef property var


TensorFlow Lite
-----------------

To run inference with TensorFlow Lite models in Arduino, you'll use the same
:doc:`TensorFlow Lite Micro APIs <tensorflow>` that are used with FreeRTOS apps.
Of course, the rest of your code can continue using the Arduino language and
other Arduino-style APIs such as `Camera <#camera>`_.


**Example** (from ``sketches/CameraDetection/``):

This code performs object detection with TensorFlow Lite using images from
the camera.

.. literalinclude:: ../arduino/libraries/examples/examples/CameraDetection/CameraDetection.ino
   :start-after: [start-snippet:ardu-detection]
   :end-before: [end-snippet:ardu-detection]
