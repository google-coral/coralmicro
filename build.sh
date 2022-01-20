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
EOF
)
    local build_dir
    local build_arduino
    local args=$(getopt hmab: $*)
    set -- $args

    for i; do
        case "$1" in
            -b) # build_dir
                build_dir="$2"
                shift 2
                ;;
            -a)
                build_arduino=true
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
        build_dir="${ROOTDIR}/build"
    fi

    set -xe

    mkdir -p ${build_dir}
    cmake -B ${build_dir}
    make -C ${build_dir} -j $(nproc)

    if [[ ! -z ${build_arduino} ]]; then
        python3 ${ROOTDIR}/arduino/package.py --output_dir=${build_dir}
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
    python3 -m http.server --directory ${build_dir} &
    http_pid="$!"
    ${SCRIPT_DIR}/third_party/arduino-cli/arduino-cli core update-index
    ${SCRIPT_DIR}/third_party/arduino-cli/arduino-cli core install coral:valiant
    ${ROOTDIR}/third_party/arduino-cli/arduino-cli compile -b coral:valiant:valiant ${ROOTDIR}/sketches/HelloWorld -v
    kill ${http_pid}
    wait
    fi
}

main "$@"
