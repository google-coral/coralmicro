#include "apps/OOBE/oobe_json.h"

#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/base/httpd.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/mutex.h"
#include "libs/posenet/posenet.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/mjson/src/mjson.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/base64.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"

#include <cstdio>
#include <cstring>
#include <list>

// Don't reorder. Causes some failures in other headers
// due to the macros gluing lwip and curl together.
/* clang-format off */
#include "libs/curl/curl.h"
/* clang-format on */

static unsigned int id = 0;

std::vector<ip4_addr_t> host_addresses;
static SemaphoreHandle_t sema;

constexpr int kPosenetSize = 481 * 353 * 3;
static SemaphoreHandle_t posenet_input_queue_mtx;
constexpr int kPosenetInputQueueMaxSize = 3;
std::list<std::pair<unsigned int, std::unique_ptr<uint8_t[]>>> posenet_input_queue;

static int camera_http_fs_open_custom(void *context, struct fs_file *file, const char *name) {
    static_cast<void>(context);
    const char *fmt = "/camera/%u";
    unsigned int index;
    int ret = sscanf(name, fmt, &index);
    if (ret != 1) {
        return 0;
    }

    {
        valiant::MutexLock lock(posenet_input_queue_mtx);
        for (auto& it : posenet_input_queue) {
            if (it.first == index) {
                // Found it!
                file->data = reinterpret_cast<const char*>(it.second.release());
                file->len = kPosenetSize;
                file->index = kPosenetSize;
                file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
                file->is_custom_file = true;
                file->pextension = reinterpret_cast<void*>(index);
                return 1;
            }
        }
    }
    return 0;
}

static void camera_http_fs_close_custom(void *context, struct fs_file *file) {
    static_cast<void>(context);
    if (file->data) {
        delete file->data;
    }
}

static valiant::httpd::HTTPDHandlers camera_http_handlers = {
    .fs_open_custom = camera_http_fs_open_custom,
    .fs_close_custom = camera_http_fs_close_custom,
};

