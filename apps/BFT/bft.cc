#include "apps/BFT/micro_test.h"
#include "libs/base/filesystem.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/random.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "libs/tasks/PowerMonitorTask/power_monitor_task.h"
#include "libs/tpu/edgetpu_manager.h"
#include "libs/tpu/edgetpu_op.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "third_party/tensorflow/tensorflow/lite/micro/micro_interpreter.h"
#include "third_party/tensorflow/tensorflow/lite/version.h"
#include <cstdio>

namespace testing {

static tflite::MicroMutableOpResolver<1> resolver;
constexpr size_t kTensorArenaSize = 1024 * 10 * 2;
static uint8_t tensor_arena[kTensorArenaSize] __attribute__((aligned(16)));
size_t testconv1_edgetpu_tflite_len, testconv1_test_input_bin_len, testconv1_expected_output_bin_len;
const uint8_t *testconv1_edgetpu_tflite;
const uint8_t *testconv1_expected_output_bin;
const uint8_t *testconv1_test_input_bin;

constexpr size_t sdram_memory_size = 1024 * 1024;
static uint8_t sdram_memory[sdram_memory_size] __attribute__((section(".sdram_bss,\"aw\",%nobits @")));

constexpr size_t kAudioSamples = 512;
static uint16_t audio_samples[kAudioSamples];

TF_LITE_MICRO_TESTS_BEGIN

TF_LITE_MICRO_TEST(MulticoreTest_CheckM4) {
    valiant::IPCM7* ipc = static_cast<valiant::IPCM7*>(valiant::IPC::GetSingleton());
    TF_LITE_MICRO_EXPECT_NE(ipc, nullptr);
    ipc->StartM4();
    TF_LITE_MICRO_EXPECT(
        ipc->M4IsAlive(1000 /* ms */));
}

TF_LITE_MICRO_TEST(PowerMonitorTest_CheckChipId) {
    const char* kMfrModel = "INA233";
    valiant::power_monitor::ChipIdResponse chip_id =
        valiant::PowerMonitorTask::GetSingleton()->GetChipId();
    TF_LITE_MICRO_EXPECT_EQ(memcmp(kMfrModel, chip_id.mfr_model, strlen(kMfrModel)), 0);
    valiant::power_monitor::MeasurementResponse measurement =
        valiant::PowerMonitorTask::GetSingleton()->GetMeasurement();
    // TODO(atv): This should probably be a higher value: if we get back 2V it's probably an error.
    TF_LITE_MICRO_EXPECT_GE(measurement.voltage, 2.0f);
    TF_LITE_MICRO_EXPECT_LE(measurement.voltage, 5.2f);
}

TF_LITE_MICRO_TEST(TPUTest_CheckTestConv1) {
    valiant::EdgeTpuTask::GetSingleton()->SetPower(true);
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();

    testconv1_edgetpu_tflite = valiant::filesystem::ReadToMemory("/models/testconv1-edgetpu.tflite", &testconv1_edgetpu_tflite_len);
    testconv1_expected_output_bin = valiant::filesystem::ReadToMemory("/models/testconv1-expected-output.bin", &testconv1_expected_output_bin_len);
    testconv1_test_input_bin = valiant::filesystem::ReadToMemory("/models/testconv1-test-input.bin", &testconv1_test_input_bin_len);

    if (!testconv1_edgetpu_tflite || testconv1_edgetpu_tflite_len == 0) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load model!");
        return false;
    }
    if (!testconv1_expected_output_bin || testconv1_expected_output_bin_len == 0) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load expected output!");
        return false;
    }
    if (!testconv1_test_input_bin || testconv1_test_input_bin_len == 0) {
        TF_LITE_REPORT_ERROR(micro_test::reporter, "Failed to load test input!");
        return false;
    }

    const tflite::Model *model = tflite::GetModel(testconv1_edgetpu_tflite);
    TF_LITE_MICRO_EXPECT_EQ(static_cast<int>(model->version()), TFLITE_SCHEMA_VERSION);

    resolver.AddCustom("edgetpu-custom-op", valiant::RegisterCustomOp());
    tflite::MicroInterpreter interpreter(model, resolver, tensor_arena, kTensorArenaSize, micro_test::reporter);
    TF_LITE_MICRO_EXPECT_EQ(interpreter.AllocateTensors(), kTfLiteOk);

    TfLiteTensor *input = interpreter.input(0);
    TfLiteTensor *output = interpreter.output(0);
    TF_LITE_MICRO_EXPECT_EQ(input->bytes, testconv1_test_input_bin_len);
    TF_LITE_MICRO_EXPECT_EQ(output->bytes, testconv1_expected_output_bin_len);
    memcpy(tflite::GetTensorData<uint8_t>(input), testconv1_test_input_bin, testconv1_test_input_bin_len);

    for (int i = 0; i < 100; ++i) {
        memset(tflite::GetTensorData<uint8_t>(output), 0, output->bytes);
        TF_LITE_MICRO_EXPECT_EQ(interpreter.Invoke(), kTfLiteOk);
        TF_LITE_MICRO_EXPECT_EQ(
                memcmp(tflite::GetTensorData<uint8_t>(output), testconv1_expected_output_bin, testconv1_expected_output_bin_len),
                0);
    }
}

