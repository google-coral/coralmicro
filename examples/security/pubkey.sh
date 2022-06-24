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
