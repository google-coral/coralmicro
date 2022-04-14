#include <algorithm>
#include <cstdio>
#include <cstring>
#include <map>
#include <vector>

#include "libs/base/filesystem.h"
#include "libs/base/httpd.h"
#include "libs/base/utils.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

using valiant::filesystem::Lfs;

namespace {
constexpr char kUrlPrefix[] = "/fs/";

template <size_t N>
constexpr size_t StrLen(const char (&)[N]) {
    return N - 1;
}

template <size_t N>
bool StartsWith(const char* s, const char (&prefix)[N]) {
    return std::strncmp(s, prefix, StrLen(prefix)) == 0;
}

template <size_t N>
bool EndsWith(const std::string& s, const char (&suffix)[N]) {
    if (s.size() < StrLen(suffix)) return false;
    return std::strcmp(s.c_str() + s.size() - StrLen(suffix), suffix) == 0;
}

template <typename... T>
void Append(std::vector<uint8_t>* v, const char* format, T... args) {
    const int size = std::snprintf(nullptr, 0, format, args...) + 1;
    v->resize(v->size() + size);
    auto* s = reinterpret_cast<char*>(v->data() + v->size() - size);
    std::snprintf(s, size, format, args...);
    v->pop_back();  // remove null terminator
}

void RemoveDuplicateChar(std::string& s, char ch) {
    s.erase(std::unique(s.begin(), s.end(),
                        [ch](char a, char b) { return a == ch && b == ch; }),
            s.end());
}

std::string GetPath(const char* uri) {
    static constexpr char kIndexShtml[] = "index.shtml";
    std::string path(uri);
    RemoveDuplicateChar(path, '/');

    if (EndsWith(path, kIndexShtml))
        return path.substr(0, path.size() - StrLen(kIndexShtml));
    return path;
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
    Append(html, "<table>\r\n");
    int res;
    lfs_info info;
    while ((res = lfs_dir_read(Lfs(), dir, &info)) != 0) {
        if (res < 0) return false;

        if (info.type == LFS_TYPE_REG) {
            Append(html, "<tr>\r\n");
            Append(html, "<td>&#128462;</td>\r\n");
            Append(html, "<td><a href=\"%s%s%s\">%s</a></td>\r\n", kUrlPrefix,
                   dirname + 1, info.name, info.name);
            Append(html, "<td>%d</td>", info.size);
            Append(html, "</tr>\r\n");
        } else if (info.type == LFS_TYPE_DIR) {
            if (std::strcmp(info.name, ".") == 0) continue;
            Append(html, "<tr>\r\n");
            Append(html, "<td>&#128193;</td>\r\n");
            Append(html, "<td><a href=\"%s%s%s/\">%s/</a></td>\r\n", kUrlPrefix,
                   dirname + 1, info.name, info.name);
            Append(html, "<td></td>\r\n");
            Append(html, "</tr>\r\n");
        }
    }
    Append(html, "</table>\r\n");
    Append(html, "<hr>\r\n");
    Append(html, "</body>\r\n");
    Append(html, "</html>\r\n");
    return true;
}

valiant::HttpServer::Content UriHandler(const char* uri) {
    if (!StartsWith(uri, kUrlPrefix)) return {};
    auto path = GetPath(uri + StrLen(kUrlPrefix) - 1);
    assert(!path.empty() && path.front() == '/');

    printf("GET %s => %s\r\n", uri, path.c_str());

    lfs_dir_t dir;
    int res = lfs_dir_open(Lfs(), &dir, path.c_str());
    if (res < 0) {
        if (valiant::filesystem::FileExists(path.c_str())) return path;
        return {};
    }

    // Make sure directory path has '/' at the end.
    if (path.back() != '/') path.push_back('/');

    std::vector<uint8_t> html;
    html.reserve(1024);
    if (!DirHtml(&dir, path.c_str(), &html)) {
        lfs_dir_close(Lfs(), &dir);
        return {};
    }

    lfs_dir_close(Lfs(), &dir);
    return html;
}

class FileHttpServer : public valiant::HttpServer {
   public:
    err_t PostBegin(void* connection, const char* uri, const char* http_request,
                    u16_t http_request_len, int content_len, char* response_uri,
                    u16_t response_uri_len, u8_t* post_auto_wnd) override {
        (void)http_request;
        (void)http_request_len;
        (void)response_uri;
        (void)response_uri_len;
        (void)post_auto_wnd;

        if (!StartsWith(uri, kUrlPrefix)) return ERR_ARG;
        auto path = GetPath(uri + StrLen(kUrlPrefix) - 1);
        assert(!path.empty() && path.front() == '/');

        if (path.back() == '/') return ERR_ARG;
        assert(path.size() > 1);

        printf("POST %s (%d bytes) => %s\r\n", uri, content_len, path.c_str());

        auto dirname = valiant::filesystem::Dirname(path.c_str());
        if (!valiant::filesystem::MakeDirs(dirname.c_str())) return ERR_ARG;

        auto file = std::make_unique<lfs_file_t>();
        if (lfs_file_open(Lfs(), file.get(), path.c_str(),
                          LFS_O_WRONLY | LFS_O_TRUNC | LFS_O_CREAT) < 0)
            return ERR_ARG;

        files_[connection] = {std::move(file), std::move(dirname)};
        return ERR_OK;
    }

    err_t PostReceiveData(void* connection, struct pbuf* p) override {
        auto* file = files_[connection].file.get();

        struct pbuf* cp = p;
        while (cp) {
            if (lfs_file_write(Lfs(), file, cp->payload, cp->len) != cp->len)
                return ERR_ARG;
            cp = p->next;
        }

        pbuf_free(p);
        return ERR_OK;
    }

    void PostFinished(void* connection, char* response_uri,
                      u16_t response_uri_len) override {
        const auto& entry = files_[connection];
        lfs_file_close(Lfs(), entry.file.get());
        snprintf(response_uri, response_uri_len, "%s%s", kUrlPrefix,
                 entry.dirname.c_str() + 1);
        files_.erase(connection);
    }

   private:
    struct Entry {
        std::unique_ptr<lfs_file_t> file;
        std::string dirname;
    };
    std::map<void*, Entry> files_;  // connection-to-entry map
};

}  // namespace

extern "C" void app_main(void* param) {
    (void)param;
    printf("WebFileBrowser\r\n");
    FileHttpServer http_server;
    http_server.AddUriHandler(UriHandler);
    valiant::UseHttpServer(&http_server);

    std::string ip;
    if (valiant::utils::GetUSBIPAddress(&ip)) {
        printf("BROWSE:   http://%s%s\r\n", ip.c_str(), kUrlPrefix);
        printf("UPLOAD:   curl -X POST http://%s%sfile --data-binary @file\r\n",
               ip.c_str(), kUrlPrefix);
        printf("DOWNLOAD: curl -O http://%s%sfile\r\n", ip.c_str(), kUrlPrefix);
    }

    vTaskSuspend(nullptr);
}
