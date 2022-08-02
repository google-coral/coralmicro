Audio APIs
=============================
All microphone interactions are handled by :cpp:any:`~coralmicro::AudioDriver`,
which directly interacts with the on-board PDM microphone and processes audio.
The Audio APIs are a series of of APIs split between `audio_driver`,
which handles creating and allocating space for :cpp:any:`~coralmicro::AudioDriver`,
an `audio_service`, which creates wrappers for :cpp:any:`~coralmicro::AudioDriver`
and allow for higher levels of abstraction which are much more easily handled by the user.

Within `audio_driver` are three classes: cpp:any:`~coralmicro::AudioDriver`,
cpp:any:`~coralmicro::AudioDriverBuffers`, and cpp:any:`~coralmicro::AudioDriverConfig`.
cpp:any:`~coralmicro::AudioDriver` is used to create an audio driver.
The audio driver turns on the microphone and processes the audio through a callback method which is called by `Enable()`.
An Audio Driver is created using cpp:any:`~coralmicro::AudioDriverBuffers` which tracks the total space allocated for an audio driver.
cpp:any:`~coralmicro::AudioDriverConfig` is used to represent the config for an cpp:any:`~coralmicro::AudioDriver`.
That config is used to track the space used for a specific audio driver and can be checked against cpp:any:`~coralmicro::AudioDriverBuffers`
to see if space is available for the audio driver.

The file `audio_service` is used to offer higher level wrappers for cpp:any:`~coralmicro::AudioDriver` which are
cpp:any:`~coralmicro::AudioReader` and cpp:any:`~coralmicro::AudioService`. The `audio_service` file also includes an implementation of a common use case
in the form of cpp:any:`~coralmicro::LatestSamples`. The cpp:any:`~coralmicro::AudioReader` class is created using an cpp:any:`~coralmicro::AudioDriver`
and cpp:any:`~coralmicro::AudioDriverConfig` that is used to check if there is available space for the Audio Driver.
Audio Reader owns an audio driver which it `Enables()` upon creation which it uses to fill an intermediate ring buffer. Its main use case is to handle continuously filling a ring buffer with audio samples.

The cpp:any:`~coralmicro::AudioService` is the top level class and is the main class for a user to handle audio processing.
It also uses an AudioDriver and AudioDriverConfig to create an AudioReader that handles filling a ring buffer so all audio is already handled.
AudioService creates a callback method which the user writes to control how to process audio which is done with `AddCallback()`.
These callbacks are then handled with a dedicated FreeRTOS task.
Multiple callbacks can be given for one AudioService. The class cpp:any:`~coralmicro::LatestSamples` offers a class to store the latest
`num_samples` amount of audio samples, where `num_samples` is given as an input. To use LatestSamples simply
create an instance of LatestSamples and AudioService then call `AddCallback()` to add cpp:any:`~coralmicro::LatestSamples::Append()`.
The callback `Append()` populates LatestSamples to be accessed by `AccessLatestSamples()` and can be copied with `CopyLatestSamples()`.




Audio service
----------------

`[audio_service.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/audio/audio_service.h>`_

.. doxygenfile:: libs/audio/audio_service.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib


Audio driver
----------------

`[audio_driver.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/audio/audio_driver.h>`_

.. doxygenfile:: libs/audio/audio_driver.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib
