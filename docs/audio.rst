Audio APIs
=============================

The Dev Board Micro has one on-board PDM microphone from which you can capture
audio using the APIs on this page.

All interactions with the microphone are handled by
:cpp:any:`~coralmicro::AudioDriver` and you can use that class to get direct
memory access to the incoming audio stream. However, we recommend you instead
use :cpp:any:`~coralmicro::AudioReader` or :cpp:any:`~coralmicro::AudioService`
to get audio samples from the microphone. These APIs
provide wrappers around the :cpp:any:`~coralmicro::AudioDriver` to simplify
the code required to properly manage the audio buffer:

+ :cpp:any:`~coralmicro::AudioReader` provides on-demand audio samples. That is,
  whenever you want to get the latest audio data, call
  :cpp:any:`~coralmicro::AudioReader::FillBuffer()` and then read the samples
  copied to the buffer.

+ :cpp:any:`~coralmicro::AudioService` provides continuous audio samples with
  a callback function. So your task will receive regular callbacks with new
  audio samples whenever the internal buffer fills up.

To use either one, create an instance of :cpp:any:`~coralmicro::AudioDriver`
(the constructor needs to know the buffer size, which you define
with :cpp:any:`~coralmicro::AudioDriverBuffers`), and then pass it to the
:cpp:any:`~coralmicro::AudioReader` or :cpp:any:`~coralmicro::AudioService`
constructor. These constructors also need to know some other audio
configurations (such as audio sample rate), which you can specify with
:cpp:any:`~coralmicro::AudioDriverConfig`. Then you're ready to start
reading audio samples. See below for more details.

.. note::
   These audio APIs are currently not compatible with M4 programs.


Audio reader
------------

The audio reader allows you to read audio samples from the Dev Board Micro's
microphone on-demand, by calling
:cpp:any:`~coralmicro::AudioReader::FillBuffer()` whenever you want to fetch
new audio samples.

This is in contrast to the :ref:`audio service<Audio service>`, which instead
continuously delivers you new audio samples in a callback function.

.. doxygenclass:: coralmicro::AudioReader
   :members:
   :undoc-members:


.. _audio-service

Audio service
-------------

The audio service allows you to continuously receive new audio samples from a
separate FreeRTOS task that fetches audio from the Dev Board Micro's
microphone and delivers them to you with one or more callback functions that
you specify with :cpp:any:`~coralmicro::AudioService::AddCallback`.

You can process the audio samples as your callback receives them or save
copies of the audio samples in an instance of
:cpp:any:`~coralmicro::LatestSamples` so you can process them later.

This is in contrast to the :ref:`audio reader<Audio reader>`, which instead
provides audio samples only when you request them.


.. doxygenclass:: coralmicro::AudioService
   :members:
   :undoc-members:

.. doxygenclass:: coralmicro::LatestSamples
   :members:
   :undoc-members:


Audio driver & configuration
----------------------------

These APIs define the microphone driver and audio configuration to
get audio samples from the Dev Board Micro's microphone.

Although you can receive audio samples directly from `AudioDriver`, it's easier
to instead use `audio reader <#audio-reader>`_ or
`audio service <#audio-service>`_.

`[audio_driver.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/audio/audio_driver.h>`_

.. doxygenfile:: libs/audio/audio_driver.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum
