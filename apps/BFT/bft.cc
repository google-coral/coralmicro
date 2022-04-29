#include "apps/BFT/micro_test.h"
#include "libs/base/filesystem.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/random.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tflite-micro/tensorflow/lite/micro/micro_interpreter.h"
#include <cstdio>
#include <vector>

namespace testing {

static tflite::MicroMutableOpResolver<1> resolver;
constexpr size_t kTensorArenaSize = 1024 * 10 * 2;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
std::vector<uint8_t> testconv1_edgetpu_tflite;
std::vector<uint8_t> testconv1_expected_output_bin;
std::vector<uint8_t> testconv1_test_input_bin;

constexpr size_t sdram_memory_size = 1024 * 1024;
static uint8_t sdram_memory[sdram_memory_size] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

TF_LITE_MICRO_TESTS_BEGIN

TF_LITE_MICRO_TEST(MulticoreTest_CheckM4) {
    auto* ipc = coral::micro::IPCM7::GetSingleton();
    TF_LITE_MICRO_EXPECT_NE(ipc, nullptr);
    ipc->StartM4();
    TF_LITE_MICRO_EXPECT(ipc->M4IsAlive(1000/*ms*/));
}

TF_LITE_MICRO_TEST(TPUTest_CheckTestConv1) {
    coral::micro::EdgeTpuTask::GetSingleton()->SetPower(true);
    coral::micro::EdgeTpuManager::GetSingleton()->OpenDevice();

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-edgetpu.tflite",
                                       &testconv1_edgetpu_tflite)) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load model!");
        return false;
    }

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-expected-output.bin",
                                       &testconv1_expected_output_bin)) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load expected output!");
        return false;

    }

    if (!coral::micro::filesystem::ReadFile("/models/testconv1-test-input.bin",
                                       &testconv1_test_input_bin)) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load test input!");
        return false;
    }

    const tflite::Model *model = tflite::GetModel(testconv1_edgetpu_tflite.data());
    TF_LITE_MICRO_EXPECT_EQ(static_cast<int>(model->version()), TFLITE_SCHEMA_VERSION);

    resolver.AddCustom("edgetpu-custom-op", coral::micro::RegisterCustomOp());
    tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, micro_test::reporter);
    TF_LITE_MICRO_EXPECT_EQ(interpreter.AllocateTensors(), kTfLiteOk);

    TfLiteTensor *input = interpreter.input(0);
    TfLiteTensor *output = interpreter.output(0);
    TF_LITE_MICRO_EXPECT_EQ(input->bytes, testconv1_test_input_bin.size());
    TF_LITE_MICRO_EXPECT_EQ(output->bytes, testconv1_expected_output_bin.size());
    memcpy(tflite::GetTensorData<uint8_t>(input), testconv1_test_input_bin.data(), testconv1_test_input_bin.size());

    for (int i = 0; i < 100; ++i) {
        memset(tflite::GetTensorData<uint8_t>(output), 0, output->bytes);
        TF_LITE_MICRO_EXPECT_EQ(interpreter.Invoke(), kTfLiteOk);
        TF_LITE_MICRO_EXPECT_EQ(
                memcmp(tflite::GetTensorData<uint8_t>(output), testconv1_expected_output_bin.data(), testconv1_expected_output_bin.size()),
                0);
    }
}

TF_LITE_MICRO_TEST(CameraTest_CheckTestPattern) {
    coral::micro::CameraTask::GetSingleton()->SetPower(true);
    coral::micro::CameraTask::GetSingleton()->Enable(coral::micro::camera::Mode::STREAMING);
    coral::micro::CameraTask::GetSingleton()->SetTestPattern(
            coral::micro::camera::TestPattern::WALKING_ONES);

    uint8_t *buffer = nullptr;
    int index = -1;
    index = coral::micro::CameraTask::GetSingleton()->GetFrame(&buffer, true);

    TF_LITE_MICRO_EXPECT_GE(index, 0);
    TF_LITE_MICRO_EXPECT(buffer);

    uint8_t expected = 0;
    uint8_t mismatch_count = 0;
    for (unsigned int i = 0; i < coral::micro::CameraTask::kWidth * coral::micro::CameraTask::kHeight; ++i) {
        if (buffer[i] != expected) {
            mismatch_count++;
        }
        if (expected == 0) {
            expected = 1;
        } else {
            expected = expected << 1;
        }
    }
    TF_LITE_MICRO_EXPECT_EQ(mismatch_count, 0);

    coral::micro::CameraTask::GetSingleton()->ReturnFrame(index);
    coral::micro::CameraTask::GetSingleton()->Disable();
    coral::micro::CameraTask::GetSingleton()->SetPower(false);
}

TF_LITE_MICRO_TEST(SDRAMTest_CheckReadWrite) {
    for (size_t i = 0; i < sdram_memory_size; ++i) {
        sdram_memory[i] = (i % 256);
    }
    for (size_t i = 0; i < sdram_memory_size; ++i) {
        TF_LITE_MICRO_EXPECT_EQ(sdram_memory[i], (i % 256));
    }
}

TF_LITE_MICRO_TEST(PmicTest_CheckChipId) {
    TF_LITE_MICRO_EXPECT_EQ(coral::micro::PmicTask::GetSingleton()->GetChipId(), 0x62);
}

TF_LITE_MICRO_TEST(RandomTest_CheckRandom) {
    constexpr int kRandomValues = 4;
    constexpr uint8_t kConstByte = 0xa5;
    constexpr uint32_t kConstInt = 0xa5a5a5a5;
    uint32_t data[kRandomValues];
    memset(data, kConstByte, sizeof(data));

    bool success = coral::micro::Random::GetSingleton()->GetRandomNumber(data, sizeof(data));
    TF_LITE_MICRO_EXPECT(success);

    for (size_t i = 0; i < ARRAY_SIZE(data); ++i) {
        TF_LITE_MICRO_EXPECT_NE(data[i], kConstInt);
    }
}

if (micro_test::tests_failed == 0) {
    printf("Blink LEDs success\r\n");
} else {
    printf("Blink LEDs failure\r\n");
}

TF_LITE_MICRO_TESTS_END

}  // namespace testing

extern "C" void app_main(void *param) {
    testing::main(0, NULL);
    vTaskSuspend(NULL);
}
