Cryptography
=============================

The following functions are convenience helpers for basic usage of the a71ch
crypto chip with a more modern C++ style. For the full a71ch's API, see the
reference manual at `nxp.com/a71ch <https://www.nxp.com/a71ch>`_.

.. note::
   The following APIs can be used together with the full a71ch APIs in
   ``third_party/a71ch-crypto-support/``, however,
   you must always call :cpp:any:`coralmicro::A71ChInit()` first.

For example, this code shows how to use a variety of the available APIs
(from ``examples/security/``):

.. literalinclude:: ../examples/security/security.cc
   :start-after: [start-snippet:crypto]
   :end-before: [end-snippet:crypto]

`[a71ch.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/a71ch/a71ch.h>`_

.. doxygenfile:: a71ch/a71ch.h
