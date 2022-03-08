FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    cmake \
    git-core \
    ninja-build \
    protobuf-compiler \
    python3 \
    python3-pip \
    python3-protobuf
RUN python3 -m pip install "pyinstaller>=4.6" "semver>=2.13.0,<3" "GitPython>=3.1.24" "py7zr>=0.18.1"
