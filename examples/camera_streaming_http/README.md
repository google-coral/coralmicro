## camera_streaming_http

This app demonstrates how the Micro Dev Board can take an RGB image, converts it to jpeg and then serve it as a http endpoint. It also shows how the client can take that image and show it on the webpage with their own configuration choices. Since the resizing is done on the client side instead of on the device, changing the image size does not affect the image transfer latency.

There are 2 endpoints:

- `/coral_micro_camera.html` which serves the main webpage.
- `/camera_stream` which serves the image.

### Flashing

```
python3 scripts/flashtool.py --build_dir build \
  --example camera_streaming --subapp camera_streaming_http
```

### Running the app

After flashing the board, observe that it'll show the url:

```
http://10.10.10.1/coral_micro_camera.html
```

Click on that link and the app should show on your browser.
