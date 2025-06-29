#Build tested to work with gstreamer 1.24 and android ndk 25.
#Requires environment variables for ANDROID_NDK_ROOT and GSTREAMER_ROOT_ANDROID.
#You can set both here below.

DEFAULT_NDK_PATH=$HOME/Android/Sdk/ndk/27.2.12479018
DEFAULT_GST_PATH=$HOME/gstreamer-android/gstreamer-1.0-android-universal-1.26.2

if [[ -z "${ANDROID_NDK_ROOT}" ]]; then
    echo No value set for ANDROID_NDK_ROOT, setting to script default...
    export ANDROID_NDK_ROOT=$DEFAULT_NDK_PATH
    echo ANDROID_NDK_ROOT set to $ANDROID_NDK_ROOT
fi

if [[ -z "${GSTREAMER_ROOT_ANDROID}" ]]; then
    echo No value set for GSTREAMER_ROOT_ANDROID, setting to script default...
    export GSTREAMER_ROOT_ANDROID=$DEFAULT_GST_PATH
    echo GSTREAMER_ROOT_ANDROID set to $GSTREAMER_ROOT_ANDROID
fi

$ANDROID_NDK_ROOT/ndk-build V=1 NDK_PROJECT_PATH=. \
NDK_APPLICATION_MK=jni/Application.mk
