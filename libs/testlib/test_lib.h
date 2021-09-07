#ifndef _LIBS_TESTLIB_TEST_LIB_H_
#define _LIBS_TESTLIB_TEST_LIB_H_

#include "third_party/mjson/src/mjson.h" 

namespace valiant {
namespace testlib {

void GetSerialNumber(struct jsonrpc_request *request);

}  // namespace testlib
}  // namespace valiant

#endif  // _LIBS_TESTLIB_TEST_LIB_H_