TF_LITE_MICRO_TEST(CameraTest_CheckTestPattern) {
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable();
    valiant::CameraTask::GetSingleton()->SetTestPattern(
            valiant::camera::TestPattern::WALKING_ONES);

    uint8_t *buffer = nullptr;
    int index = -1;
    index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);

    TF_LITE_MICRO_EXPECT_GE(index, 0);
    TF_LITE_MICRO_EXPECT(buffer);

    uint8_t expected = 0;
    uint8_t mismatch_count = 0;
    for (unsigned int i = 0; i < valiant::CameraTask::kWidth * valiant::CameraTask::kHeight; ++i) {
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

    valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    valiant::CameraTask::GetSingleton()->Disable();
    valiant::CameraTask::GetSingleton()->SetPower(false);
}

// Check that no more kBadSamplePercentage of samples are
// either our fill value or the default return value.
// This helps to accomodate for the time for the microphone to start returning data
// after clock/power application.
// If the audio pipeline interface changes to eliminate that, reduce the percentage.
TF_LITE_MICRO_TEST(AudioTest_CheckNonZeroSamples) {
    constexpr float kBadSamplePercentage = 0.2;
    constexpr uint16_t kFillValue = 0xa5a5;
    constexpr uint16_t kDefaultValue = 0x9000;
    SemaphoreHandle_t sema = xSemaphoreCreateBinary();
    memset(audio_samples, kFillValue, ARRAY_SIZE(audio_samples));
    valiant::AudioTask::GetSingleton()->SetCallback([](void *param) {
        static int buffer_count = 0;
        if (buffer_count == 4) {
            BaseType_t reschedule = pdFALSE;
            SemaphoreHandle_t sema = reinterpret_cast<SemaphoreHandle_t>(param);
            xSemaphoreGiveFromISR(sema, &reschedule);
            portYIELD_FROM_ISR(reschedule);
            return reinterpret_cast<uint32_t*>(0);
        } else {
            buffer_count++;
            return reinterpret_cast<uint32_t*>(audio_samples);
        }
    }, sema);
    valiant::AudioTask::GetSingleton()->SetPower(true);
    valiant::AudioTask::GetSingleton()->SetBuffer(reinterpret_cast<uint32_t*>(audio_samples), sizeof(audio_samples));
    valiant::AudioTask::GetSingleton()->Enable();
    xSemaphoreTake(sema, pdMS_TO_TICKS(1000));

    int non_matching_count = 0;
    for (size_t i = 0; i < ARRAY_SIZE(audio_samples); ++i) {
        if (audio_samples[i] == kFillValue || audio_samples[i] == kDefaultValue) {
            non_matching_count++;
        }
    }
    TF_LITE_MICRO_EXPECT_LT(non_matching_count, static_cast<int>(kAudioSamples * kBadSamplePercentage));
    if (non_matching_count >= static_cast<int>(kAudioSamples * kBadSamplePercentage)) {
        printf("non_matching_count: %d\r\n", non_matching_count);
    }

    valiant::AudioTask::GetSingleton()->Disable();
    valiant::AudioTask::GetSingleton()->SetPower(false);
    vSemaphoreDelete(sema);
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
    TF_LITE_MICRO_EXPECT_EQ(valiant::PmicTask::GetSingleton()->GetChipId(), 0x62);
}

TF_LITE_MICRO_TEST(RandomTest_CheckRandom) {
    constexpr int kRandomValues = 4;
    constexpr uint8_t kConstByte = 0xa5;
    constexpr uint32_t kConstInt = 0xa5a5a5a5;
    uint32_t data[kRandomValues];
    memset(data, kConstByte, sizeof(data));

    bool success = valiant::Random::GetSingleton()->GetRandomNumber(data, sizeof(data));
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
