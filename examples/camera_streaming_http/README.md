## camera_streaming_http

This app demonstrates how the Micro Dev Board can take an RGB image, converts it to jpeg and then serve it as a http endpoint. It also shows how the client can take that image and show it on the webpage with their own configuration choices. Since the resizing is done on the client side instead of on the device, changing the image size does not affect the image transfer latency.

There are 2 endpoints:

- `/coral_micro_camera.html` which serves the main webpage.
- `/camera_stream` which serves the image.

### Flashing

This example also demonstrates how the image can be served over USB, Ethernet, or Wi-Fi network.

- USB (Linux and Windows clients only)

```
python3 scripts/flashtool.py -e camera_streaming_http --subapp camera_streaming_http_usb
```

- Ethernet

```
python3 scripts/flashtool.py -e camera_streaming_http --subapp camera_streaming_http_ethernet
```

- Wi-Fi

```
python3 scripts/flashtool.py -e camera_streaming_http --subapp camera_streaming_http_wifi \
  --wifi_ssid your-wifi-sid --wifi_psk your-wifi-password
```

### Running the app

After flashing the board, observe that it'll show the url:

```
http://10.10.10.1/coral_micro_camera.html
```

Click on that link and the app should show on your browser.
