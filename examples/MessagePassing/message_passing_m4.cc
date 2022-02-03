#include "examples/MessagePassing/example_message.h"
#include "libs/base/ipc.h"
#include "libs/base/gpio.h"
#include "third_party/freertos_kernel/include/task.h"


extern "C" void app_main(void* param) {
    // Create and register message handler.
    auto message_handler = [](const uint8_t data[valiant::ipc::kMessageBufferDataSize], void* param) {
        const auto* msg = reinterpret_cast<const mp_example::ExampleAppMessage*>(data);
        if (msg->type == mp_example::ExampleMessageType::LED_STATUS) {
            switch (msg->led_status) {
                case mp_example::LEDStatus::ON: {
                    valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, true);
                    break;
                }
                case mp_example::LEDStatus::OFF: {
                    valiant::gpio::SetGpio(valiant::gpio::Gpio::kUserLED, false);
                    break;
                }
                default: {
                    printf("Unknown LED_STATUS\r\n");
                }
            }
            valiant::ipc::Message reply{};
            reply.type = valiant::ipc::MessageType::APP;
            auto *ack = reinterpret_cast<mp_example::ExampleAppMessage*>(&reply.message.data);
            ack->type = mp_example::ExampleMessageType::ACKNOWLEDGED;
            valiant::IPC::GetSingleton()->SendMessage(reply);
        }
    };
    valiant::IPC::GetSingleton()->RegisterAppMessageHandler(message_handler, nullptr);
    vTaskSuspend(nullptr);
}