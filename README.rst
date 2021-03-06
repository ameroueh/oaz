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
to compile and run the OAZ C++ code, and create install the Python package `pyoaz`.
Building the image may take a while (several hours) as tensorflow shared libraries
are built as part of the process. Note that the image creates a user `oaz` with default UID 1000 (this can be overridden) for normal usage not requiring privileged access, and creates a directory `/home/oaz/io` meant to contain a host directory (typically the oaz repository). This image is meant to help developing OAZ.


Run

.. code-block:: bash

 $ cd oaz
 $ docker build . -t oaz

to build the image.

C++ test suite
++++++++++++++

To test the alpha zero training loop for Tic Tac Toe,
create a container running bash with

.. code-block:: bash

 $ cd oaz
 $ docker run --rm --gpus all -it --mount source=$(pwd),destination=/home/oaz/io,type=bind oaz'

Then, in the container, run

.. code-block:: bash

 $ cmake . -B build && cd build && make -j$(nproc) 

to build all the tests and shared libraries, then run

.. code-block:: bash

 $ ctest .

to run the tests. 

Installing the Python package
+++++++++++++++++++++++++++++

Although OAZ can be used as a C++ library, a Python
package with Python bindings to the C++ code as well
modules to facilitate running self-play and training is 
provided. To build and install it, run:

.. code-block:: bash

 $ python setup.py bdist_wheel && cd dist && pip install .


Python test suite 
+++++++++++++++++

Run

.. code-block:: bash
 
 $ cd pyoaz_tests && pytest .

to run the Python tests.
