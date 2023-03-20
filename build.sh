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

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly PACKAGE_PY="${SCRIPT_DIR}/arduino/package.py"

function die {
    echo "$@" >/dev/stderr
    exit 1
}

if [[ "$(uname -s)" == "Darwin" ]]; then
    function nproc {
        sysctl -n hw.logicalcpu
    }
fi

# $1 = arduino_cli
function build_sketches {
    local arduino_cli="$1"
    for sketch in $(find . -type f -path "*/arduino/libraries/*" -name "*.ino"); do
        for variant in coral_micro coral_micro_wifi coral_micro_poe; do
            if [[ ${sketch} =~ WiFi ]]; then
                if [[ ! ${variant} =~ wifi ]]; then
                    continue
                fi
            fi
            if [[ ${sketch} =~ Ble ]]; then
                if [[ ! ${variant} =~ wifi ]]; then
                    continue
                fi
            fi
            if [[ ${sketch} =~ PoE ]]; then
              if [[ ! ${variant} =~ poe ]]; then
                continue
              fi
            fi
            if [[ ${sketch} =~ Ethernet ]]; then
              if [[ ! ${variant} =~ poe ]]; then
                continue
              fi
            fi
            "${arduino_cli}" compile -b coral:coral_micro:${variant} ${sketch};
        done
    done
}

# $1 = build_dir
# $2 = arduino_cli
function install_arduino {
    local build_dir="$1"
    local arduino_cli="$2"
    cat <<EOF >${SCRIPT_DIR}/arduino-cli.yaml
board_manager:
  additional_urls:
    - http://localhost:8000/package_coral_index.json
daemon:
  port: "50051"
directories:
  data: ${SCRIPT_DIR}/.arduino15
  downloads: ${SCRIPT_DIR}/.arduino15/staging
  user: ${SCRIPT_DIR}/.arduino15/user
library:
  enable_unsafe_install: true
logging:
  file: ""
  format: text
  level: info
metrics:
  addr: :9090
  enabled: true
sketch:
  always_export_binaries: false
EOF
    local platform_name
    local flashtool_name
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        platform_name="linux64"
        flashtool_name="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        platform_name="osx"
        flashtool_name="mac"
    elif [[ "$OSTYPE" == "win32" || "$OSTYPE" == "msys" ]]; then
        platform_name="windows"
        flashtool_name="win"
    else
        die "Unknown platform name : $OSTYPE"
    fi

    python3 -m http.server --directory ${build_dir} &
    http_pid="$!"
    trap "kill ${http_pid}" EXIT

    local core_archive_name=coral-micro-$(git rev-parse HEAD).tar.bz2
    local flashtool_archive_name=coral-flashtool-${platform_name}-$(git rev-parse HEAD).tar.bz2

    python3 "${PACKAGE_PY}" --output_dir=${build_dir} --manifest \
      --manifest_revision=9.9.9 \
      --core_url=http://localhost:8000/${core_archive_name} \
      --${flashtool_name}_flashtool_url=http://localhost:8000/${flashtool_archive_name}

    # Remove old Coral packages.
    rm -rf "${SCRIPT_DIR}/.arduino15/packages/coral"

    # Install updated Coral packages.
    "${arduino_cli}" core update-index
    "${arduino_cli}" core install coral:coral_micro
}

function main {
    local usage=$(cat <<EOF
Usage: docker_build.sh [-b <build_dir>]
  -b <build_dir>   directory in which to generate artifacts (defaults to ${SCRIPT_DIR}/build)
  -a               build arduino library archive
  -s               build all arduino sketches
  -c               skip building CMake world
  -o               skip building arduino core
  -n               build using ninja
EOF
)
    local build_dir="${SCRIPT_DIR}/build"
    local skip_build_cmake
    local skip_install_arduino
    local skip_arduino_flashtool
    local skip_arduino_core
    local build_arduino
    local build_sketches
    local args=$(getopt hmaifgnscb: $*)
    set -- $args

    for i; do
        case "$1" in
            -b) # build_dir
                mkdir -p "$2"
                build_dir="$(cd "$2" && pwd)"
                shift 2
                ;;
            -a)
                build_arduino=true
                shift
                ;;
            -s)
                build_sketches=true
                shift
                ;;
            -c)
                skip_build_cmake=true
                shift
                ;;
            -i)
                skip_install_arduino=true
                shift
                ;;
            -f)
                skip_arduino_flashtool=true
                shift
                ;;
            -g)
                skip_arduino_core=true
                shift
                ;;
            -n)
                ninja=true
                shift
                ;;
            --)
                shift
                break
                ;;
            *)
                die "${usage}"
                ;;
        esac
    done

    if [[ -z "${build_dir}" ]]; then
        if [[ ! -z "${ninja}" ]]; then
            build_dir="${SCRIPT_DIR}/build-ninja"
        else
            build_dir="${SCRIPT_DIR}/build"
        fi
    fi

    if [[ ! -z "${ninja}" ]]; then
        generator="Ninja"
    else
        generator="Unix Makefiles"
    fi

    set -xe

    # Even when "skipping" the CMake build, setup the environment.
    # This ensures that runtime-fetched artifacts like the toolchain
    # are available.
    cmake -B ${build_dir} -G "${generator}" -S "${SCRIPT_DIR}"
    if [[ -z ${skip_build_cmake} ]]; then
        if [[ ! -z "${ninja}" ]]; then
            ninja -C ${build_dir}
        else
            make -C ${build_dir} -j "$(nproc)"
        fi
    fi

    if [[ ! -z ${build_arduino} ]]; then
        python3 -m pip install -r ${SCRIPT_DIR}/scripts/requirements.txt
        python3 -m pip install -r ${SCRIPT_DIR}/arduino/requirements.txt
        if [[ -z ${skip_arduino_flashtool} ]]; then
            python3 ${PACKAGE_PY} --output_dir=${build_dir} --flashtool
        fi
        if [[ -z ${skip_arduino_core} ]]; then
            python3 ${PACKAGE_PY} --output_dir=${build_dir} --core
        fi
        if [[ -z ${skip_install_arduino} ]]; then
            if [[ ! -d "${SCRIPT_DIR}/third_party/arduino-cli" ]]; then
                python3 "${SCRIPT_DIR}/arduino/get_arduino_cli.py" \
                  --version 0.28.0 \
                  --output_dir "${SCRIPT_DIR}/third_party/arduino-cli"
            fi
            local readonly arduino_cli="${SCRIPT_DIR}/third_party/arduino-cli/arduino-cli"
            install_arduino "${build_dir}" "${arduino_cli}"
            if [[ ! -z ${build_sketches} ]]; then
                build_sketches "${arduino_cli}"
            fi
        fi
    fi
}

main "$@"
