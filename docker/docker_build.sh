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
readonly ROOTDIR="${SCRIPT_DIR}/../"

function die {
    echo "$@" >/dev/stderr
    exit 1
}

function main {
    local usage=$(cat <<EOF
Usage: docker_build.sh [-b <build_dir>]
  -b <build_dir>   directory in which to generate artifacts (defaults to ${ROOTDIR}/build)
  -a               build arduino library archive
  -s               build all arduino sketches
  -n               build using ninja
EOF
)
    local build_dir
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

    local arduino_build_flags
    if [[ ! -z "${build_arduino}" ]]; then
        arduino_build_flags="-a"
    fi

    local sketch_build_flags
    if [[ ! -z "${build_sketches}" ]]; then
        sketch_build_flags="-s"
    fi

    local ninja_build_flags
    if [[ ! -z "${ninja}" ]]; then
        ninja_build_flags="-n"
    fi

    if [[ -z "${build_dir}" ]]; then
        build_dir="${ROOTDIR}/build"
    fi

    docker build -t coral-micro ${ROOTDIR}/docker
    rm -rf ${build_dir}
    mkdir -p ${build_dir}
    if [[ -t 1 ]]; then
        INTERACTIVE="-it"
    else
        INTERACTIVE=""
    fi
    docker run --rm ${INTERACTIVE} -w ${ROOTDIR} -v ${ROOTDIR}:${ROOTDIR} -v ${build_dir}:${ROOTDIR}/build coral-micro bash -xc "
        chmod a+w /
        groupadd --gid $(id -g) $(id -g -n)
        useradd -m -e '' -s /bin/bash --gid $(id -g) --uid $(id -u) $(id -u -n)
        su $(id -u -n) -c 'bash ${ROOTDIR}/build.sh -b ${ROOTDIR}/build ${arduino_build_flags} ${sketch_build_flags} ${ninja_build_flags}'
    "
}

main "$@"
