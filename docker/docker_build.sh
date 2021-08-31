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
  -m               build for mfgtest
EOF
)

    local build_dir
    local mfgtest
    local args=$(getopt hmb: $*)
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
            --)
                shift
                break
                ;;
            *)
                die "${usage}"
                ;;
        esac
    done

    local mfgtest_build_flags
    if [[ ! -z "${mfgtest}" ]]; then
        mfgtest_build_flags="-m"
    fi

    if [[ -z "${build_dir}" ]]; then
        if [[ -z "${mfgtest}" ]]; then
            build_dir="${ROOTDIR}/build"
        else
            build_dir="${ROOTDIR}/build-mfgtest"
        fi
    fi

    docker build -t valiant ${ROOTDIR}/docker
    mkdir -p ${build_dir}
    if [[ -t 1 ]]; then
        INTERACTIVE="-it"
    else
        INTERACTIVE=""
    fi
    docker run --rm ${INTERACTIVE} -w /valiant -v ${ROOTDIR}:/valiant -v ${build_dir}:/build valiant bash -xc "
        chmod a+w /
        groupadd --gid $(id -g) $(id -g -n)
        useradd -m -e '' -s /bin/bash --gid $(id -g) --uid $(id -u) $(id -u -n)
        su $(id -u -n) -c 'bash /valiant/build.sh -b /build ${mfgtest_build_flags}'
    "
}

main "$@"
