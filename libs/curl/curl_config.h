/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_CURL_CURL_CONFIG_H_
#define LIBS_CURL_CURL_CONFIG_H_

#define HAVE_ARPA_INET_H 1
#define HAVE_NETDB_H 1
#define HAVE_FCNTL_O_NONBLOCK 1
#define HAVE_SYS_SELECT_H 1
#define HAVE_STDBOOL_H 1
#define HAVE_BOOL_T 1
#define HAVE_STRUCT_TIMEVAL 1
#define HAVE_GETTIMEOFDAY 1
#define SIZEOF_CURL_OFF_T 4

#define HAVE_RECV 1
#define RECV_TYPE_ARG1 int
#define RECV_TYPE_ARG2 void*
#define RECV_TYPE_ARG3 size_t
#define RECV_TYPE_ARG4 int
#define RECV_TYPE_RETV ssize_t

#define HAVE_SEND 1
#define SEND_TYPE_ARG1 int
#define SEND_QUAL_ARG2 const
#define SEND_TYPE_ARG2 void*
#define SEND_TYPE_ARG3 size_t
#define SEND_TYPE_ARG4 int
#define SEND_TYPE_RETV ssize_t

#define HAVE_SOCKET 1
#define CURL_DISABLE_SOCKETPAIR 1

#define HAVE_INET_PTON 1
#define HAVE_INET_NTOP 1

#define HAVE_SELECT 1

#define CURL_DISABLE_RTSP 1
#define CURL_DISABLE_MQTT 1
#define CURL_DISABLE_SMB 1
#define CURL_DISABLE_POP3 1
#define CURL_DISABLE_DICT 1
#define CURL_DISABLE_GOPHER 1
#define CURL_DISABLE_COOKIES 1
#define CURL_DISABLE_CRYPTO_AUTH 1
#define CURL_DISABLE_DOH 1
#define CURL_DISABLE_FILE 1
#define CURL_DISABLE_FTP 1
#define CURL_DISABLE_IMAP 1
#define CURL_DISABLE_LDAP 1
#define CURL_DISABLE_LDAPS 1
#define CURL_DISABLE_MIME 1
#define CURL_DISABLE_NETRC 1
#define CURL_DISABLE_NTLM 1
#define CURL_DISABLE_PROXY 1
#define CURL_DISABLE_SMTP 1
#define CURL_DISABLE_TELNET 1
#define CURL_DISABLE_TFTP 1

#define USE_MBEDTLS 1
#define USE_LWIPSOCK 1

#define CURL_CA_BUNDLE "/ca-certificates.crt"

#endif  // LIBS_CURL_CURL_CONFIG_H_
