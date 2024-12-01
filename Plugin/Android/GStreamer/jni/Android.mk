LOCAL_PATH := $(call my-dir)
GSTREAMER_ROOT           := $(GSTREAMER_ROOT_ANDROID)/arm64
$(warning GSTREAMER_ROOT set to $(GSTREAMER_ROOT))
GSTREAMER_NDK_BUILD_PATH := $(GSTREAMER_ROOT)/share/gst-android/ndk-build/
include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk

GSTREAMER_PLUGINS    := $(GSTREAMER_PLUGINS_CORE) \
                        $(GSTREAMER_PLUGINS_PLAYBACK) \
                        $(GSTREAMER_PLUGINS_CODECS) \
                        $(GSTREAMER_PLUGINS_NET) \
                        $(GSTREAMER_PLUGINS_SYS) \
                        $(GSTREAMER_PLUGINS_EFFECTS) \
                        $(GSTREAMER_PLUGINS_CAPTURE) \
                        $(GSTREAMER_PLUGINS_ENCODING) \
                        $(GSTREAMER_PLUGINS_CODECS_RESTRICTED)

G_IO_MODULES                :=  openssl

GSTREAMER_EXTRA_DEPS        :=  gstreamer-video-1.0 \
                                gstreamer-app-1.0 \
                                gstreamer-audio-1.0 \
                                gstreamer-1.0 \
                                gstreamer-base-1.0 \
                                gstreamer-plugins-base-1.0 \
                                gstreamer-plugins-bad-1.0 \
                                gstreamer-pbutils-1.0 \
                                gstreamer-gl-1.0 \
                                gstreamer-gl-prototypes-1.0 \
                                gstreamer-rtp-1.0 \
                                gstreamer-codecparsers-1.0 \
                                gstreamer-vulkan-1.0 \
                                gstreamer-transcoder-1.0 \

GSTREAMER_EXTRA_LIBS        :=  -liconv -lglib-2.0 -lgio-2.0 -lgobject-2.0 \
-lgmodule-2.0 -lffi -lorc-0.4 -lpcre2-8 -lintl -lgthread-2.0

include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk
