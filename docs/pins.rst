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

   <figure>
      <img src="images/micro-gpio-header.png" alt="" />
      <figcaption><b>Figure 1.</b> Primary functions for the 12-pin headers</figcaption>
   </figure>


.. warning::
   When handling any of these pins, be cautious
   to avoid electrostatic discharge or contact with conductive materials
   (metals). Failure to properly handle the board can result in a short circuit,
   electric shock, serious injury, death, fire, or damage to your board and other
   property.


Pinout
----------

Figure 1 illustrates the pin locations on the board and table 1 includes the
pin names for use with the following GPIO APIs.

.. raw:: html
   :file: includes/pinout-table.html


.. note::
   All pins are powered by the 1.8 V power rail, and provide a max current of
   approximately 6 mA on most pins.


GPIO
-------------------------

Almost all digital pins on the 12-pin headers can be used for GPIO
(exceptions are the UART TX/RX and DAC pins).

To use a GPIO, specify the pin direction and pull-up or pull-down with
:cpp:any:`coralmicro::GpioSetMode` and then set the value or get the value with
:cpp:any:`coralmicro::GpioSet` and :cpp:any:`coralmicro::GpioGet()`.
See table 1 for the appropriate GPIO pin names to use with these functions.

For example::

   extern "C" void app_main(void* param) {
     coralmicro::GpioSetMode(coralmicro::Gpio::kAA, false,
                             coralmicro::GpioPullDirection::kPullDown);
     bool on = true;
     while (true) {
       on = !on;
       coralmicro::GpioSet(coralmicro::Gpio::kAA, on);
       vTaskDelay(pdMS_TO_TICKS(1000));
     }
   }

`[gpio.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/gpio.h>`_

.. doxygenfile:: base/gpio.h



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

These APIs allow you to control the LEDs built into the Dev Board Micro.
To control other LEDs attached to GPIOs, you
must instead use the `GPIO APIs <#gpio>`_, but beware that a GPIO pin
alone is not strong enough to drive an LED.

.. raw:: html

   <figure>
      <img src="images/micro-leds.png" alt="" style="max-width:280px" />
      <figcaption><b>Figure 2.</b> On-board LEDs available for control</figcaption>
   </figure>

.. note::
   The camera LED is not available with this API in order to provide consistent
   LED behavior when the camera is in operation.


**Example** (from `examples/blink_led/`):

.. literalinclude:: ../examples/blink_led/main_app.cc
   :start-after: [start-sphinx-snippet:blink-led]
   :end-before: [end-sphinx-snippet:blink-led]

`[led.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/led.h>`_

.. doxygenfile:: base/led.h
