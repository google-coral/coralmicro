Camera APIs
=============================

The Dev Board Micro includes an on-board camera module with
324 x 324 px resolution. All camera control is handled through the
:cpp:any:`~coralmicro::CameraTask` singleton.

To get started, you must acquire the ``CameraTask`` object with
:cpp:any:`~coralmicro::CameraTask::GetSingleton`.
Then power on the camera with :cpp:any:`~coralmicro::CameraTask::SetPower`
and specify the camera mode (trigger or streaming mode) with
:cpp:any:`~coralmicro::CameraTask::Enable`.

If you enable trigger mode, the camera captures a single image and saves it to
the camera's memory when you call :cpp:any:`~coralmicro::CameraTask::Trigger`.

If you enable streaming mode, the camera continuously captures new images and
saves them to an internal buffer.

In either mode, you must manually fetch the latest image by calling
:cpp:any:`~coralmicro::CameraTask::GetFrame`,
which requires you to specify the image format you want with
:cpp:any:`~coralmicro::CameraFrameFormat`. This object also specifies
your own buffer where you want to save the processed image. You can specify
multiple formats for each frame, which is useful if you want one format to use
as input for your ML model and another format to use for display.

.. note::
   In streaming mode, your perceived framerate is only as fast as your main
   loop, because :cpp:any:`~coralmicro::CameraTask::GetFrame` returns only
   the most recent frame captured. And if your loop is faster
   than the camera, it will be blocked by
   :cpp:any:`~coralmicro::CameraTask::GetFrame` until a new frame is available.

For example, the following code enables the camera with trigger mode so it
will capture an image when you press the User button on the Dev Board Micro.
The image is saved internally in raw format until the app fetches it with
:cpp:any:`~coralmicro::CameraTask::GetFrame`, which is called by the
``GetCapturedImage()`` RPC function.

This example is available in ``coralmicro/examples/camera_triggered/``, which
also provides a Python client to fetch images over RPC.

.. literalinclude:: ../examples/camera_triggered/camera_triggered.cc
   :start-after: [start-snippet:camera-trigger]
   :end-before: [end-snippet:camera-trigger]


`[camera.h source] <https://github.com/google-coral/coralmicro/blob/main/libs/camera/camera.h>`_

.. doxygenfile:: camera/camera.h
   :sections: briefdescription detaileddescription innernamespace innerclass define func public-attrib public-func public-slot public-static-attrib public-static-func public-type enum
