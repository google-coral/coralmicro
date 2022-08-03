# Multicore Model Cascade

The `multicore_model_cascade` application demonstrates running a cascade of machine learning models across the multiple cores of the Dev Board Micro.
The first step in the cascade is a person detection model, running on the M4 core.
If the person detection model indicates that it has detected someone in the camera frame, it stops running and wakes the M7 core.
When awoken, the M7 core will run a TPU-based pose detection model. If this does not successfully detect a pose for 5 seconds, it will stop and transition back to the M4.

## Running the application

There are three variants of the application, differentiated by the network interface that is used: USB, Wi-Fi, or Ethernet; as well as a client for the host machine.

To connect to the RPC client over USB (Linux only), flash the USB variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade
```

To connect to the RPC client over Wi-Fi, flash the Wi-Fi variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade --subapp multicore_model_cascade_wifi
```

To connect to the RPC client over Ethernet, flash the Ethernet variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade --subapp multicore_model_cascade_ethernet
```

While the application is running, the orange status LED in the corner of the device will flash on and off, and the camera LED will illuminate.
If the M4 detects a person and hands over control to the M7, the green LED in the center of the board will illuminate.

## Running the host client
To be able to see what the device is seeing, there is a Python application that will retrieve camera frames and pose keypoints, and render them.

To run with default settings, over USB:
```
python3 apps/multicore_model_cascade/multicore_model_cascade.py
```

To see more options:
```
python3 apps/multicore_model_cascade/multicore_model_cascade.py --help
```

For use on Wi-Fi or Ethernet, the `--device_ip_address` flag can be used to provide the address of the device on the network.

## Demo version
In addition to the standard version of the application, there is a "demo" version that uses timers to transition through the cascade instead of the model output.

To flash the USB only variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade --subapp multicore_model_cascade_demo
```

To flash the Wi-Fi variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade --subapp multicore_model_cascade_demo_wifi
```

To flash the Ethernet variant:
```
python3 scripts/flashtool.py --build_dir build --app multicore_model_cascade --subapp multicore_model_cascade_demo_ethernet
```
