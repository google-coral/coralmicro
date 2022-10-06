System APIs
=============================

APIs to manage system processes.


Inter-processor communication (IPC)
-----------------------------------

APIs to initiate multicore processing and pass messages between the M7 and M4
cores.

For example, the following code (from ``examples/multi_core_ipc/``) executes on
the M7. It registers a handler to receive messages from the M4 program
right before it starts the M4. Then it starts a loop to also periodically
send messages to the M4:

.. literalinclude:: ../examples/multi_core_ipc/multi_core_ipc.cc
   :start-after: [start-sphinx-snippet:ipc-m7]
   :end-before: [end-sphinx-snippet:ipc-m7]

And the following is the counterpart code that runs on the M4. It registers
a message handler to receive incoming messages from the M7, which simply
passes another message back to the M7:

.. literalinclude:: ../examples/multi_core_ipc/multi_core_ipc_m4.cc
   :start-after: [start-sphinx-snippet:ipc-m4]
   :end-before: [end-sphinx-snippet:ipc-m4]

Notice that the data for ``IpcMessage`` uses a custom message
format, which is defined in ``examples/multi_core_ipc/example_message.h`` like
this:

.. literalinclude:: ../examples/multi_core_ipc/example_message.h
   :start-after: [start-sphinx-snippet:ipc-message]
   :end-before: [end-sphinx-snippet:ipc-message]

For information about how to get started with multicore processing with the M4,
see the guide to `create a multicore app </docs/dev-board-micro/multicore/>`_.

.. doxygenfile:: base/ipc.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum

`[ipc_m7.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/ipc_m7.h>`_

.. doxygenfile:: base/ipc_m7.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum

`[ipc_m4.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/ipc_m4.h>`_

.. doxygenfile:: base/ipc_m4.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum

`[ipc_message_buffer.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/ipc_message_buffer.h>`_

.. doxygenfile:: base/ipc_message_buffer.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum


Mutex
------------

APIs to ensure mutual-exclusive access to resources.


`[mutex.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/mutex.h>`_

.. doxygenfile:: base/mutex.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Watchdog
-------------------------

APIs to create watchdog timers that monitor the MCU behavior and reset it when
it appears to be malfunctioning.

`[watchdog.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/watchdog.h>`_

.. doxygenfile:: base/watchdog.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum


Reset
-------------------------

APIs to reset the Dev Board Micro into different states and read reset
stats.

`[reset.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/reset.h>`_

.. doxygenfile:: base/reset.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum
