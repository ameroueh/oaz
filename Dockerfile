ARG NVIDIA_CUDA_IMAGE=11.1-cudnn8-devel-ubuntu18.04
FROM nvidia/cuda:$NVIDIA_CUDA_IMAGE

SHELL [ "/bin/bash", "-c" ]

# Install toolchain
RUN apt-get -qq update
RUN apt-get install -qy unzip wget gcc g++ git

# Add user

ARG UID=1000
RUN useradd -m oaz --uid=${UID}

# Install miniconda

USER oaz
WORKDIR /home/oaz

ARG MINICONDA_VERSION=4.7.12.1
ENV CONDA_DIR /home/oaz/miniconda3

RUN wget --quiet https://repo.anaconda.com/miniconda/Miniconda3-$MINICONDA_VERSION-Linux-x86_64.sh -O miniconda.sh
RUN chmod +x miniconda.sh
RUN ./miniconda.sh -b -p $CONDA_DIR
RUN rm miniconda.sh

ENV PATH=$CONDA_DIR/bin:$PATH
RUN echo ". $CONDA_DIR/etc/profile.d/conda.sh" >> ~/.profile
RUN conda init bash

RUN conda create --name oaz python=3.6
RUN echo "conda activate oaz" >> ~/.bashrc

# Set-up entrypoint
RUN echo $'#!/bin/bash \n\
if [[ $EUID -ne 0 ]]; then \n\
	__conda_setup="$($CONDA_DIR/bin/conda shell.bash hook 2> /dev/null)" \n\
	eval "$__conda_setup" \n\
	conda activate oaz\n\
fi \n\
exec "$@"' >> /home/oaz/entrypoint.sh && chmod +x /home/oaz/entrypoint.sh
SHELL ["/home/oaz/entrypoint.sh", "/bin/bash", "-c"]
ENTRYPOINT ["/home/oaz/entrypoint.sh"]

# Install and build tensorflow

# Install tensorflow dependencies

RUN pip install six 'numpy<1.19.0' wheel setuptools mock 'futures>=0.17.1'
RUN pip install keras_preprocessing --no-deps

# Install Bazel

ARG BAZEL_VERSION=3.1.0
RUN wget https://github.com/bazelbuild/bazel/releases/download/$BAZEL_VERSION/bazel-$BAZEL_VERSION-installer-linux-x86_64.sh && \
 	chmod +x bazel-$BAZEL_VERSION-installer-linux-x86_64.sh && \
	./bazel-$BAZEL_VERSION-installer-linux-x86_64.sh --user && \
	rm bazel-$BAZEL_VERSION-installer-linux-x86_64.sh
ENV PATH=/home/oaz/bin:$PATH

# Get tensorflow

RUN git clone https://github.com/tensorflow/tensorflow.git --depth 1 -b r2.4

# Configure

WORKDIR /home/oaz/tensorflow

ARG TF_SET_ANDROID_WORKSPACE=0
ARG TF_NEED_OPENCL_SYCL=0
ARG TF_NEED_ROCM-0
ARG TF_DOWNLOAD_CLANG=0
ARG TF_NEED_MPI-0 
ARG TF_NEED_CUDA=1
ARG TF_CUDA_COMPUTE_CAPABILITIES=8.6
ARG GCC_HOST_COMPILER_PATH="/usr/bin/gcc"
ARG CC_OPT_FLAGS="-march=native -mtune=native"
ARG USE_DEFAULT_PYTHON_LIB_PATH=1
ARG TF_NEED_JEMALLOC=1
ARG TF_NEED_GCP=0
ARG TF_NEED_HDFS=0
ARG TF_ENABLE_XLA=0
ARG TF_NEED_OPENCL=0
ARG TF_NEED_MKL=0

RUN ./configure

ARG BAZEL_LOCAL_RAM_RESOURCES=16384
ARG BAZEL_JOBS=16

RUN bazel build --local_ram_resources=$BAZEL_LOCAL_RAM_RESOURCES \
		--jobs=$BAZEL_JOBS \
		--config=opt \
		//tensorflow/tools/pip_package:build_pip_package
RUN ./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg && \
	pip install /tmp/tensorflow_pkg/tensorflow-*whl && \
	rm -rf /tmp/tensorflow
RUN bazel build --local_ram_resources=$BAZEL_LOCAL_RAM_RESOURCES \
		--jobs=$BAZEL_JOBS \
		--config=opt \
		//tensorflow:libtensorflow.so
RUN bazel build --local_ram_resources=$BAZEL_LOCAL_RAM_RESOURCES \
		--jobs=$BAZEL_JOBS \
		--config=opt \
		//tensorflow:libtensorflow_cc.so

ENV TENSORFLOW_DIR=/home/oaz/tensorflow

# Install cmake

USER root
ARG CMAKE_VERSION=3.19.1
RUN apt remove --purge --auto-remove cmake -qy
RUN apt-get update
RUN apt-get install libssl-dev -qy
RUN mkdir /tmp/cmake && \
	cd /tmp/cmake && \
	wget https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.tar.gz && \
	tar -xzvf cmake-$CMAKE_VERSION.tar.gz && \
	cd /tmp/cmake/cmake-$CMAKE_VERSION && \
	./bootstrap && \
	make -j$(nproc) && \
 	make install && \
	rm -rf /tmp/cmake
USER oaz

# Install boost

RUN conda install py-boost boost-cpp

# Install swig

RUN conda install swig

# Install OAZ python requirements

COPY requirements.txt .
RUN pip install -r requirements.txt

# Set up working directory

RUN mkdir /home/oaz/io
WORKDIR /home/oaz/io