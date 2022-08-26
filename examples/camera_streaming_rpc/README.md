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

This example also demonstrates how the image can be served over usb, ethernet, or wifi network.

### Flashing

- USB

```
python3 scripts/flashtool.py --build_dir build --example camera_streaming --subapp camera_streaming_usb
```

- Ethernet

```
python3 scripts/flashtool.py --build_dir build --example camera_streaming --subapp camera_streaming_ethernet
```

- WiFi

```
python3 scripts/flashtool.py --build_dir build \
  --example camera_streaming --subapp camera_streaming_wifi \
  --wifi_ssid your-wifi-sid --wifi_psk your-wifi-password
```

### Running the app

Observes that the board starts up differently over different network mode, however it'll always output it's ip address:

```
Starting Image RPC Server on: 10.10.10.1
```

Use that ip address to run the app:

```
python3 examples/camera_streaming/camera_streaming_app.py --host_ip 10.10.10.1
```