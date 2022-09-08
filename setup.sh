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

set -e

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly OS="$(uname -s)"

function error {
  echo -e "\033[0;31m${1}\033[0m"  # red
}

if [[ "${OS}" == "Linux" ]]; then
  sudo apt-get update && sudo apt-get -y install \
    make \
    cmake \
    libhidapi-hidraw0 \
    libusb-1.0-0-dev \
    libudev-dev \
    python3-dev \
    python3-pip

  if [[ -x "$(command -v udevadm)" ]]; then
    sudo cp "${SCRIPT_DIR}/scripts/99-coral-micro.rules" /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    sudo udevadm trigger
  fi
elif [[ "${OS}" == "Darwin" ]]; then
  # Preinstalled on macOS: git, make
  if [[ -x "$(command -v brew)" ]]; then
    brew install \
      cmake \
      libusb \
      lsusb
  elif [[ -x "$(command -v port)" ]]; then
    sudo $(command -v port) install \
      cmake \
      libusb \
      usbutils
  else
    error "Please install MacPorts or Homebrew to get required dependencies."
    exit 1
  fi
else
  error "Your operating system is not supported."
  exit 1
fi

python3 -m pip install pip --upgrade
python3 -m pip install -r "${SCRIPT_DIR}/scripts/requirements.txt"
