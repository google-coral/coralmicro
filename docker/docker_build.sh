#!/bin/bash
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
                build_dir="$2"
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

    docker build -t valiant ${ROOTDIR}/docker
    rm -rf ${build_dir}
    mkdir -p ${build_dir}
    if [[ -t 1 ]]; then
        INTERACTIVE="-it"
    else
        INTERACTIVE=""
    fi
    docker run --rm ${INTERACTIVE} -w ${ROOTDIR} -v ${ROOTDIR}:${ROOTDIR} -v ${build_dir}:${ROOTDIR}/build valiant bash -xc "
        chmod a+w /
        groupadd --gid $(id -g) $(id -g -n)
        useradd -m -e '' -s /bin/bash --gid $(id -g) --uid $(id -u) $(id -u -n)
        su $(id -u -n) -c 'bash ${ROOTDIR}/build.sh -b ${ROOTDIR}/build ${arduino_build_flags} ${sketch_build_flags} ${ninja_build_flags}'
    "
}

main "$@"
