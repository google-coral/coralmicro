#!/bin/bash
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function die {
    echo "$@" >/dev/stderr
    exit 1
}

ROOTDIR=''
if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ ! $(type greadlink) > /dev/null ]]; then
        die "Install greadlink: brew install coreutils"
    fi
    ROOTDIR=$(greadlink -f "${SCRIPT_DIR}")
else
    ROOTDIR=$(readlink -f "${SCRIPT_DIR}")
fi
readonly ROOTDIR

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

    if [[ -z "${build_dir}" ]]; then
        if [[ ! -z "${ninja}" ]]; then
            build_dir="${ROOTDIR}/build-ninja"
        else
            build_dir="${ROOTDIR}/build"
        fi
    fi

    if [[ ! -z "${ninja}" ]]; then
        generator="Ninja"
    else
        generator="Unix Makefiles"
    fi

    set -xe

    mkdir -p ${build_dir}
    cmake -B ${build_dir} -G "${generator}"

    if [[ ! -z "${ninja}" ]]; then
        ninja -C ${build_dir}
    else
        make -C ${build_dir} -j $(nproc)
    fi

    if [[ ! -z ${build_arduino} ]]; then
        python3 ${ROOTDIR}/arduino/package.py --output_dir=${build_dir} --core
        python3 ${ROOTDIR}/arduino/package.py --output_dir=${build_dir} --flashtool
        cat <<EOF >${ROOTDIR}/arduino-cli.yaml
board_manager:
  additional_urls:
    - file://${build_dir}/package_coral_index.json
daemon:
  port: "50051"
directories:
  data: ${ROOTDIR}/.arduino15
  downloads: ${ROOTDIR}/.arduino15/staging
  user: ${ROOTDIR}/.arduino15/user
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
    local platform_name=""
    local flashtool_name=""
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
        die "unkown platform name : $OSTYPE"
    fi

    python3 -m http.server --directory ${build_dir} &
    http_pid="$!"
    trap "kill ${http_pid}" EXIT
    python3 ${ROOTDIR}/arduino/package.py --output_dir=${build_dir} --manifest \
    --manifest_revision=9.9.9 \
    --core_url=http://localhost:8000/coral-micro-$(git rev-parse HEAD).tar.bz2 \
    --core_sha256=$(sha256sum ${build_dir}/coral-micro-$(git rev-parse HEAD).tar.bz2 | cut -d ' ' -f 1) \
    --${flashtool_name}_flashtool_url=http://localhost:8000/coral-flashtool-${platform_name}-$(git rev-parse HEAD).tar.bz2 \
    --${flashtool_name}_flashtool_sha256=$(sha256sum ${build_dir}/coral-flashtool-${platform_name}-$(git rev-parse HEAD).tar.bz2 | cut -d ' ' -f 1)
    ${SCRIPT_DIR}/third_party/arduino-cli/${flashtool_name}/arduino-cli core update-index
    ${SCRIPT_DIR}/third_party/arduino-cli/${flashtool_name}/arduino-cli core install coral:coral_micro
    if [[ ! -z ${build_sketches} ]]; then
        for sketch in ${ROOTDIR}/sketches/*; do
            ${ROOTDIR}/third_party/arduino-cli/${flashtool_name}/arduino-cli compile -b coral:coral_micro:coral_micro ${sketch};
        done
    fi
    fi
}

main "$@"
