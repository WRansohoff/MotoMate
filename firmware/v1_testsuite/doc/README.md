# Firmware Documentation

I'd like to put together some proper documentation for the firmware code, because it's a good habit to get into and it might make this project easier to follow.

Currently, there is not much information included in these docs, but I plan to fill them out as I learn more about how to use Sphinx with a C project.

It looks like Sphinx is usually used with Python projects, but there is a package called Hawkmoth which uses Python's clang bindings to extend it to C. Installation instructions can be found in the project's repository:

https://github.com/jnikula/hawkmoth

Depending on how you install it, you might need to add its location (e.g. `/usr/local/lib/python3.x/dist-packages`) to your $PYTHONPATH environment variable.

# Generating the Documentation

First, install the dependencies (Python3, Sphinx, and Hawkmoth).

Then, run `make html` in this directory.
