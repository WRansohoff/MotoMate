# Firmware Documentation

I'd like to put together some proper documentation for the firmware code, because it's a good habit to get into and it might make this project easier to follow.

Currently, there is not much information included in these docs, but I plan to fill them out as I learn more about how to use Sphinx with a C project.

It looks like Sphinx is usually used with Python projects, but there is a package called Hawkmoth which uses Python's clang bindings to extend it to C. Installation instructions can be found in the project's repository:

https://github.com/jnikula/hawkmoth

You'll probably need to set the LD\_LIBRARY\_PATH environment variable before running Sphinx:

`export LD_LIBRARY_PATH=$(llvm-config --libdir)`

Also, depending on how you install it, you might need to add its location (e.g. `/usr/local/lib/python3.x/dist-packages`) to your $PYTHONPATH environment variable.

# Generating the Documentation

First, install the dependencies (Python3, Sphinx, and Hawkmoth).

Then, run `make html` in this directory.

I've noticed a few discrepencies in the generated documents, but I would guess that most if it can be sorted out if I learn more about how to configure things.

The autodoc plugin seems to have trouble with multiple declarations on one line, such as `int x, y;`. And it seems to mark structs as `int`s, but that might just be because I haven't generated documentation for the struct definitions.
