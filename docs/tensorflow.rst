TensorFlow Lite Micro APIs
===========================

The Coral Dev Board Micro allows you to run two types of TensorFlow models:
TensorFlow Lite Micro models that run on entirely the microcontroller (MCU) and
TensorFlow Lite models that are compiled for acceleration on the Coral Edge TPU.
Although you can run TensorFlow Lite Micro models on either MCU core
(M4 or M7), currently, you must execute Edge TPU models from the M7.

.. note::
   If you have experience with TensorFlow Lite on other platforms (including
   other Coral boards/accelerators), a lot of the code to run inference on the
   Dev Board Micro should be familiar, but the APIs are actually different
   for microcontrollers, so your code is not 100% portable.

To run any TensorFlow Lite model on the Dev Board Micro, you must use the
TensorFlow interpreter provided by
`TensorFlow Lite for Microcontrollers <https://www.tensorflow.org/lite/microcontrollers>`_
(TFLM): :cpp:any:`tflite::MicroInterpreter`.
If you're running a model on the Edge TPU, the only difference compared to
running a model on the MCU is that you
need to specify the Edge TPU custom op when you instantiate the
:cpp:any:`tflite::MicroInterpreter`
(and your model must be `compiled for the Edge TPU
</docs/edgetpu/models-intro/>`_).
The following steps describe the basic procedures to run inference on the
Dev Board Micro using either type of model.

First, you need to perform some setup:

1. If using on the Edge TPU, power on the Edge TPU with
   :cpp:any:`~coralmicro::EdgeTpuManager::OpenDevice`::

      auto tpu_context = EdgeTpuManager::GetSingleton()->OpenDevice();
      if (!tpu_context) {
        printf("ERROR: Failed to get EdgeTpu context\r\n");
      }

2. Load your ``.tflite`` model from a file into a byte array
   with :cpp:any:`~coralmicro::LfsReadFile`::

      constexpr char kModelPath[] =
          "/models/tf2_ssd_mobilenet_v2_coco17_ptq_edgetpu.tflite";
      std::vector<uint8_t> model;
      if (!LfsReadFile(kModelPath, &model)) {
        printf("ERROR: Failed to load %s\r\n", kModelPath);
      }

   **Note:** Some micro ML apps instead load their model from a C array that's
   compiled with the app, which is also an option, but that's intended for
   microcontrollers without a filesystem (Dev Board Micro has a
   littlefs filesystem).

3. Specify each of the TensorFlow ops required by your model with a
   :cpp:any:`~tflite::MicroMutableOpResolver`. When using a model compiled for
   the Edge TPU, you must include the :cpp:any:`~coralmicro::kCustomOp` with
   :cpp:any:`~tflite::MicroMutableOpResolver::AddCustom`. For example::

       tflite::MicroMutableOpResolver<3> resolver;
       resolver.AddDequantize();
       resolver.AddDetectionPostprocess();
       resolver.AddCustom(kCustomOp, RegisterCustomOp());

4. Specify the memory arena required for your model's input, output, and
   intermediate tensors. To ensure 16-bit alignment (required by TFLM) and
   avoid running out of heap space, you should use either the
   :c:macro:`STATIC_TENSOR_ARENA_IN_SDRAM` or
   :c:macro:`STATIC_TENSOR_ARENA_IN_OCRAM` macro to allocate your
   tensor arena::

      constexpr int kTensorArenaSize = 8 * 1024 * 1024;
      STATIC_TENSOR_ARENA_IN_SDRAM(tensor_arena, kTensorArenaSize);

   Selecting the best arena size depends on the model and requires some
   trial-and-error: Just start with a small number like 1024 and run it;
   TFLM will throw an error at runtime and tell you the size you actually need.

5. Instantiate a :cpp:any:`~tflite::MicroInterpreter`, passing it your model,
   op resolver, tensor arena, and a :cpp:any:`~tflite::MicroErrorReporter`::

       tflite::MicroErrorReporter error_reporter;
       tflite::MicroInterpreter interpreter(tflite::GetModel(model.data()),
                                            resolver, tensor_arena,
                                            kTensorArenaSize, &error_reporter);

6. Allocate all model tensors with
   :cpp:any:`~tflite::MicroInterpreter::AllocateTensors`::

       if (interpreter.AllocateTensors() != kTfLiteOk) {
         printf("ERROR: AllocateTensors() failed\r\n");
       }


