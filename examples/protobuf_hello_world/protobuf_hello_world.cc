// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cstdio>
#include <pb_encode.h>
#include <pb_decode.h>

#include "libs/base/console_m7.h"

#include "simple.pb.h"

static void hello_proto() {
  /* This is the buffer where we will store our message. */
  uint8_t buffer[128];
  size_t message_length;
  bool status;

  printf("Hello world proto.\r\n");

  /* Encode our message */
  {
    /* Allocate space on the stack to store the message data.
     *
     * Nanopb generates simple struct definitions for all the messages.
     * - check out the contents of simple.pb.h!
     * It is a good idea to always initialize your structures
     * so that you do not have garbage data from RAM in there.
     */
    SimpleMessage message = SimpleMessage_init_zero;

    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    /* Fill in the lucky number */
    message.lucky_number = 13;
    message.unlucky.number = 42;

    /* Now we are ready to encode the message! */
    status = pb_encode(&stream, SimpleMessage_fields, &message);
    message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status) {
        printf("Encoding failed: %s\r\n", PB_GET_ERROR(&stream));
        return;
    }
  }

  /* Now we could transmit the message over network, store it in a file or
   * wrap it to a pigeon's leg.
   */

  /* But because we are lazy, we will just decode it immediately. */
  {
    /* Allocate space for the decoded message. */
    SimpleMessage message = SimpleMessage_init_zero;

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, SimpleMessage_fields, &message);

    /* Check for errors... */
    if (!status) {
        printf("Decoding failed: %s\r\n", PB_GET_ERROR(&stream));
        return;
    }

    /* Print the data contained in the message. */
    printf("Your lucky number was %ld!\r\n", message.lucky_number);
    printf("Your unlucky number was %lu!\r\n", message.unlucky.number);
    printf("\r\n");
  }
}

extern "C" void app_main(void *param) {
  while (true) {
    hello_proto();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
