Bluetooth APIs
===============

APIs for classic Bluetooth (BR/EDR) and Bluetooth low-energy (LE).

.. note::
   Using Bluetooth requires the Coral Wireless Add-on board (or similar add-on).

You need to include only the
``libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h`` header.
This header defines just the ``InitEdgefastBluetooth()`` function but also
includes all the other Bluetooth APIs shown here.

`[bluetooth.h source] <https://github.com/google-coral/coralmicro/blob/main/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include/bluetooth/bluetooth.h>`_

.. doxygenfile:: nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h


----

.. doxygenfile:: bluetooth/bluetooth.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type
