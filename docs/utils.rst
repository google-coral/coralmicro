Utility APIs
=============================

Miscellaneous helper APIs.


JPEGs
-------------------------

APIs to create JPEG files from RGB images.

For example, this code shows how to create a JPEG with an image captured from
the camera (within a callback for an :cpp:any:`~coralmicro::HttpServer`;
from ``examples/camera_streaming_http/``):

.. literalinclude:: ../examples/camera_streaming_http/camera_streaming_http.cc
   :start-after: [start-snippet:jpeg]
   :end-before: [end-snippet:jpeg]

`[jpeg.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/libjpeg/jpeg.h>`_

.. doxygenfile:: libjpeg/jpeg.h


Strings
-------------------------

`[strings.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/strings.h>`_

.. doxygenfile:: base/strings.h


Timers
------------

`[timer.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/timer.h>`_

.. doxygenfile:: base/timer.h


Random numbers
-----------------

`[random.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/random.h>`_

.. doxygenfile:: base/random.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Temperature sensors
-------------------------

`[tempsense.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/tempsense.h>`_

.. doxygenfile:: base/tempsense.h


Cryptography
-------------------------

The following functions are convenience helpers for basic usage of the a71ch
crypto chip with a more modern C++ style. For the full a71ch's API, see the
reference manual at `nxp.com/a71ch <https://www.nxp.com/a71ch>`_.

.. note::
   The following APIs can be used together with the full a71ch APIs, however,
   you must always call :cpp:any:`coralmicro::A71ChInit()` first.

For example, this code shows how to use a variety of the available APIs
(from ``examples/security/``):

.. literalinclude:: ../examples/security/security.cc
   :start-after: [start-snippet:crypto]
   :end-before: [end-snippet:crypto]

`[a71ch.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/a71ch/a71ch.h>`_

.. doxygenfile:: a71ch/a71ch.h
