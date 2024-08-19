# GstUnityPlugin
GStreamer plugin for Unity3D.

Tested withn Unity3D version 2022.3.42. Probably works well enough
on older versions. 

This plugin has been tested successfully on x86_64 Windows using the 
Unity3D DirectX 11, OpenGL (core), and Vulkan graphics backends.

The plugin has also been tested successfully on ARM 64 Android using 
the Open Unity3D OpenGL ES3 and Vulkan graphics backends. 

This plugin has been tested successfully on Meta Quest 3, and is able
to utilize hardware video decode features via the 
amcviddec-omxqcomvideodecoder set of gstreamer plugins available on that
device.

Debug binaries for ARM64 Android (.so) and x86_64 Windows (.dll) are 
available in the enclosed Unity3D example project.

Plugin has been built with both GStreamer 1.22 and 1.24. Enclosed binaries
are generated from GStreamer 1.24. Enclosed ARM64 Android binary generated
with NDK 25. 

Use enclosed CmakeLists.txt to build plugin for windows. This (of course)
will require GStreamer installation on build machine. Use Android NDK 25 
with the enclosed Anrdoid ndk-build project to build for ARM64 Android. 
ARM v7 32bit isn't directly supported, but probably works. 

The project *probably* won't build successfully for Android on a Windows 
machine due to the Windows limit on command string length. There may be
a work around, but I was only able to build successfully with my 
Ubuntu 22.04 workstation due to this limitation on Windows. 

This plugin was inspired by the mrayGStreamerUnity project,
available here:
https://github.com/mrayy/mrayGStreamerUnity

However, very little (if any) code in this project is a 1:1 copy
from the above. 

This plugin was also developed by directly using example code given in the 
Unity3D NativeRenderingPlugin project (just like mrayGStreamerUnity),
available here:
https://github.com/Unity-Technologies/NativeRenderingPlugin

The example project includes shaders that can handle RGB/RGBA textures
as well as NV12 textures. Don't try to use RGB w/ DirectX or Vulkan,
use RGBA instead.

TO DO:

4k video is still an issue. When streaming from something like a udpsrc,
video is decoded into an intermediary buffer, before being copied into the 
Unity 3D texture memory. If you're attempting to run the stream in an 
application with a framerate of 90hz, this copy step takes to long with
textures of that size. Frame stuttering will occurr as a result, and 
performance is rather miserable.

A better solution would be to decode video directly into Unity3D texture 
memory, thus skipping the intermediary copy step. I'm working on it. 

In the meantime, I have been able to stream 4x 1920x1080p h264 video 
streams on the Meta Quest 3 simultaneously at 60fps, with somewhere 
around 200ms latency. I am confident solving this intermediary copy 
step problem with lower that latency further.

Thanks,

Reed
