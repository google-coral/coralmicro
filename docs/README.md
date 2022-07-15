# coralmicro API docs

This directory holds the source files required to build the API reference for
the coralmicro C++ library.

Essentially, you just need to run `makedocs.sh` to generate the docs.

Of course, it requires a few tool dependencies. So if it's your first time,
you should set up as follows:

```
# You should use a Python virtual environment (definitely need Python3):
python3 -m venv ~/.my_venvs/coraldocs
source ~/.my_venvs/coraldocs/bin/activate

# Navigate to the micro/docs/ directory.

# Install doxygen:
sudo apt-get install doxygen

# Install other dependencies:
python3 -m pip install -r requirements.txt

# Build the docs:
bash makedocs.sh
```

The results are output in `_build/`. The `_build/preview/` files are for local
viewing--just open the `index.html` page. The `_build/web/` files are designed
for publishing on the Coral website.

For more information about the syntax in the RST files, see the
[breathe docs](https://breathe.readthedocs.io/en/latest/directives.html) and
[reStructuredText docs](http://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html).
