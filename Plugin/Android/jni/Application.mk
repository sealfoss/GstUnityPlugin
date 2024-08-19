APP_ABI := arm64-v8a
APP_PLATFORM := latest

#APP_STL := gnustl_static
APP_STL := c++_shared
#APP_STL := c++_static
APP_CPPFLAGS += -std=c++17
NDK_TOOLCHAIN_VERSION := clang
APP_CPPFLAGS += -frtti
#APP_OPTIM := debug
#APP_CPPFLAGS += -fexceptions
