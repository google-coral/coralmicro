Filesystem APIs
=============================

APIs to read and write files on the Dev Board Micro flash memory,
based on littlefs.

For example, the following code demonstrates a variety of file operations
(from ``examples/file_read_write/``):

.. literalinclude:: ../examples/file_read_write/file_read_write.cc
   :start-after: [start-sphinx-snippet:file-rw]
   :end-before: [end-sphinx-snippet:file-rw]


`[filesystem.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/filesystem.h>`_

.. doxygenfile:: base/filesystem.h
