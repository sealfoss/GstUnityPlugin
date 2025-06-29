#Build tested to work with gstreamer 1.24 and android ndk 25.
#Requires environment variable for ANDROID_NDK_ROOT.
#You can set here below.

DEFAULT_NDK_PATH=$HOME/Android/Sdk/ndk/25.2.9519653

if [[ -z "${ANDROID_NDK_ROOT}" ]]; then
    echo No value set for ANDROID_NDK_ROOT, setting to script default.
    export ANDROID_NDK_ROOT=$DEFAULT_NDK_PATH
fi

$ANDROID_NDK_ROOT/ndk-build V=1 NDK_PROJECT_PATH=. \
NDK_APPLICATION_MK=jni/Application.mk clean