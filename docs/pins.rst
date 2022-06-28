I/O Pin Control APIs
=============================

ADC / DAC
-------------------------

APIs for analog input/output.

**Example** (from `examples/analog/`):

.. literalinclude:: ../examples/analog/analog.cc
   :start-after: [start-sphinx-snippet:dac-adc]
   :end-before: [end-sphinx-snippet:dac-adc]

`[analog.h source] <https://github.com/google-coral/micro/blob/master/libs/base/analog.h>`_

.. doxygenfile:: base/analog.h

GPIO
-------------------------

`[gpio.h source] <https://github.com/google-coral/micro/blob/master/libs/base/gpio.h>`_

.. doxygenfile:: base/gpio.h

PWM
-------------------------

APIs to control PWM pins.

**Example** (from `examples/pwm/`):

.. literalinclude:: ../examples/pwm/pwm.cc
   :start-after: [start-sphinx-snippet:pwm]
   :end-before: [end-sphinx-snippet:pwm]

`[pwm.h source] <https://github.com/google-coral/micro/blob/master/libs/base/pwm.h>`_

.. doxygenfile:: base/pwm.h


LEDs
-------------------------

APIs to control on-board LEDs.

**Example** (from `examples/blink_led_m4/`):

.. literalinclude:: ../examples/blink_led_m4/blink_led_m4.cc
   :start-after: [start-sphinx-snippet:blink-led]
   :end-before: [end-sphinx-snippet:blink-led]

`[led.h source] <https://github.com/google-coral/micro/blob/master/libs/base/led.h>`_

.. doxygenfile:: base/led.h
