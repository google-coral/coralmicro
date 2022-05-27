#!/bin/bash
KEY="$1"
TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

echo "${KEY}" > ${TMPDIR}/key.hex
xxd -r -p ${TMPDIR}/key.hex ${TMPDIR}/key.bin
openssl ecparam -name prime256v1 -genkey -noout -out ${TMPDIR}/tempkey.pem
openssl ec -in ${TMPDIR}/tempkey.pem -pubout -outform der -out ${TMPDIR}/temppub.der >/dev/null 2>&1
head -c 26 ${TMPDIR}/temppub.der >${TMPDIR}/public-header.der
cat ${TMPDIR}/public-header.der ${TMPDIR}/key.bin > ${TMPDIR}/publickey.der
openssl ec -in ${TMPDIR}/publickey.der -pubin -inform der -out ${TMPDIR}/publickey.pem -pubout >/dev/null 2>&1
cat ${TMPDIR}/publickey.pem
