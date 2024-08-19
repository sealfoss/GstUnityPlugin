cmake_minimum_required(VERSION 3.29.2)
project("GstUnityPlugin")
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
add_compile_definitions(GSTUNITYPLUGIN_EXPORTS)
set (CMAKE_CXX_STANDARD 17)

pkg_check_modules(GSTLIBS REQUIRED
        gstreamer-1.0
        gobject-2.0
        glib-2.0
        gstreamer-app-1.0
        gstreamer-audio-1.0
        gstreamer-video-1.0
        gstreamer-gl-1.0
)

INCLUDE_DIRECTORIES("${GSTLIBS_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES("./include" )

set (SOURCE ${SOURCE}   
        "source/GstUnityPlugin.cpp"
        "source/GstUnityLogger.cpp"
        "source/GstStreamController.cpp"
        "source/Dx11TextureController.cpp"
        "source/GlTextureController.cpp"
        "source/VkTextureController.cpp"
        "source/UnityTextureController.cpp"
        "source/gl3w.c"
)

link_directories("${GSTLIBS_LIBRARY_DIRS}")
add_library("${PROJECT_NAME}" SHARED "${SOURCE}")
link_directories("${GSTLIBS_LIBRARY_DIRS}")
target_link_libraries("${PROJECT_NAME}" ${GSTLIBS_LIBRARIES})
target_link_libraries("${PROJECT_NAME}" ${OPENGL_LIBRARIES})