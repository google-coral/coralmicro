# coralmicro examples

These are code examples for the [Coral Dev Board
Micro](https://coral.ai/products/dev-board-micro).

Most of them can be flashed to the board by simply passing the directory name
to the flashtool with the `-e` (or `--example`) argument. For example (run this
from the `coralmicro` root):

```bash
# First build all examples
bash build.sh

# Then flash one
python3 scripts/flashtool.py -e blink_led
```

Some examples also require that you specify the `--subapp` argument because
the example has multiple build targets.

Some examples also include a host-side Python script to communicate with the
board, such as to send RPC commands and receive images/data from the board.

See the main `.cc` file in each example for instructions.
