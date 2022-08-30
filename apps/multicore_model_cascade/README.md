# Multi-core model cascade sample

This application shows how to run a cascade of
machine learning models across the two cores of the Dev Board Micro MCU.

The first step in the cascade is a person detection model (built with
[TensorFlow Lite for Microcontrollers]
(https://www.tensorflow.org/lite/microcontrollers)), running on the M4 core.
When the model detects a person in the camera frame, it suspends the M4 task
and wakes the M7 task. The M7 task runs a pose detection model that's
accelerated on the Coral Edge TPU (and makes results available over RPC). If
this does not successfully detect a pose for 5 seconds, it stops and
transitions back to the M4 person detection model.

When the M4 detects a person, it turns on the board's green LED. When the M7
starts the pose detection model, the white LED turns on to indicate the
Edge TPU is active.


## Flash the app

There are three variants of the application, differentiated by the network
interface used to send pose detection results to your computer (via the [Python
host client](#host-client)): USB, Wi-Fi, or Ethernet.

Flash the USB variant (works with Linux and Windows only):

```bash
python3 scripts/flashtool.py --app multicore_model_cascade
```

Also use the USB variant if you want to see results in the
[serial console](/docs/dev-board-micro/serial-console/).

Flash the Wi-Fi variant (requires the Coral Wireless Add-on):

```bash
python3 scripts/flashtool.py --app multicore_model_cascade \
    --subapp multicore_model_cascade_wifi
```

Flash the Ethernet variant (requires the Coral PoE Add-on):

```bash
python3 scripts/flashtool.py --app multicore_model_cascade \
    --subapp multicore_model_cascade_ethernet
```


<a name="host-client"></a>
## See results with the host client

The Python host client allows you to see camera frames and pose keypoints
on your computer.

If using the USB variant, run it with the default settings:

```bash
python3 apps/multicore_model_cascade/multicore_model_cascade.py
```

If using the Wi-Fi or Ethernet variant, specify the board's IP address
with the `--device_ip_address` flag.

The window that appears will be black until the app detects a person and
starts the pose detection model.

To see more options:

```bash
python3 apps/multicore_model_cascade/multicore_model_cascade.py --help
```


## Flash the demo mode

There is a "demo" mode that uses timers to transition through the cascade
instead of doing so based on inference results. It's available for
each variant above with another "demo" build target.

Flash the USB demo:

```bash
python3 scripts/flashtool.py --app multicore_model_cascade \
    --subapp multicore_model_cascade_demo
```

Flash the Wi-Fi demo:

```bash
python3 scripts/flashtool.py --app multicore_model_cascade \
    --subapp multicore_model_cascade_demo_wifi
```

Flash the Ethernet demo:

```bash
python3 scripts/flashtool.py --app multicore_model_cascade \
    --subapp multicore_model_cascade_demo_ethernet
```