namespace valiant {
namespace oobe {

static void RegisterReceiver(struct jsonrpc_request *request) {
    std::vector<char> address;
    if (!valiant::testlib::JSONRPCGetStringParam(request, "address", &address)) {
        jsonrpc_return_error(request, -1, "'address' not found", nullptr);
        return;
    }
    int a, b, c, d;
    int count = sscanf(address.data(), "%d.%d.%d.%d", &a, &b, &c, &d);
    if (count != 4) {
        jsonrpc_return_error(request, -1, "malformed 'address'", nullptr);
        return;
    }
    ip4_addr_t host_address;
    IP4_ADDR(&host_address, d, c, b, a);
    host_addresses.push_back(host_address);
    jsonrpc_return_success(request, "{}");
    xSemaphoreGive(sema);
}

static void UnregisterReceiver(struct jsonrpc_request *request) {
    std::vector<char> address;
    if (!valiant::testlib::JSONRPCGetStringParam(request, "address", &address)) {
        jsonrpc_return_error(request, -1, "'address' not found", nullptr);
        return;
    }
    int a, b, c, d;
    int count = sscanf(address.data(), "%d.%d.%d.%d", &a, &b, &c, &d);
    if (count != 4) {
        jsonrpc_return_error(request, -1, "malformed 'address'", nullptr);
        return;
    }
    ip4_addr_t host_address;
    IP4_ADDR(&host_address, d, c, b, a);

    for (auto it = host_addresses.begin(); it != host_addresses.end(); ++it) {
        if (ip4_addr_eq(it, &host_address)) {
            host_addresses.erase(it);
            break;
        }
    }

    jsonrpc_return_success(request, "{}");
}

static void HandleAppMessage(const uint8_t data[valiant::ipc::kMessageBufferDataSize], void* param) {
    vTaskResume(reinterpret_cast<TaskHandle_t>(param));
}

static void SendJSONRPCRequest(const char *json) {
    CURL* curl;
    CURLcode res;
    struct curl_slist *headers = nullptr;

    if (host_addresses.empty()) {
        return;
    }

    curl = curl_easy_init();
    if (curl) {
        headers = curl_slist_append(headers, "Content-Type: application/json;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(json));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, void* param) {
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, json);
        for (auto host_address : host_addresses) {
            size_t url_len = snprintf(nullptr, 0, "http://%lu:8080/", ip_addr_get_ip4_u32(&host_address)) + 1;
            auto url = std::make_unique<char[]>(url_len);
            snprintf(url.get(), url_len, "http://%lu:8080/", ip_addr_get_ip4_u32(&host_address));
            curl_easy_setopt(curl, CURLOPT_URL, url.get());
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                printf("curl_easy_perform failed: %s\r\n", curl_easy_strerror(res));
            }
        }
        curl_slist_free_all(headers);
        headers = nullptr;
        curl_easy_cleanup(curl);
    }
}

void main() {
    curl_global_init(CURL_GLOBAL_ALL);
    sema = xSemaphoreCreateBinary();
    posenet_input_queue_mtx = xSemaphoreCreateMutex();

    valiant::httpd::Init();
    valiant::httpd::RegisterHandlerForPath("/camera", &camera_http_handlers);

    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    valiant::rpc::RPCServer rpc_server;
    if (!rpc_server_io_http.Init()) {
        vTaskSuspend(NULL);
    }
    if (!rpc_server.Init()) {
        vTaskSuspend(NULL);
    }
    rpc_server.RegisterIO(rpc_server_io_http);
    rpc_server.RegisterRPC("register_receiver", RegisterReceiver);
    rpc_server.RegisterRPC("unregister_receiver", UnregisterReceiver);

    valiant::IPCM7::GetSingleton()->RegisterAppMessageHandler(HandleAppMessage, xTaskGetCurrentTaskHandle());
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice(valiant::PerformanceMode::kMax);
    if (!valiant::posenet::setup()) {
        printf("setup() failed\r\n");
        vTaskSuspend(NULL);
    }
    static_cast<valiant::IPCM7*>(valiant::IPC::GetSingleton())->StartM4();
    vTaskSuspend(NULL);
    while (true) {
        printf("CM7 awoken\r\n");

        valiant::posenet::Output output;
        valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::STREAMING);

        TickType_t last_good_pose = xTaskGetTickCount();
        while (true) {
            TfLiteTensor *input = valiant::posenet::input();
            valiant::camera::FrameFormat fmt;
            fmt.width = input->dims->data[2];
            fmt.height = input->dims->data[1];
            fmt.fmt = valiant::camera::Format::RGB;
            fmt.preserve_ratio = false;
            fmt.buffer = tflite::GetTensorData<uint8_t>(input);
            if (posenet_input_queue.size() > kPosenetInputQueueMaxSize) {
                posenet_input_queue.erase(posenet_input_queue.begin());
            }

            valiant::CameraTask::GetFrame({fmt});
            valiant::posenet::loop(&output);
            int good_poses_count = 0;
            float kThreshold = 0.4;
            for (int i = 0; i < valiant::posenet::kPoses; ++i) {
                if (output.poses[i].score > kThreshold) {
                    good_poses_count++;
                }
            }

            TickType_t now = xTaskGetTickCount();
            {
                valiant::MutexLock lock(posenet_input_queue_mtx);
                posenet_input_queue.emplace_back(std::make_pair<unsigned int, std::unique_ptr<uint8_t[]>>(0, std::make_unique<uint8_t[]>(kPosenetSize)));
                auto& entry = posenet_input_queue.back();
                entry.first = now;
                memcpy(entry.second.get(), tflite::GetTensorData<uint8_t>(input), kPosenetSize);
            }
            {
                size_t url_len = snprintf(nullptr, 0, "/camera/%lu", now) + 1;
                auto url = std::make_unique<char[]>(url_len);
                snprintf(url.get(), url_len, "/camera/%lu", now);
                auto s = valiant::oobe::CreateFrameJSON(id++, now, good_poses_count, url.get());
                SendJSONRPCRequest(s.get());
            }

            if (good_poses_count > 0) {
                last_good_pose = now;
                for (int i = 0; i < valiant::posenet::kPoses; ++i) {
                    if (output.poses[i].score > kThreshold) {
                        auto s = valiant::oobe::CreatePoseJSON(id++, now, output, i);
                        SendJSONRPCRequest(s.get());
                    }
                }
            }

            if ((now - last_good_pose) > pdMS_TO_TICKS(5000)) {
                break;
            }
        }

        printf("Transition back to M4\r\n");
        valiant::CameraTask::GetSingleton()->Disable();
        valiant::ipc::Message msg;
        msg.type = valiant::ipc::MessageType::APP;
        valiant::IPCM7::GetSingleton()->SendMessage(msg);
        vTaskSuspend(NULL);
    }

    curl_global_cleanup();
}

}  // namespace oobe
}  // namespace valiant

extern "C" void app_main(void *param) {
    valiant::oobe::main();
    vTaskSuspend(NULL);
}