Now you're ready to run each inference as follows:

1. Get the allocated input tensor with
   :cpp:any:`~tflite::MicroInterpreter::input_tensor` and fill it with your
   input data. For example, if you're
   using the Dev Board Micro camera, you can simply set the input tensor as the
   ``buffer`` for your :cpp:any:`~coralmicro::CameraFrameFormat` (see
   ``examples/detect_faces/``). Or you can copy
   your input data using :cpp:any:`~tflite::micro::GetTensorData` and
   :cpp:any:`std::memcpy` like this::

       auto* input_tensor = interpreter.input_tensor(0);
       std::memcpy(tflite::GetTensorData<uint8_t>(input_tensor), image.data(),
                   image.size());

2. Execute the model with :cpp:any:`~tflite::MicroInterpreter::Invoke()`::

       if (interpreter.Invoke() != kTfLiteOk) {
         printf("ERROR: Invoke() failed\r\n");
       }

3. Similar to writing the input tensor, you can then read the output tensor with
   :cpp:any:`~tflite::micro::GetTensorData` by passing
   it :cpp:any:`~tflite::MicroInterpreter::output_tensor`.

   However, instead of processing this output data yourself, you can use the
   APIs below that correspond to the type of model you're running. For example,
   if you're running an object detection model, instead of reading the output
   tensor directly, call :cpp:any:`~coralmicro::tensorflow::GetDetectionResults`
   and pass it your :cpp:any:`~tflite::MicroInterpreter`. This function returns
   an :cpp:any:`~coralmicro::tensorflow::Object` for each detected object, which
   specifies the detected object's label id, prediction score, and
   bounding-box coordinates::

       auto results = tensorflow::GetDetectionResults(&interpreter, 0.6, 3);
       printf("%s\r\n", tensorflow::FormatDetectionOutput(results).c_str());

See the following documentation for more code examples, each of which is
included from the coralmicro examples, which you can browse at
`coralmicro/examples/
<https://github.com/google-coral/coralmicro/blob/main/examples>`_.

Also check out the `TensorFlow Lite for Microcontrollers documentation
<https://www.tensorflow.org/lite/microcontrollers>`_.


TFLM interpreter
--------------------

This is just a small set of APIs from TensorFlow Lite for Microcontrollers
(TFLM) that represent the core APIs you need to run inference on the Dev Board
Micro. You can see the rest of the TFLM APIs in
``coralmicro/third_party/tflite-micro/``.

.. note::
   The version of TFLM included in coralmicro is not continuously updated,
   so some APIs might be different from the latest version of `TFLM on GitHub
   <https://github.com/tensorflow/tflite-micro>`_.

For usage examples, see the following sections, such as for
:ref:`image classification<Image classification>`.


`[micro_interpreter.h source] <https://github.com/tensorflow/tflite-micro/tree/24c08505dfd2a97b343220bd1c4006f881061ea6/tensorflow/lite/micro/micro_interpreter.h>`_

.. doxygenclass:: tflite::MicroInterpreter
   :members:


`[schema_generated.h source] <https://github.com/tensorflow/tflite-micro/tree/24c08505dfd2a97b343220bd1c4006f881061ea6/tensorflow/lite/schema/schema_generated.h>`_

.. doxygenfunction:: tflite::GetModel


`[micro_mutable_op_resolver.h source] <https://github.com/tensorflow/tflite-micro/tree/24c08505dfd2a97b343220bd1c4006f881061ea6/tensorflow/lite/micro/micro_mutable_op_resolver.h>`_

.. doxygenclass:: tflite::MicroMutableOpResolver
   :members: AddCustom

.. note::
   The :cpp:any:`tflite::MicroMutableOpResolver` has a long list of ``Add...``
   functions to specify the ops that you need for your model. To see them
   all, refer to the `micro_mutable_op_resolver.h source code
   <https://github.com/google-coral/coralmicro/blob/main/third_party/tflite-micro/tensorflow/lite/micro/micro_mutable_op_resolver.h>`_.


`[micro_error_reporter.h source] <https://github.com/tensorflow/tflite-micro/tree/24c08505dfd2a97b343220bd1c4006f881061ea6/tensorflow/lite/micro/micro_error_reporter.h>`_

.. doxygenclass:: tflite::MicroErrorReporter
   :members:
   :undoc-members:


