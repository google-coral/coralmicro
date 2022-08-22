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

function main {
    local usage=$(cat <<EOF
Usage: docker_build.sh [-b <build_dir>]
  -b <build_dir>   directory in which to generate artifacts (defaults to ${SCRIPT_DIR}/build)
  -a               build arduino library archive
  -s               build all arduino sketches
  -n               build using ninja
EOF
)
    local build_dir="${SCRIPT_DIR}/build"
    local build_arduino
    local build_sketches
    local args=$(getopt hmansb: $*)
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

    cmake -B ${build_dir} -G "${generator}" -S "${SCRIPT_DIR}"

    if [[ ! -z "${ninja}" ]]; then
        ninja -C ${build_dir}
    else
        make -C ${build_dir} -j $(nproc)
    fi

    if [[ ! -z ${build_arduino} ]]; then
        python3 -m pip install -r ${SCRIPT_DIR}/arduino/requirements.txt
        python3 ${PACKAGE_PY} --output_dir=${build_dir} --core
        python3 ${PACKAGE_PY} --output_dir=${build_dir} --flashtool

        cat <<EOF >${SCRIPT_DIR}/arduino-cli.yaml
board_manager:
  additional_urls:
    - file://${build_dir}/package_coral_index.json
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
        elif [[ "$OSTYPE" == "win32" ]]; then
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
          --core_sha256=$(sha256sum ${build_dir}/${core_archive_name} | cut -d ' ' -f 1) \
          --${flashtool_name}_flashtool_url=http://localhost:8000/${flashtool_archive_name} \
          --${flashtool_name}_flashtool_sha256=$(sha256sum ${build_dir}/${flashtool_archive_name} | cut -d ' ' -f 1)

        if [[ ! -d "${SCRIPT_DIR}/third_party/arduino-cli" ]]; then
            python3 "${SCRIPT_DIR}/arduino/get_arduino_cli.py" \
              --version 0.26.0 \
              --output_dir "${SCRIPT_DIR}/third_party/arduino-cli"
        fi

        readonly arduino_cli="${SCRIPT_DIR}/third_party/arduino-cli/arduino-cli"

        # Remove old Coral packages.
        rm -rf "${SCRIPT_DIR}/.arduino15/packages/coral"

        # Install updated Coral packages.
        "${arduino_cli}" core update-index
        "${arduino_cli}" core install coral:coral_micro

        if [[ ! -z ${build_sketches} ]]; then
            for sketch in ${SCRIPT_DIR}/sketches/*; do
                for variant in coral_micro coral_micro_wifi coral_micro_poe; do
                    if [[ ${sketch} =~ WiFi ]]; then
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
        fi
    fi
}

main "$@"
