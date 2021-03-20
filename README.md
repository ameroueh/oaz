# OAZ: An implementation of Alpha Zero

## What is OAZ?

OAZ is a C++ library designed to carry out experiments with the Alpha Zero algorithm.
It provides a Python front-end, PyOAZ, to run high-level tasks such as orchestrating 
self-play rounds or updating neural network weights.

## Quickstart

* [Get Docker](https://docs.docker.com/get-docker/)
* Pull the OAZ cpu-only Docker image:

        $ docker pull ghcr.io/ameroueh/oaz-cpu:latest

* Run the image, mapping the port to get access to Jupyter from the host:

        $ docker run -it -p 8888:8888 oaz-cpu bash

* From the container, start Jupyter:

        $ jupyter notebook --ip 0.0.0.0 --no-browser

* Open `http://localhost:8888` from your favourite browser on the host,
and open the notebook `notebooks/quickstart.ipynb`

## Installation

Clone the repository with

```bash
 $ git clone git@github.com:ameroueh/oaz.git
```

Now initialize the submodules:

```bash 
$ cd oaz && git submodule update --init
```

### Docker image

The easiest way of running OAZ is by building the Docker image.
This image installs all the required dependencies, and can be used 
to compile and run the OAZ C++ code, and create install the Python package.
Building the image may take a while (several hours) as tensorflow shared libraries
are built as part of the process.

```bash
 $ cd oaz
 $ docker build . -t oaz
```

to build the image.

#### GPU Support

By default, no GPU support is enabled. To enable GPU support for Nvidia cards, provide appropriate
parameters to the following build arguments: `BASE_IMAGE`, `TF_NEED_CUDA`, `TF_CUDA_COMPUTE_CAPABILITIES`.
For example, to build the image with support for RTX 30xx GPUs, run

```bash
 $ cd oaz
 $ docker build --build-arg BASE_IMAGE=nvidia/cuda:11.1-cudnn8-devel-ubuntu18.04 --build-arg TF_NEED_CUDA=1 --build-arg TF_CUDA_COMPUTE_CAPABILITIES=8.6 . -t oaz
```

This image is tested to work on an Ubuntu 20.10 host with `nvidia-docker2` installed. Use the option `--gpus=all` (or select specific
GPU devices) to run the image with GPU support.

#### Note

The image creates a user `oaz` with default UID 1000 (this can be overridden) for normal usage not requiring privileged access. You may want to change this default value should you wish to mount a host directory (for example, the OAZ git repository to carry out development work) to the container.


## Tests

### C++ test suite

To compile and run the C++ test suite, run

```
$ cmake . -B build && cd build && make -j$(nproc) && cd test && ctest
```

from `/home/oaz/oaz` in the Docker container.

### Python test suite

To run the Python tests, run

``` 
$ cd pyoaz_tests && pytest .
```

from `/home/oaz/oaz` in the Docker container.
