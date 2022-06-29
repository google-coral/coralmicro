TensorFlow Inferencing APIs
============================

Utilities
---------

The following APIs simplify your code when working with
`tflite::MicroInterpreter
<https://github.com/tensorflow/tflite-micro/blob/main/tensorflow/lite/micro/micro_interpreter.h>`_.

`[utils.h source] <https://github.com/google-coral/coralmicro/blob/master/libs/tensorflow/utils.h>`_

.. doxygenfile:: tensorflow/utils.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Image classification
--------------------

The following APIs assist when running image classification models on a
microcontroller with an Edge TPU.

**Example** (from `examples/classify_image/`):

.. literalinclude:: ../examples/classify_image/classify_image.cc
   :start-after: [start-sphinx-snippet:classify-image]
   :end-before: [end-sphinx-snippet:classify-image]

`[classification.h source] <https://github.com/google-coral/coralmicro/blob/master/libs/tensorflow/classification.h>`_

.. doxygenfile:: tensorflow/classification.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Object detection
----------------

The following APIs assist when running object detection models on a
microcontroller with an Edge TPU.

**Example** (from `examples/detect_image/`):

.. literalinclude:: ../examples/detect_image/detect_image.cc
   :start-after: [start-sphinx-snippet:detect-image]
   :end-before: [end-sphinx-snippet:detect-image]

`[detection.h source] <https://github.com/google-coral/coralmicro/blob/master/libs/tensorflow/detection.h>`_

.. doxygenfile:: tensorflow/detection.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type


Pose estimation
----------------

The following APIs assist when running pose estimation with PoseNet.

`[posenet_decoder_op.h source] <https://github.com/google-coral/coralmicro/blob/master/libs/tensorflow/posenet_decoder_op.h>`_

.. doxygenfile:: tensorflow/posenet_decoder_op.h

`[posenet.h source] <https://github.com/google-coral/coralmicro/blob/master/libs/tensorflow/posenet.h>`_

.. doxygenfile:: tensorflow/posenet.h

