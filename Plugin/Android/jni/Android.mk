LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := 	GStreamerUnityPlugin

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

UNITY_INCLUDE := $(EXT_SOURCE_PATH)/Unity

$(info plugin path: $(PLUGIN_SOURCE_PATH))
$(info source path: $(EXT_SOURCE_PATH))
$(info gstreamer include path: $(GST_INCLUDE))
$(info glib include path: $(GLIB_INCLUDE))
$(info glib config include path: $(GLIB_CONFIG_INCLUDE))

LOCAL_C_INCLUDES:= 	$(PLUGIN_SOURCE_PATH) $(EXT_SOURCE_PATH) $(GST_INCLUDE) \
					$(GST_PLUGIN_INCLUDE) $(GLIB_INCLUDE) $(BUILT_LIB_PATH) \
					$(GLIB_CONFIG_INCLUDE) $(UNITY_INCLUDE) 

LOCAL_SRC_FILES :=	$(PLUGIN_SOURCE_PATH)/GstUnityPlugin.cpp \
					$(PLUGIN_SOURCE_PATH)/GstStreamController.cpp \
					$(PLUGIN_SOURCE_PATH)/UnityTextureController.cpp \
					$(PLUGIN_SOURCE_PATH)/Dx11TextureController.cpp \
					$(PLUGIN_SOURCE_PATH)/GlTextureController.cpp \
					$(PLUGIN_SOURCE_PATH)/VkTextureController.cpp \
					$(PLUGIN_SOURCE_PATH)/GstUnityLogger.cpp 
							

LOCAL_C_INCLUDES += $(NDK_ROOT)/sources/third_party/vulkan/src/include

LOCAL_CPPFLAGS += -DSUPPORT_VULKAN=1

LOCAL_CPPFLAGS += -DSUPPORT_OPENGL_ES=1

LOCAL_SHARED_LIBRARIES  := gstreamer_android

LOCAL_LDLIBS	:= 	-llog \
					-lGLESv3 \
					-landroid 

include $(BUILD_SHARED_LIBRARY)
include $(GSTREAMER_PROJECT_PATH)/jni/Android.mk

