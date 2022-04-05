#include "libs/base/httpd.h"

#include <cstring>

#include "libs/base/filesystem.h"

namespace valiant {
namespace {
HttpServer* g_server = nullptr;

constexpr intptr_t kTagVector = 0b01;
constexpr intptr_t kTagFileHolder = 0b10;
constexpr intptr_t kTagMask = 0b11;

template <intptr_t Tag, typename T>
void* TaggedPointer(T* p) {
    assert((reinterpret_cast<intptr_t>(p) & kTagMask) == 0);
    return reinterpret_cast<void*>(reinterpret_cast<intptr_t>(p) | Tag);
}

intptr_t Tag(void* p) { return reinterpret_cast<intptr_t>(p) & kTagMask; }

template <typename T>
T* Pointer(void* p) {
    return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(p) & ~kTagMask);
}

struct FileHolder {
    lfs_file_t file;
    bool opened = false;

    ~FileHolder() {
        if (opened) filesystem::Close(&file);
    }
};
}  // namespace

void UseHttpServer(HttpServer* server) {
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

    for (auto& uri_handler : uri_handlers_) {
        auto content = uri_handler(name);

        if (auto* filename = std::get_if<std::string>(&content)) {
            auto file_holder = std::make_unique<FileHolder>();
            if (filesystem::Open(&file_holder->file, filename->c_str())) {
                file_holder->opened = true;
                file->data = nullptr;
                file->len = filesystem::Size(&file_holder->file);
                file->index = 0;
                file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
                file->pextension =
                    TaggedPointer<kTagFileHolder>(file_holder.release());
                return 1;
            }
        }

        if (auto* static_buffer = std::get_if<StaticBuffer>(&content)) {
            file->data = reinterpret_cast<const char*>(static_buffer->buffer);
            file->len = static_buffer->size;
            file->index = file->len;
            file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
            file->pextension = nullptr;
            return 1;
        }

        if (auto* v = std::get_if<std::vector<uint8_t>>(&content)) {
            file->data = reinterpret_cast<char*>(v->data());
            file->len = v->size();
            file->index = 0;
            file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
            file->pextension = TaggedPointer<kTagVector>(
                new std::vector<uint8_t>(std::move(*v)));
            return 1;
        }
    }

    return 0;
}

int HttpServer::FsReadCustom(struct fs_file* file, char* buffer, int count) {
    auto tag = Tag(file->pextension);

    if (tag == kTagFileHolder) {
        auto* file_holder = Pointer<FileHolder>(file->pextension);
        auto len = filesystem::Read(&file_holder->file, buffer, count);
        file->index += len;
        return len;
    }

    if (tag == kTagVector) {
        auto* v = Pointer<std::vector<uint8_t>>(file->pextension);
        std::memcpy(buffer, v->data() + file->index, count);
        file->index += count;
        return count;
    }

    return FS_READ_EOF;
};

void HttpServer::FsCloseCustom(struct fs_file* file) {
    auto tag = Tag(file->pextension);
    if (tag == kTagFileHolder) {
        delete Pointer<FileHolder>(file->pextension);
    } else if (tag == kTagVector) {
        delete Pointer<std::vector<uint8_t>>(file->pextension);
    }
}

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
}  // namespace valiant
