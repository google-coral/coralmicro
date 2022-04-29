#ifndef __LIBS_CONSOLE_CONSOLE_M4_H__
#define __LIBS_CONSOLE_CONSOLE_M4_H__

#include "libs/base/message_buffer.h"

namespace coral::micro {

void ConsoleInit();
void SetM4ConsoleBufferPtr(ipc::StreamBuffer* buffer);

}  // namespace coral::micro

#endif  // __LIBS_CONSOLE_CONSOLE_M4_H__
