#include <cstdio>
#include <cstring>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/base/httpd.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

using valiant::filesystem::Lfs;

namespace {
constexpr char kPrefix[] = "/fs/";

template <size_t N>
constexpr size_t StrLen(const char (&s)[N]) {
    return N - 1;
};

template <size_t N>
bool StartsWith(const char* s, const char (&begin)[N]) {
    return std::strncmp(s, begin, StrLen(begin)) == 0;
}

template <size_t N>
bool EndsWith(const char* s, const char (&end)[N]) {
    const auto len = std::strlen(s);
    if (len < StrLen(end)) return false;
    return std::strcmp(s + len - StrLen(end), end) == 0;
}

template <typename... T>
void Append(std::vector<uint8_t>* v, const char* format, T... args) {
    const int size = std::snprintf(nullptr, 0, format, args...) + 1;
    v->resize(v->size() + size);
    auto* s = reinterpret_cast<char*>(v->data() + v->size() - size);
    std::snprintf(s, size, format, args...);
    v->pop_back();  // remove null terminator
}

std::string GetPath(const char* uri) {
    static constexpr char kIndexShtml[] = "/index.shtml";
    auto len = std::strlen(uri);
    if (EndsWith(uri, kIndexShtml))
        return std::string(uri, uri + len - StrLen(kIndexShtml) + 1);
    return std::string(uri, uri + len);
}

bool DirHtml(lfs_dir_t* dir, const char* dirname, std::vector<uint8_t>* html) {
    Append(html, "<!DOCTYPE html>\r\n");
    Append(html, "<html lang=\"en\">\r\n");
    Append(html, "<head>\r\n");
    Append(html,
           "<meta http-equiv=\"Content-Type\" content=\"text/html; "
           "charset=utf-8\">\r\n");
    Append(html, "<title>Directory listing for %s</title>\r\n", dirname);
    Append(html, "</head>\r\n");
    Append(html, "<body>\r\n");
    Append(html, "<h1>Directory listing for %s</h1>\r\n", dirname);
    Append(html, "<hr>\r\n");
    Append(html, "<ul>\r\n");
    int res;
    lfs_info info;
    while ((res = lfs_dir_read(Lfs(), dir, &info)) != 0) {
        if (res < 0) return false;

        if (info.type == LFS_TYPE_REG) {
            Append(html, "<li><a href=\"%s%s%s\">%s</a></li>\r\n", kPrefix,
                   dirname + 1, info.name, info.name);
        } else if (info.type == LFS_TYPE_DIR) {
            Append(html, "<li><a href=\"%s%s%s/\">%s/</a></li>\r\n", kPrefix,
                   dirname + 1, info.name, info.name);
        }
    }
    Append(html, "</ul>\r\n");
    Append(html, "<hr>\r\n");
    Append(html, "</body>\r\n");
    Append(html, "</html>\r\n");
    return true;
}

valiant::HttpServer::Content UriHandler(const char* uri) {
    if (!StartsWith(uri, kPrefix)) return {};
    auto path = GetPath(uri + StrLen(kPrefix) - 1);
    printf("=> Path: %s\n\r", path.c_str());

    lfs_dir_t dir;
    int res = lfs_dir_open(Lfs(), &dir, path.c_str());
    if (res < 0) {
        if (valiant::filesystem::FileExists(path.c_str())) return path;
        return {};
    }

    std::vector<uint8_t> html;
    html.reserve(1024);
    if (!DirHtml(&dir, path.c_str(), &html)) {
        lfs_dir_close(Lfs(), &dir);
        return {};
    }

    lfs_dir_close(Lfs(), &dir);
    return html;
}
}  // namespace

extern "C" void app_main(void* param) {
    printf("WebFileBrowser\r\n");
    valiant::HttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    valiant::UseHttpServer(&http_server);
    vTaskSuspend(nullptr);
}
