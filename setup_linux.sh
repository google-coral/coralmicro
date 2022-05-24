#!/bin/bash
# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script installs the packages required for a Linux Debian host
# to build apps for and flash the Coral Dev Board Micro

set -ex

sudo apt-get update
sudo apt-get -y install \
  protobuf-compiler \
  cmake \
  libhidapi-hidraw0 \
  libusb-dev \
  libudev-dev \
  python3-dev \
  python3-pip

python3 -m pip install pip --upgrade
python3 -m pip install -r scripts/requirements.txt

sudo cp scripts/50-cmsis-dap.rules \
  scripts/99-coral-micro.rules \
  scripts/99-secure-provisioning.rules /etc/udev/rules.d
sudo udevadm control --reload-rules
sudo udevadm trigger
