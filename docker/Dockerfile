FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    cmake \
    protobuf-compiler \
    python3 \
    python3-pip \
    python3-protobuf
RUN python3 -m pip install pyinstaller==4.3
