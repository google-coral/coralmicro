#!/bin/bash
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly ROOTDIR="${SCRIPT_DIR}"

function die {
    echo "$@" >/dev/stderr
    exit 1
}

function main {
    local usage=$(cat <<EOF
Usage: docker_build.sh [-b <build_dir>]
  -b <build_dir>   directory in which to generate artifacts (defaults to ${ROOTDIR}/build)
  -m               build for mfgtest
  -a               build arduino library archive
EOF
)
    local build_dir
    local build_arduino
    local mfgtest
    local args=$(getopt hmab: $*)
    set -- $args

    for i; do
        case "$1" in
            -b) # build_dir
                build_dir="$2"
                shift 2
                ;;
            -m)
                mfgtest=true
                shift
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

    local mfgtest_cmake_flags
    if [[ ! -z "${mfgtest}" ]]; then
        mfgtest_cmake_flags="-DDEBUG_CONSOLE_TRANSFER_BLOCKING=1"
    fi

    if [[ -z "${build_dir}" ]]; then
        if [[ -z "${mfgtest}" ]]; then
            build_dir="${ROOTDIR}/build"
        else
            build_dir="${ROOTDIR}/build-mfgtest"
        fi
    fi


    mkdir -p ${build_dir}
    cmake -B ${build_dir} ${mfgtest_cmake_flags}
    make -C ${build_dir} -j $(nproc)

    if [[ ! -z ${build_arduino} ]]; then
        python3 ${ROOTDIR}/arduino/package.py --output_dir=${build_dir}
    fi
}

main "$@"
