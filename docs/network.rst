Network APIs
=============

APIs to perform various networking tasks, such as open server sockets on the
Dev Board Micro, connect to other sockets as a client, create HTTP servers,
connect to Wi-Fi networks, and more.

.. note::
   If using SSL (with Curl or Mbed TLS), you must first call
   :cpp:any:`~coralmicro::A71ChInit()` to initialize the a71ch crypto chip.

TCP/IP sockets
-------------------------

APIs to create TCP/IP network socket servers and client connections with
the Dev Board Micro.

All socket communication is facilitated by read/write functions that require
a socket file descriptor, which you can get by creating a new server with
:cpp:any:`~coralmicro::SocketServer()`, accepting an incoming client connection
with :cpp:any:`~coralmicro::SocketAccept()`, or initiating a client connection
with :cpp:any:`~coralmicro::SocketClient()`.

For an example, see ``examples/audio_streaming/``.

`[network.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/network.h>`_

.. doxygenfile:: base/network.h


Ethernet network
-------------------------

APIs to get online via Ethernet and read network details.

.. note::
   Using Ethernet requires the Coral PoE Add-on board (or similar add-on).

To get started, just call :cpp:any:`~coralmicro::EthernetInit()`. If it returns
true, then you're connected to the network and you can use other functions to
query details such as the board IP address and MAC address.

For example code that uses Curl over Ethernet, see ``examples/curl/``.

`[ethernet.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/ethernet.h>`_

.. doxygenfile:: base/ethernet.h


Wi-Fi network
-------------------------

APIs to get online and perform basic Wi-Fi tasks, such as scan for Wi-Fi
networks, connect to a Wi-Fi network, and read network details.

.. note::
   Using Wi-Fi requires the Coral Wireless Add-on board (or similar add-on).

To get started, call :cpp:any:`~coralmicro::WiFiTurnOn()` to enable the Wi-Fi
module, and then call :cpp:any:`~coralmicro::WiFiConnect()` to connect to a
Wi-Fi network.

For example code that uses Curl over Wi-Fi, see ``examples/curl/``.

`[wifi.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/wifi.h>`_

.. doxygenfile:: base/wifi.h


HTTP server
-------------------------

APIs to create an HTTP server on the Dev Board Micro.

To get started, create an instance of
:cpp:any:`~coralmicro::HttpServer` and call :cpp:any:`~coralmicro::UriHandler()`
to specify a callback function that handles incoming requests. Then pass the
``HttpServer`` to :cpp:any:`~coralmicro::UseHttpServer`.

For example, this code creates an HTTP server that responds with a "Hello World"
web page (from ``examples/http_server/``):

.. literalinclude:: ../examples/http_server/http_server.cc
   :start-after: [start-snippet:http-server]
   :end-before: [end-snippet:http-server]


`[http_server.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/http_server.h>`_

.. doxygenfile:: base/http_server.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


RPC HTTP server
-------------------------

APIs to create an RPC server on the Dev Board Micro.

The setup is the same as :cpp:any:`~coralmicro::HttpServer` but you must pass
an instance of :cpp:any:`~coralmicro::JsonRpcHttpServer` to
:cpp:any:`~coralmicro::UseHttpServer` and also initialize ``jsonrpc``.

.. note::
   You must also include ``third_party/mjson/src/mjson.h`` to initialize
   with ``jsonrpc_init()`` and specify RPC functions with ``jsonrpc_export()``.

For example, this code creates an RPC server that responds with the board
serial number (from ``examples/rpc_server/``):

.. literalinclude:: ../examples/rpc_server/rpc_server.cc
   :start-after: [start-snippet:rpc-server]
   :end-before: [end-snippet:rpc-server]

`[rpc_http_server.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/rpc/rpc_http_server.h>`_

.. doxygenfile:: rpc/rpc_http_server.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


RPC utils
-------------------------

`[rpc_utils.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/rpc/rpc_utils.h>`_

.. doxygenfile:: rpc/rpc_utils.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Device information
-------------------------

`[utils.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/base/utils.h>`_

.. doxygenfile:: base/utils.h
