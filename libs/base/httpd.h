#ifndef _LIBS_BASE_HTTPD_H_
#define _LIBS_BASE_HTTPD_H_

#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"
#include <string>

namespace valiant {
namespace httpd {

struct HTTPDHandlers {
    err_t (*post_begin)(void*, void*, const char*, const char*, u16_t, int, char*, u16_t, u8_t*);
    void (*post_finished)(void*, void*, char*, u16_t);
    err_t (*post_receive_data)(void*, void*, struct pbuf*);
    void (*cgi_handler)(void*, struct fs_file*, const char*, int, char**, char**);
    int (*fs_read_custom)(void*, struct fs_file*, char*, int);
    int (*fs_open_custom)(void*, struct fs_file*, const char*);
    void (*fs_close_custom)(void*, struct fs_file*);
    void *context;
};

void Init();
bool RegisterHandlerForPath(const std::string& name, HTTPDHandlers* handlers);

}  // namespace httpd
}  // namespace valiant

#endif  // _LIBS_BASE_HTTPD_H_
