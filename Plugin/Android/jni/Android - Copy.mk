LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

IMMERSIATV_PLAYER_ROOT  := $(abspath $(LOCAL_PATH)/../../)
PLUGIN_SOURCE_PATH      := $(IMMERSIATV_PLAYER_ROOT)/sources
EXT_SOURCE_PATH         := $(IMMERSIATV_PLAYER_ROOT)/includes
BUILT_LIB_PATH = $(IMMERSIATV_PLAYER_ROOT)/Android/gst-android-build/arm64-v8a

ifndef GSTREAMER_ROOT_ANDROID
$(error GSTREAMER_ROOT_ANDROID is not defined!)
endif

GSTREAMER_LIB_ROOT           := $(GSTREAMER_ROOT_ANDROID)/arm64
GSTREAMER_PROJECT_PATH := $(abspath $(LOCAL_PATH)/../GStreamer)
RENDERER_PROJECT_PATH := $(abspath $(LOCAL_PATH)/../Renderer)

GST_INCLUDE := $(GSTREAMER_LIB_ROOT)/include/gstreamer-1.0
GST_PLUGIN_INCLUDE := $(GSTREAMER_LIB_ROOT)/lib/gstreamer-1.0/include
GLIB_INCLUDE := $(GSTREAMER_LIB_ROOT)/include/glib-2.0
GLIB_CONFIG_INCLUDE := $(GSTREAMER_LIB_ROOT)/lib/glib-2.0/include
$(info plugin path: $(PLUGIN_SOURCE_PATH))
$(info source path: $(EXT_SOURCE_PATH))
$(info gstreamer include path: $(GST_INCLUDE))
$(info glib include path: $(GLIB_INCLUDE))
$(info glib config include path: $(GLIB_CONFIG_INCLUDE))



LOCAL_MODULE            := GStreamerUnityPlugin
LOCAL_C_INCLUDES        := $(PLUGIN_SOURCE_PATH) $(EXT_SOURCE_PATH) $(GST_INCLUDE) $(GST_PLUGIN_INCLUDE) $(GLIB_INCLUDE) $(GLIB_CONFIG_INCLUDE) $(BUILT_LIB_PATH)
LOCAL_SRC_FILES         :=  $(PLUGIN_SOURCE_PATH)/AudioAppSinkHandler.cpp\
							$(PLUGIN_SOURCE_PATH)/CMyListener.cpp\
							$(PLUGIN_SOURCE_PATH)/CMySink.cpp\
							$(PLUGIN_SOURCE_PATH)/CMySrc.cpp\
							$(PLUGIN_SOURCE_PATH)/CoreAPI.cpp\
							$(PLUGIN_SOURCE_PATH)/CustomAudioGrabber.cpp\
							$(PLUGIN_SOURCE_PATH)/GStreamerCore.cpp\
							$(PLUGIN_SOURCE_PATH)/GstAppNetAudioStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstAudioSampler.cpp\
							$(PLUGIN_SOURCE_PATH)/GstCustomDataStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstCustomDataPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstCustomVideoPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstCustomVideoStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstNetworkAudioPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstNetworkAudioStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstNetworkMultipleVideoPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstNetworkVideoPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstNetworkVideoStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/GstPipelineHandler.cpp\
							$(PLUGIN_SOURCE_PATH)/GstPlayerBin.cpp\
							$(PLUGIN_SOURCE_PATH)/IAppSinkHandler.cpp\
							$(PLUGIN_SOURCE_PATH)/IGStreamerPlayer.cpp\
							$(PLUGIN_SOURCE_PATH)/IGStreamerStreamer.cpp\
							$(PLUGIN_SOURCE_PATH)/NetworkAudioGrabber.cpp\
							$(PLUGIN_SOURCE_PATH)/PlayersAPI.cpp\
							$(PLUGIN_SOURCE_PATH)/StreamersAPI.cpp\
							$(PLUGIN_SOURCE_PATH)/UnityHelpers.cpp\
							$(PLUGIN_SOURCE_PATH)/UnityImageGrabber.cpp\
							$(PLUGIN_SOURCE_PATH)/VideoAppSinkHandler.cpp\
							$(PLUGIN_SOURCE_PATH)/rtp.cpp\
							$(EXT_SOURCE_PATH)/PixelUtil.cpp\
							$(EXT_SOURCE_PATH)/NetAddress.cpp\
							$(EXT_SOURCE_PATH)/IThreadManager.cpp\
							$(EXT_SOURCE_PATH)/ImageInfo.cpp\
							$(EXT_SOURCE_PATH)/RenderAPI.cpp\
							$(EXT_SOURCE_PATH)/UnityGraphicsDevice.cpp\
							$(EXT_SOURCE_PATH)/RenderAPI_OpenGLCoreES.cpp\
							$(EXT_SOURCE_PATH)/RenderAPI_OpenGL2.cpp\
							#$(EXT_SOURCE_PATH)/Android/AndroidMutex.cpp \
							#$(EXT_SOURCE_PATH)/Android/AndroidThreadManager.cpp\
							#$(EXT_SOURCE_PATH)/Android/AndroidThread.cpp\
							#$(EXT_SOURCE_PATH)/RenderAPI_Vulkan.cpp
							
							
							

LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/third_party/vulkan/src/include
LOCAL_CPPFLAGS += -DSUPPORT_VULKAN=1
LOCAL_CPPFLAGS += -DSUPPORT_OPENGL_ES=1
LOCAL_SHARED_LIBRARIES  := gstreamer_android libRenderUnityPlugin
#LOCAL_LDLIBS            := -llog -lGLESv2
LOCAL_LDLIBS            := -llog -lGLESv3 -landroid 
include $(BUILD_SHARED_LIBRARY)


#include $(CLEAR_VARS)
include $(GSTREAMER_PROJECT_PATH)/jni/Android.mk
include $(RENDERER_PROJECT_PATH)/jni/Android.mk