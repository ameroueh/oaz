Open Alpha Zero
---------------

An implementation of Alpha Zero.

Installation
++++++++++++

Clone the repository with

.. code-block:: bash

 $ git clone git@github.com:ameroueh/oaz.git

Now initialize the submodules:

.. code-block:: bash
 
 $ cd oaz
 $ git submodule init
 $ git submodule update

Build the Docker image
++++++++++++++++++++++

The easiest way of running OAZ is by building the Docker image.
This image installs all the required dependencies, and can be used 
to compile and run the OAZ C++ code, and create install the Python package.
Building the image may take a while (several hours) as tensorflow shared libraries
are built as part of the process. Note that the image creates a user `oaz` with default
UID 1000 (this can be overridden) for normal usage not requiring privileged access,
and creates a directory `/home/oaz/io` meant to contain a host directory (typically the oaz repository).
This is the working directory of the container. Run

.. code-block:: bash

 $ cd oaz
 $ docker build . -t oaz

to build the image. The rest of this README assumes that the user
has access to a bash shell on the container, with the OAZ repository
mounted to `/home/oaz/io`. This can be achieved with:

.. code-block:: bash

 $ cd oaz
 $ docker run --rm --gpus all -it --mount source=$(pwd),destination=/home/oaz/io,type=bind oaz'

C++ test suite
++++++++++++++

To compile and run the C++ test suite, run

.. code-block:: bash

 $ cmake . -B build && cd build && make -j$(nproc) && cd test && ctest

to build and run all the tests.

Python package
++++++++++++++

Although OAZ can be used as a C++ library, a Python
package with Python bindings to the C++ code as well as code 
facilitating running self-play and training is 
provided. To build and install it, run:

.. code-block:: bash

 $ python setup.py bdist_wheel && cd dist && pip install .

Python test suite 
+++++++++++++++++

Run

.. code-block:: bash
 
 $ cd pyoaz_tests && pytest .

to run the Python tests.
