FROM debian:buster

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y \
    cmake \
    python3 \
    python3-pip
RUN python3 -m pip install pyinstaller==4.3