`[kernel_util.h source] <https://github.com/tensorflow/tflite-micro/tree/24c08505dfd2a97b343220bd1c4006f881061ea6/tensorflow/lite/micro/kernels/kernel_util.h>`_

.. doxygenfunction:: tflite::micro::GetTensorData(TfLiteEvalTensor *tensor)



Edge TPU runtime
------------------

.. note::
   The Edge TPU is not available within M4 programs.

These APIs provide access to the Edge TPU on the Dev Board Micro.
Anytime you want to use the Edge TPU for acceleration with
:cpp:any:`~tflite::MicroInterpreter`, you need to do two things:

1. Start the Edge TPU with :cpp:any:`~coralmicro::EdgeTpuManager::OpenDevice`.

2. Register the Edge TPU custom op with your interpreter by passing
   :cpp:any:`~coralmicro::kCustomOp` and
   :cpp:any:`~coralmicro::RegisterCustomOp`
   to :cpp:any:`tflite::MicroMutableOpResolver::AddCustom`.

**Example** (from `examples/classify_images_file/`):

.. literalinclude:: ../examples/classify_images_file/classify_images_file.cc
   :start-after: [start-sphinx-snippet:edgetpu]
   :end-before: [end-sphinx-snippet:edgetpu]


.. note::
   Unlike the libcoral C++ API, when using this coralmicro C++ API, you
   do not need to pass the ``EdgeTpuContext`` to the
   ``tflite::MicroInterpreter``, but the context must be opened and the custom
   op must be registered before you create an interpreter. (This is different
   because libcoral is based on TensorFlow Lite and coralmicro is based on
   TensorFlow Lite for Microcontrollers.)

`[edgetpu_manager.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tpu/edgetpu_manager.h>`_

.. doxygenfile:: tpu/edgetpu_manager.h
   :sections: briefdescription detaileddescription innernamespace innerclass public-func public-slot public-attrib public-static-func public-static-attrib


`[edgetpu_op.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tpu/edgetpu_op.h>`_

.. doxygenfile:: tpu/edgetpu_op.h



Image classification
--------------------

These APIs simplify the pre- and post-processing for image classification models.

**Example** (from `examples/classify_images_file/`):

.. literalinclude:: ../examples/classify_images_file/classify_images_file.cc
   :start-after: [start-sphinx-snippet:classify-image]
   :end-before: [end-sphinx-snippet:classify-image]

`[classification.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/classification.h>`_

.. doxygenfile:: tensorflow/classification.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Object detection
----------------

These APIs simplify the post-processing for object detection models.

**Example** (from `examples/detect_objects_file/`):

.. literalinclude:: ../examples/detect_objects_file/detect_objects_file.cc
   :start-after: [start-sphinx-snippet:detect-image]
   :end-before: [end-sphinx-snippet:detect-image]

`[detection.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/detection.h>`_

.. doxygenfile:: tensorflow/detection.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Pose estimation
----------------

These APIs not only simplify the post-processing for pose estimation with
PoseNet, but also optimize execution of the post-processing layers on the MCU
with a custom op (because the post-processing ops are not compatible with the
Edge TPU).

So when running PoseNet, in addition to specifying the
:cpp:any:`~coralmicro::kCustomOp` for the
Edge TPU, you should also register the :cpp:any:`~coralmicro::kPosenetDecoderOp`
provided here.

**Example** (from `examples/detect_poses/`):

.. literalinclude:: ../examples/detect_poses/detect_poses.cc
   :start-after: [start-sphinx-snippet:posenet]
   :end-before: [end-sphinx-snippet:posenet]


`[posenet_decoder_op.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/posenet_decoder_op.h>`_

.. doxygenfile:: tensorflow/posenet_decoder_op.h

`[posenet.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/posenet.h>`_

.. doxygenfile:: tensorflow/posenet.h


Audio Classification
----------------------

The following APIs assist with running audio classification models on the
Dev Board Micro, either on CPU or Edge TPU. For supported models, see the
`audio classification models </models/audio-classification/>`_.

For an example, see ``examples/classify_speech/``.

`[audio_models.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/audio_models.h>`_

.. doxygenfile:: tensorflow/audio_models.h




Utilities
---------

The following functions help with some common tasks during inferencing, such as
manipulate images and tensors.

`[utils.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/tensorflow/utils.h>`_

.. doxygenfile:: tensorflow/utils.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type
