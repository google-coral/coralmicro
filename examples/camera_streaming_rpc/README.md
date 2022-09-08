## camera_streaming_rpc:

- This app demonstrates the Micro Dev Board's camera capabilities on the device:
  - resize using different algorithms
  - rotate
  - different format (RGB, GRAY, RAW)
  - auto while balancing

The board host this image over a 'get_image_from_camera' rpc endpoint that takes the following parameters:

```json
[
  {
    "width": 500,
    "height": 500,
    "format": "RGB",
    "filter": "BILINEAR",
    "rotation": 0,
    "auto_white_balance": true
  }
]
```

### Flashing

This example also demonstrates how the image can be served over USB, Ethernet, or Wi-Fi network.
However, only the USB client works with the Arduino version of the app.

- USB (Linux and Windows clients only)

```
python3 scripts/flashtool.py -e camera_streaming_rpc --subapp camera_streaming_rpc_usb
```

- Ethernet

```
python3 scripts/flashtool.py -e camera_streaming_rpc --subapp camera_streaming_rpc_ethernet
```

- Wi-Fi

```
python3 scripts/flashtool.py -e camera_streaming_rpc --subapp camera_streaming_rpc_wifi \
  --wifi_ssid your-wifi-sid --wifi_psk your-wifi-password
```

### Running the app

Observes that the board starts up differently over different network mode, however it'll always output it's ip address:

```
Starting Image RPC Server on: 10.10.10.1
```

Use that ip address to run the app:

```
python3 examples/camera_streaming_rpc/camera_streaming_app.py --host_ip 10.10.10.1
```
