FROM ubuntu:19.04

ARG uid
ARG gid

COPY /etc/group /etc/group
COPY /etc/passwd /etc/passwd
COPY /etc/shadow /etc/shadow

USER $uid:$gid

WORKDIR /root

# Set shell to bash

SHELL ["/bin/bash", "--login", "-c"]

# Install toolchain

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -qq update
RUN apt-get install -qy g++ gcc git wget unzip

# Install miniconda

COPY environment.yml . 
ARG miniconda_version=4.7.12.1
ARG conda_dir=/opt/conda
ENV CONDA_DIR=$conda_dir

RUN wget --quiet https://repo.anaconda.com/miniconda/Miniconda3-$miniconda_version-Linux-x86_64.sh -O miniconda.sh && \
	chmod +x miniconda.sh && \
	./miniconda.sh -b -p $conda_dir && \
	rm miniconda.sh && \
	ln -s $conda_dir/etc/profile.d/conda.sh /etc/profile.d/conda.sh 

ENV PATH=$conda_dir/bin:$PATH

# Build conda environment

RUN conda env create -f environment.yml

# Make entrypoint which activates the environment

RUN echo $'#!/bin/bash \n\
__conda_setup="$(/opt/conda/bin/conda shell.bash hook 2> /dev/null)" \n\
eval "$__conda_setup" \n\
conda activate oaz \n\
echo "ENTRYPOINT: CONDA_DEFAULT_ENV=${CONDA_DEFAULT_ENV}" \n\
exec "$@"' >> /entrypoint.sh && chmod +x /entrypoint.sh
SHELL ["/entrypoint.sh", "/bin/bash", "--login", "-c"]

RUN echo "source activate oaz" >> .bashrc

# Install and build tensorflow

# Install tensorflow dependencies

RUN pip install six 'numpy<1.19.0' wheel setuptools mock 'future>=0.17.1'
RUN pip install keras_applications --no-deps
RUN pip install keras_preprocessing --no-deps 

# Install Bazel

RUN wget https://github.com/bazelbuild/bazel/releases/download/0.26.1/bazel-0.26.1-installer-linux-x86_64.sh && \
 	chmod +x bazel-0.26.1-installer-linux-x86_64.sh && \
	./bazel-0.26.1-installer-linux-x86_64.sh --user && \
	rm bazel-0.26.1-installer-linux-x86_64.sh
ENV PATH=/root/bin:$PATH

# Get tensorflow

RUN git clone https://github.com/tensorflow/tensorflow.git && \
	cd tensorflow && \
	git checkout r1.15
WORKDIR /root/tensorflow

# Patch tensorflow 1.15 (gettid name collision)

COPY patches/tensorflow.patch .
RUN git apply tensorflow.patch 

# Configure tensorflow build

ENV TF_SET_ANDROID_WORKSPACE=0
ENV TF_NEED_OPENCL_SYCL=0
ENV TF_NEED_ROCM=0
ENV TF_DOWNLOAD_CLANG=0
ENV TF_NEED_MPI=0 
ENV TF_NEED_CUDA=0
ENV GCC_HOST_COMPILER_PATH=/usr/bin/gcc
ENV CC_OPT_FLAGS="-march=native -mtune=native"
ENV PYTHON_BIN_PATH="$CONDA_DIR/envs/oaz/bin/python"
ENV USE_DEFAULT_PYTHON_LIB_PATH=1
ENV TF_NEED_JEMALLOC=1
ENV TF_NEED_GCP=0
ENV TF_NEED_HDFS=0
ENV TF_ENABLE_XLA=0
ENV TF_NEED_OPENCL=0

RUN ./configure

RUN bazel build --config=opt //tensorflow/tools/pip_package:build_pip_package
RUN ./bazel-bin/tensorflow/tools/pip_package/build_pip_package /tmp/tensorflow_pkg && \
	pip install /tmp/tensorflow_pkg/tensorflow-*whl && \
	rm -rf /tmp/tensorflow
RUN bazel build --config=opt //tensorflow:libtensorflow.so
RUN bazel build --config=opt //tensorflow:libtensorflow_cc.so

# Install OAZ specific dependencies

RUN apt-get install make cmake swig

WORKDIR /root
