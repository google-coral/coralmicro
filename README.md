# Coral Dev Board Micro source code (coralmicro)

This repository contains all the code required to build apps for the [Coral Dev
Board Micro](https://coral.ai/products/dev-board-micro). The Dev Board Micro is
based on the NXP RT1176 microcontroller (dual-core MCU with Cortex M7 and M4)
and includes an on-board camera (324x324 px), a microphone, and a Coral Edge TPU
to accelerate TensorFlow Lite models.

The software platform for Dev Board Micro is called `coralmicro` and is based
on [FreeRTOS](https://www.freertos.org/). It also includes libraries for
compatibility with the Arduino programming language.

The `coralmicro` build system is based on CMake and includes support for Make
and Ninja builds. After you build the included projects, you can flash
them to your board with the included flashtool (`scripts/flashtool.py`).

![main](https://github.com/google-coral/coralmicro/actions/workflows/ci.yml/badge.svg?event=push)
![arduino](https://github.com/google-coral/coralmicro/actions/workflows/arduino.yml/badge.svg?event=push)


## Documentation

+ [Get Started with the Dev Board Micro](https://coral.ai/docs/dev-board-micro/get-started/)

+ [Get Started with Arduino](https://coral.ai/docs/dev-board-micro/arduino/)

+ [Build an out-of-tree project](https://github.com/google-coral/coralmicro-out-of-tree-sample/blob/main/README.md)

+ [coralmicro API reference](http://coral.ai/docs/reference/micro/)

+ [coralmicro examples](/examples/)



## Get the code

1. Clone `coralmicro` and all submodules:

    ```bash
    git clone --recurse-submodules -j8 https://github.com/google-coral/coralmicro
    ```

2. Install the required tools:

    ```bash
    cd coralmicro && bash setup.sh
    ```


## Build the code

This builds everything in a folder called `build` (or you can specify a
different path with `-b`, but if you do then you must also specify that path
everytime you call `flashtool.py`):

```bash
bash build.sh
```

## Flash the board

This example blinks the board's green LED:

```bash
python3 scripts/flashtool.py -e blink_led
```

You can see the code at [examples/blink_led/](examples/blink_led/).


### Reset the board to Serial Downloader

Flashing the Dev Board Micro might fail sometimes and you can usually solve
it by starting Serial Downloader mode in one of two ways:

+ Hold the User button while you press the Reset button.
+ Or, hold the User button while you plug in the USB cable.

Then try flashing the board again.

For more details, see the [troubleshooting info on
coral.ai](https://coral.ai/docs/dev-board-micro/get-started/#serial-downloader).


## Update the repo

Use the following commands to keep all coralmicro submodules in sync (rebasing your current branch):

```bash
git fetch origin

git rebase origin/main

git submodule update --init --recursive
```

