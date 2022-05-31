#ifndef LIBS_BASE_HTTP_SERVER_HANDLERS_H_
#define LIBS_BASE_HTTP_SERVER_HANDLERS_H_

#include "libs/base/http_server.h"

namespace coral::micro {

struct FileSystemUriHandler {
    const char* prefix = "/fs/";
    HttpServer::Content operator()(const char* uri);
};

struct TaskStatsUriHandler {
    const char* name = "/stats.html";
    HttpServer::Content operator()(const char* uri);
};

}  // namespace coral::micro

#endif  // LIBS_BASE_HTTP_SERVER_HANDLERS_H_
