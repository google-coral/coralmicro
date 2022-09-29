I/O Pins APIs
=============================

The Dev Board Micro provides access to several digital pins on the
two 12-pin headers, including GPIO, PWM, I2C, and SPI. This page describes all
the coralmicro APIs available for these pins, plus APIs for the on-board LEDs.

.. note::
   The Dev Board Micro does not include header pins. For development, we
   suggest you solder header pins to the board with pins facing down
   (the direction is important to attach to breadboards and for compatibility
   with Coral cases). If you plan to put the board into a case, be sure your
   header pins are long enough to be accessible through the case.

.. raw:: html

   <figure id="figure1">
      <img src="images/micro-pinout.png" alt="" />
      <figcaption><b>Figure 1.</b> Pinout for the 12-pin headers, LEDs and User button</figcaption>
   </figure>


.. note::
   All pins are powered by the 1.8 V power rail, and provide a max current of
   approximately 6 mA on most pins.

.. warning::
   When handling any of these pins, be cautious
   to avoid electrostatic discharge or contact with conductive materials
   (metals). Failure to properly handle the board can result in a short circuit,
   electric shock, serious injury, death, fire, or damage to your board and other
   property.


GPIO
-------------------------

Almost all digital pins on the 12-pin headers can be used for GPIO
(exceptions are the UART TX/RX and DAC pins).

To use a GPIO, specify the GPIO pin name (indicated in figure 1), the
direction, and pull-up or pull-down with
:cpp:any:`coralmicro::GpioSetMode`. Then set the value or get the value with
:cpp:any:`coralmicro::GpioSet` and :cpp:any:`coralmicro::GpioGet()`.

For example::

   extern "C" void app_main(void* param) {
     coralmicro::GpioSetMode(coralmicro::Gpio::kAA, coralmicro::GpioMode::kOutput);
     bool on = true;
     while (true) {
       on = !on;
       coralmicro::GpioSet(coralmicro::Gpio::kAA, on);
       vTaskDelay(pdMS_TO_TICKS(1000));
     }
   }

`[gpio.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/gpio.h>`_

.. doxygenfile:: base/gpio.h



PWM
-------------------------

There are two pins pre-configured for pulse-width modulation (PWM)
on the left header (pins 9 and 10).

To use a PWM pin, you must first call
:cpp:any:`coralmicro::PwmInit`. Then specify the PWM settings in an instance of
:cpp:any:`coralmicro::PwmPinConfig` and pass it to
:cpp:any:`coralmicro::PwmEnable`.

**Example** (from `examples/pwm/`):

.. literalinclude:: ../examples/pwm/pwm.cc
   :start-after: [start-sphinx-snippet:pwm]
   :end-before: [end-sphinx-snippet:pwm]

`[pwm.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/pwm.h>`_

.. doxygenfile:: base/pwm.h



ADC / DAC
-------------------------

There are two analog-to-digital converter (ADC) pins pre-configured on the left
header (pins 3 and 4) and one digital-to-analog converter (DAC) pin on the right
header (pin 9).

To use ADC in either single-ended or differential mode, you must first call
:cpp:any:`coralmicro::AdcInit` and
:cpp:any:`coralmicro::AdcCreateConfig`. Then you can read input with
:cpp:any:`coralmicro::AdcRead`.

For DAC, first call :cpp:any:`coralmicro::DacInit()` and
:cpp:any:`coralmicro::DacEnable`. Then you can write output with
:cpp:any:`coralmicro::DacWrite`.

**Example** (from `examples/analog/`):

.. literalinclude:: ../examples/analog/analog.cc
   :start-after: [start-sphinx-snippet:dac-adc]
   :end-before: [end-sphinx-snippet:dac-adc]

`[analog.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/analog.h>`_

.. doxygenfile:: base/analog.h



I2C
-------------------------

You can use the board as either the device controller or target, using
either of two I2C lines on the 12-pin headers:

+ I2C1 (`I2c::kI2c1`)

   + SDA is pin 10 on the right side
   + SCL is pin 11 on the left side

+ I2C6 (`I2c::kI2c6`)

   + SDA is pin 12 on the right side
   + SCL is pin 11 on the right side


**Example** (from `examples/i2c/controller.cc`):

.. literalinclude:: ../examples/i2c/controller.cc
   :start-after: [start-sphinx-snippet:i2c-controller]
   :end-before: [end-sphinx-snippet:i2c-controller]

`[i2c.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/i2c.h>`_

.. doxygenfile:: base/i2c.h



SPI
-------------------------

There is one serial peripheral interface (SPI) bus pre-configured on the
right header:

+ Pin 5 is chip select
+ Pin 6 is clock
+ Pin 7 is data out
+ Pin 8 is data in


**Example** (from `examples/spi/`):

.. literalinclude:: ../examples/spi/spi.cc
   :start-after: [start-sphinx-snippet:spi]
   :end-before: [end-sphinx-snippet:spi]

`[spi.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/spi.h>`_

.. doxygenfile:: base/spi.h



LEDs
-------------------------

These APIs allow you to control the LEDs built into the Dev Board Micro,
indicated in `figure 1 <#figure1>`_.

To control other LEDs attached to GPIO pins, you
must instead use the `GPIO APIs <#gpio>`_, but beware that a GPIO pin
alone is not strong enough to drive an LED.

.. note::
   The camera LED is not available with this API because it's intended to give
   people awareness that images are being captured by an image sensor for
   storage, processing, and/or transmission. We strongly recommend this LED
   behavior remain unchanged and always be visible to users.


**Example** (from `examples/blink_led/`):

.. literalinclude:: ../examples/blink_led/blink_led.cc
   :start-after: [start-sphinx-snippet:blink-led]
   :end-before: [end-sphinx-snippet:blink-led]

`[led.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/led.h>`_

.. doxygenfile:: base/led.h
