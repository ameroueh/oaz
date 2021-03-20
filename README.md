# OAZ: An open implementation of Alpha Zero

## What is OAZ?

OAZ is a C++ library designed to carry out experiments with the Alpha Zero algorithm.
It provides a Python front-end, PyOAZ, to run high-level tasks such as orchestrating 
self-play rounds or updating neural network weights.

## Quickstart

* [Get Docker](https://docs.docker.com/get-docker/)
* Pull the OAZ cpu-only image:
  ```$ docker pull ghcr.io/ameroueh/oaz-cpu:latest```
* Run the image with 


## Installation

Clone the repository with

```bash
 $ git clone git@github.com:ameroueh/oaz.git
```

Now initialize the submodules:

```bash 
$ cd oaz
$ git submodule init
$ git submodule update
```

### Build the Docker image

The easiest way of running OAZ is by building the Docker image.
This image installs all the required dependencies, and can be used 
to compile and run the OAZ C++ code, and create install the Python package.
Building the image may take a while (several hours) as tensorflow shared libraries
are built as part of the process. Note that the image creates a user `oaz` with default
UID 1000 (this can be overridden) for normal usage not requiring privileged access,
and creates a directory `/home/oaz/io` meant to contain a host directory (typically the oaz repository).
This is the working directory of the container. Run

```bash
 $ cd oaz
 $ docker build . -t oaz
```

to build the image. The rest of this README assumes that the user
has access to a bash shell on the container, with the OAZ repository
mounted to `/home/oaz/io`. This can be achieved with:

### Python package

Although OAZ can be used as a C++ library, a Python
package with Python bindings to the C++ code as well as code 
facilitating running self-play and training is 
provided. To build and install it, run:

```bash
$ python setup.py bdist_wheel && cd dist && pip install .
```

### Implement your own games

```bash
$ cd oaz
$ docker run --rm --gpus all -it --mount source=$(pwd),destination=/home/oaz/io,type=bind oaz'
```

## Tests

### C++ test suite

To compile and run the C++ test suite, run

```
$ cmake . -B build && cd build && make -j$(nproc) && cd test && ctest
```

to build and run all the tests.

### Python test suite

Python test suite 
+++++++++++++++++

Run

.. code-block:: bash
``` 
$ cd pyoaz_tests && pytest .
```

to run the Python tests.
