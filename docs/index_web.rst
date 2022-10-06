coralmicro API overview
========================

The ``coralmicro`` library is written in C++ and is designed for
the `Coral Dev Board Micro </products/dev-board-micro/>`_. It is not compatible
with other Coral boards or accelerators.

Because the Dev Board Micro is a microcontoller platform, it doesn't run a
general-purpose operating system (such as Linux). Instead, it
runs just one application at a time. So your application must include
implementation for all the system features it needs, such as a task manager, a
filesystem, or a camera interface. Of course, you don't have to write those
yourself; ``coralmicro`` provides them for you and only the parts you use
are compiled into your app.

``coralmicro`` is based on
`FreeRTOS <https://www.freertos.org/index.html>`_, which provides real-time
operating system features such as multitasking, task scheduling, and task
prioritization. On top of FreeRTOS, ``coralmicro`` adds APIs to interact with
the camera, microphone, GPIOs, Edge TPU, and much more, all of which
are documented in the pages below.

This API reference doesn't cover the entire ``coralmicro`` platform, but it
provides everything that most apps need. You can
find the rest in the `coralmicro source code
<https://github.com/google-coral/coralmicro/>`_.

If you're just getting started with the board, see the `Dev Board Micro setup
guide </docs/dev-board-micro/get-started/>`_. Or to see how to set up your own
project (either as an in-tree or out-of-tree project), build it, and flash it,
see how to
`build apps with FreeRTOS </docs/dev-board-micro/freertos/>`_.


API summary
-------------


+ :doc:`tensorflow`

   APIs to execute ML models on the MCU or on the Edge TPU, including
   helper APIs that format the output tensors for popular models such as
   classification, object detection, and pose estimation.

+ :doc:`camera`

   APIs to capture images from the Dev Board Micro's on-board camera.

+ :doc:`audio`

   APIs to capture audio from the Dev Board Micro's on-board microphone.

+ :doc:`pins`

   APIs to interact with digital and analog pins on the two 12-pin
   headers (GPIO, ADC, DAC, PWM, I2C, SPI), plus APIs to toggle the on-board
   LEDs.

+ :doc:`filesystem`

   APIs to read and write files on the flash memory.

+ :doc:`network`

   APIs for everything network related, including Wi-Fi, Ethernet,
   HTTP and RPC servers.

+ :doc:`bluetooth`

   APIs for wireless communication with either classic Bluetooth or
   Bluetooth low-energy.

+ :doc:`usb`

   APIs for everything USB related, including operating the board as
   a USB host or USB device, and emulating Ethernet over USB.

+ :doc:`system`

   Various system-level APIs, such as to perform inter-process communication
   between MCU cores, establish mutex locks, watchdogs, and reset the board.

+ :doc:`crypto`

   APIs to perform various cryptography tasks with the a71ch chip.

+ :doc:`utils`

   Various utility APIs, such as to manipulate strings, generate random numbers,
   create timers, read temperature sensors, and create JPEG files.


And if you want to build apps for the Dev Board Micro with Arduino, this page
includes additional APIs that make some of the above features available with an
Arduino-style programming interface (these APIs are meant for Arduino sketches
only, not for traditional C++ apps):

+ :doc:`arduino`



API index
-----------

.. raw:: html

   {! src/static/docs/reference/micro/genindex.md !}
