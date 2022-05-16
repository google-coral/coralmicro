#include "examples/MessagePassing/example_message.h"
#include "libs/base/ipc_m4.h"
#include "libs/base/led.h"
#include "third_party/freertos_kernel/include/task.h"

extern "C" void app_main(void* param) {
    // Create and register message handler.
    auto message_handler =
        [](const uint8_t data[coral::micro::ipc::kMessageBufferDataSize],
           void* param) {
            const auto* msg =
                reinterpret_cast<const mp_example::ExampleAppMessage*>(data);
            if (msg->type == mp_example::ExampleMessageType::LED_STATUS) {
                switch (msg->led_status) {
                    case mp_example::LEDStatus::ON: {
                        coral::micro::led::Set(coral::micro::led::LED::kUser,
                                               true);
                        break;
                    }
                    case mp_example::LEDStatus::OFF: {
                        coral::micro::led::Set(coral::micro::led::LED::kUser,
                                               false);
                        break;
                    }
                    default: {
                        printf("Unknown LED_STATUS\r\n");
                    }
                }
                coral::micro::ipc::Message reply{};
                reply.type = coral::micro::ipc::MessageType::APP;
                auto* ack = reinterpret_cast<mp_example::ExampleAppMessage*>(
                    &reply.message.data);
                ack->type = mp_example::ExampleMessageType::ACKNOWLEDGED;
                coral::micro::IPCM4::GetSingleton()->SendMessage(reply);
            }
        };
    coral::micro::IPCM4::GetSingleton()->RegisterAppMessageHandler(
        message_handler, nullptr);
    vTaskSuspend(nullptr);
}
