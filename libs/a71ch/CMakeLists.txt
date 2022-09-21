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

add_library_m7(libs_a71ch STATIC
    a71ch.cc
    entropy.c
    # i2c_imxrt.c is copied from SDK, and modified to use the correct I2C bus for this device.
    ${PROJECT_SOURCE_DIR}/third_party/modified/a71ch/i2c_imxrt.c
    # sci2c.c is copied verbatim from the SDK, so that local import paths pick up our config header.
    ${PROJECT_SOURCE_DIR}/third_party/modified/a71ch/sci2c.c
    # sm_timer.c is copied verbatim from the SDK, because the legacy Apache-licensed version doesn't
    # include the RTOS sleep options.
    ${PROJECT_SOURCE_DIR}/third_party/modified/a71ch/sm_timer.c

    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/a71ch/src/a71ch_module.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/api/src/ax_scp.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/api/src/ax_sss_scp.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/api/src/ax_util.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/hostCrypto/HostCryptoAPImbedtls.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/global_platf.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/nxLog.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/sm_apdu.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/sm_connect.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/sm_errors.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra/sm_printf.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/scp/scp_a7x.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/smCom/smCom.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/smCom/smComSCI2C.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/src/a71ch/fsl_sscp_a71ch.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/src/fsl_sss_apis.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/src/fsl_sss_util_asn1_der.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/src/mbedtls/fsl_sss_mbedtls_apis.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/src/sscp/fsl_sss_sscp.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/a71ch/src/a71ch_sst.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/a71ch/src/a71ch_crypto_ecc.c
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/hostCrypto/hcAsn.c
)

target_compile_definitions(libs_a71ch PUBLIC
    AX_EMBEDDED
    SSS_USE_FTR_FILE
    SSS_CONFIG_FILE="${PROJECT_SOURCE_DIR}/third_party/modified/a71ch/fsl_sss_ftr.h"
    SCI2C
    SCP_MODE=C_MAC_C_ENC_R_MAC_R_ENC
    SDK_OS_FREE_RTOS
    NO_SECURE_CHANNEL_SUPPORT
    TGT_A71CH
    MAX_DATA_LEN=270
)

target_include_directories(libs_a71ch PUBLIC
    .
    ${PROJECT_SOURCE_DIR}/third_party/modified/a71ch
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/inc
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/hostCrypto
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/infra
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/log
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/libCommon/smCom
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/hostlib/hostLib/platform/inc
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/ex/inc
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/inc
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/port/default
    ${PROJECT_SOURCE_DIR}/third_party/a71ch-crypto-support/sss/port/ksdk
)

target_link_libraries(libs_a71ch PUBLIC
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk-mbedtls
)
