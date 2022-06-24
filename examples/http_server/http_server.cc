#include <cstring>

#include "libs/base/http_server.h"
#include "libs/base/utils.h"
#include "libs/base/strings.h"

// Hosts a simple HTTP server on the Dev Board Micro.
//
// Run it and open a web browser on your computer to the URL
// shown in the serial terminal, such as http://10.10.10.1/hello.html

namespace coral::micro {
namespace {

HttpServer::Content UriHandler(const char* path) {
    printf("Request received for %s\r\n", path);
    std::vector<uint8_t> html;
    html.reserve(64);
    if (std::strcmp(path, "/hello.html") == 0) {
        printf("Hello World!\r\n");
        StrAppend(&html, "<html><body>Hello World!</body></html>");
        return html;
    }
    return {};
}

}  // namespace
}  // namespace coral::micro

extern "C" void app_main(void* param) {
    (void)param;
    printf("Starting server...\r\n");
    coral::micro::HttpServer http_server;
    http_server.AddUriHandler(coral::micro::UriHandler);
    coral::micro::UseHttpServer(&http_server);

    std::string ip;
    if (coral::micro::utils::GetUSBIPAddress(&ip)) {
        printf("GO TO:   http://%s/%s\r\n", ip.c_str(), "/hello.html");
    }
    vTaskSuspend(nullptr);
}
