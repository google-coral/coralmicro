FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    cmake \
    git-core \
    libusb-1.0-0-dev \
    libudev-dev \
    libhidapi-hidraw0 \
    ninja-build \
    protobuf-compiler \
    python3-dev \
    python3-pip \
    python3-protobuf
RUN python3 -m pip install --upgrade setuptools
RUN python3 -m pip install "pyinstaller>=4.6" "semver>=2.13.0,<3" "GitPython>=3.1.24" "py7zr>=0.18.1" "progress>=1.5" "hexformat>=0.2" "hidapi>=0.10.1" "pyserial>=3.5" "pyusb>=1.2.0"
RUN python3 -m pip freeze
