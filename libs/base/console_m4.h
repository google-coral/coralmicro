#ifndef LIBS_BASE_CONSOLE_M4_H_
#define LIBS_BASE_CONSOLE_M4_H_

#include "libs/base/message_buffer.h"

namespace coral::micro {

void ConsoleInit();
void SetM4ConsoleBufferPtr(ipc::StreamBuffer* buffer);

}  // namespace coral::micro

#endif  // LIBS_BASE_CONSOLE_M4_H_
