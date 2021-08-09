#include "libs/base/httpd.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/fs.h"
#include "third_party/nxp/rt1176-sdk/middleware/lwip/src/include/lwip/apps/httpd.h"
#include <map>

namespace valiant {
namespace httpd {

static std::map<std::string, HTTPDHandlers*> handlers_map;
static std::map<void*, HTTPDHandlers*> connections_map;

void Init() {
    static bool initialized = false;
    if (!initialized) {
        LOCK_TCPIP_CORE();
        httpd_init();
        UNLOCK_TCPIP_CORE();
        initialized = true;
    }
}

bool RegisterHandlerForPath(const std::string& name, HTTPDHandlers* handlers) {
    if (handlers_map.find(name) != handlers_map.end()) {
        return false;
    }
    handlers_map.insert({name, handlers});
    return true;
}

extern "C" {
err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    auto it = connections_map.find(connection);
    if (it != connections_map.end()) {
        HTTPDHandlers *handlers = it->second;
        return handlers->post_receive_data(handlers->context, connection, p);
    }
    return ERR_ARG;
}

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
    auto it = handlers_map.find(uri);
    if (it != handlers_map.end()) {
        HTTPDHandlers *handlers = it->second;
        connections_map.insert({connection, handlers});
        return handlers->post_begin(handlers->context, connection, uri, http_request, http_request_len, content_len, response_uri, response_uri_len, post_auto_wnd);
    }
    return ERR_ARG;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    auto it = connections_map.find(connection);
    if (it != connections_map.end()) {
        HTTPDHandlers *handlers = it->second;
        return handlers->post_finished(handlers->context, connection, response_uri, response_uri_len);
    }
}

void httpd_cgi_handler(struct fs_file *file, const char *uri, int iNumParams, char **pcParam, char **pcValue) {
    void *connection = 0;
    for (int i = 0; i < iNumParams; ++i) {
        if (strcmp("connection", pcParam[i]) == 0) {
            connection = reinterpret_cast<void*>(strtoul(pcValue[i], nullptr, 16));
        }
    }

    if (!connection) {
        return;
    }
    file->pextension = connection;

    auto it = connections_map.find(connection);
    if (it != connections_map.end()) {
        HTTPDHandlers *handlers = it->second;
        handlers->cgi_handler(handlers->context, file, uri, iNumParams, pcParam, pcValue);
    }
}

int fs_read_custom(struct fs_file *file, char *buffer, int count) {
    void *connection = reinterpret_cast<void*>(file->pextension);
    auto it = connections_map.find(connection);
    if (it != connections_map.end()) {
        HTTPDHandlers *handlers = it->second;
        return handlers->fs_read_custom(handlers->context, file, buffer, count);
    }
    return -1;
}

int fs_open_custom(struct fs_file *file, const char *name) {
    for (auto it : handlers_map) {
        if (memcmp(name, it.first.c_str(), it.first.length()) == 0) {
            HTTPDHandlers *handlers = it.second;
            return handlers->fs_open_custom(handlers->context, file, name);
        }
    }
    return 0;
}

void fs_close_custom(struct fs_file *file) {
    void *connection = reinterpret_cast<void*>(file->pextension);
    auto it = connections_map.find(connection);
    if (it != connections_map.end()) {
        HTTPDHandlers *handlers = it->second;
        handlers->fs_close_custom(handlers->context, file);
    }
}

}  // extern "C"

}  // namespace httpd
}  // namespace valiant
