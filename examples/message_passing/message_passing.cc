#include "examples/message_passing/example_message.h"
#include "libs/base/ipc_m7.h"
#include "third_party/freertos_kernel/include/task.h"

// This runs on the M7 core and sends IPC messages to the M4 with
// instructions to toggle the user LED.

extern "C" [[noreturn]] void app_main(void* param) {
    // Create and register message handler for the M7.
    auto message_handler =
        [](const uint8_t data[coral::micro::ipc::kMessageBufferDataSize],
           void* param) {
            const auto* msg =
                reinterpret_cast<const mp_example::ExampleAppMessage*>(data);
            if (msg->type == mp_example::ExampleMessageType::ACKNOWLEDGED) {
                printf("[M7] ACK received from M4\r\n");
            }
        };
    coral::micro::IPCM7::GetSingleton()->RegisterAppMessageHandler(
        message_handler, nullptr);
    coral::micro::IPCM7::GetSingleton()->StartM4();

    bool led_status{false};
    auto ipc = coral::micro::IPCM7::GetSingleton();
    while (true) {
        led_status = !led_status;
        printf("---\r\n[M7] Sending M4 LEDStatus::%s\r\n",
               led_status ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(1000));
        coral::micro::ipc::Message msg{};
        msg.type = coral::micro::ipc::MessageType::kApp;
        auto* app_message =
            reinterpret_cast<mp_example::ExampleAppMessage*>(&msg.message.data);
        app_message->type = mp_example::ExampleMessageType::LED_STATUS;
        app_message->led_status =
            led_status ? mp_example::LEDStatus::ON : mp_example::LEDStatus::OFF;
        ipc->SendMessage(msg);
    }
}
