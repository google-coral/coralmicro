# coralmicro examples

This directory contains code examples for the [Coral Dev Board
Micro](https://coral.ai/products/dev-board-micro), using FreeRTOS and CMake.
(For Arduino examples, instead see
[arduino/libraries/](/arduino/libraries/) or the
[Arduino starter guide](https://coral.ai/docs/dev-board-micro/arduino/).)

Once built, you can flash most of these examples by passing the directory name
to the flashtool with the `-e` (or `--example`) argument. For example (run this
from the `coralmicro` root):

```bash
# First build all examples
bash build.sh

# Then flash one
python3 scripts/flashtool.py -e blink_led
```

Some examples also require that you specify the `--subapp` argument because
the example has multiple build targets (defined in the example's
`CMakeLists.txt`).

Some examples also include a host-side Python script to communicate with the
board, such as to send RPC commands and receive images/data from the board.

See the main `.cc` file in each example for details. Or for
step-by-step instructions, see the [Dev Board Micro setup
guide](https://coral.ai/docs/dev-board-micro/get-started/).

