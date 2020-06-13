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

.. code-block:: bash

 $ cd oaz/extern/tensorflow
 $ git apply ../../../patches/tensorflow.patch

We recommend creating a Python environment for OAZ,
e.g. with Miniconda:

.. code-block:: bash

 $ conda create -n oaz python=3.6
 $ conda activate oaz

Download and install Bazel 0.26.1, for example
on Linux:

.. code-block:: bash

 $ wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-installer-linux-x86_64.sh 
 $ chmod +x bazel-0.26.1-installer-linux-x86_64.sh
 $ ./bazel-0.26.1-installer-linux-x86_64.sh --user

Build TensorFlow from source:

.. code-block:: bash
 
 $ cd oaz/extern/tensorflow
 $ ./configure
 $ bazel build //tensorflow/tools/pip_package:build_pip_package
 $ ./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg
 $ cd /tmp/tensorflow_pkg
 $ pip install tensorflow-version-tags.whl

You must also manually compile ``libtensorflow.so`` and ``libtensorflow_cc.so``:

.. code-block:: bash

 $ cd oaz/extern/tensorflow
 $ bazel build //tensorflow:libtensorflow.so
 $ bazel build //tensorflow:libtensorflow_cc.so

Build
+++++

Run

.. code-block:: bash

 $ cd oaz
 $ cmake . -B build

to build the project.

Tests
+++++

Run

.. code-block:: bash
 
 $ cd oaz/test
 $ make 

and then

.. code-block:: bash

 $ make test

to build and run all the tests.
