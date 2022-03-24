#include "libs/base/httpd.h"

#include <cstring>

namespace valiant {
namespace httpd {
namespace {
HttpServer* g_server = nullptr;
}  // namespace

void Init(HttpServer* server) {
    static bool initialized = false;
    if (!initialized) {
        LOCK_TCPIP_CORE();
        httpd_init();
        UNLOCK_TCPIP_CORE();
        initialized = true;
    }
    g_server = server;
}

int HttpServer::FsOpenCustom(struct fs_file* file, const char* name) {
    std::memset(file, 0, sizeof(*file));

    if (dynamic_file_handler_) {
        std::vector<uint8_t> buffer;
        if (dynamic_file_handler_(name, &buffer)) {
            auto* v = new std::vector(std::move(buffer));
            file->pextension = v;
            file->data = reinterpret_cast<char*>(v->data());
            file->len = v->size();
            file->index = file->len;
            file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
            return 1;
        }
    }

    if (static_file_handler_) {
        const uint8_t* buffer;
        size_t size;
        if (static_file_handler_(name, &buffer, &size)) {
            file->data = reinterpret_cast<const char*>(buffer);
            file->len = size;
            file->index = file->len;
            file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
            return 1;
        }
    }

    return 0;
}
int HttpServer::FsReadCustom(struct fs_file* file, char* buffer, int count) {
    return FS_READ_EOF;
};

void HttpServer::FsCloseCustom(struct fs_file* file) {
    if (file->pextension) {
        delete reinterpret_cast<std::vector<uint8_t>*>(file->pextension);
    }
};

extern "C" {
err_t httpd_post_begin(void* connection, const char* uri,
                       const char* http_request, u16_t http_request_len,
                       int content_len, char* response_uri,
                       u16_t response_uri_len, u8_t* post_auto_wnd) {
    return g_server->PostBegin(connection, uri, http_request, http_request_len,
                               content_len, response_uri, response_uri_len,
                               post_auto_wnd);
}

err_t httpd_post_receive_data(void* connection, struct pbuf* p) {
    return g_server->PostReceiveData(connection, p);
}

void httpd_post_finished(void* connection, char* response_uri,
                         u16_t response_uri_len) {
    g_server->PostFinished(connection, response_uri, response_uri_len);
}

void httpd_cgi_handler(struct fs_file* file, const char* uri, int iNumParams,
                       char** pcParam, char** pcValue) {
    g_server->CgiHandler(file, uri, iNumParams, pcParam, pcValue);
}

int fs_open_custom(struct fs_file* file, const char* name) {
    return g_server->FsOpenCustom(file, name);
}

int fs_read_custom(struct fs_file* file, char* buffer, int count) {
    return g_server->FsReadCustom(file, buffer, count);
}

void fs_close_custom(struct fs_file* file) { g_server->FsCloseCustom(file); }
}  // extern "C"
}  // namespace httpd
}  // namespace valiant
