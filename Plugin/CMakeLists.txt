cmake_minimum_required(VERSION 3.29.2)
project("GStreamerUnityPlugin")
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
add_compile_definitions(GSTREAMERUNITYPLUGIN_EXPORTS)
set (CMAKE_CXX_STANDARD 17)

pkg_check_modules(GSTLIBS REQUIRED
        gstreamer-1.0
        gobject-2.0
        glib-2.0
        gstreamer-app-1.0
        gstreamer-audio-1.0
        gstreamer-video-1.0
)


INCLUDE_DIRECTORIES("${GSTLIBS_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES("./includes" )
INCLUDE_DIRECTORIES("./sources" )

set (SOURCE ${SOURCE}   "sources/GstUnityPlugin.cpp"
			"sources/GstUnityLogger.cpp"
                        "sources/GstStreamController.cpp"
                        "sources/Dx11TextureController.cpp"
                        "sources/GlTextureController.cpp"
			"sources/VkTextureController.cpp"
                        "sources/UnityTextureController.cpp"
			"sources/gl3w.c"
)

link_directories("${GSTLIBS_LIBRARY_DIRS}")
add_library("${PROJECT_NAME}"   SHARED  "${SOURCE}")
#target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
link_directories("${GSTLIBS_LIBRARY_DIRS}")
target_link_libraries("${PROJECT_NAME}" ${GSTLIBS_LIBRARIES})
target_link_libraries("${PROJECT_NAME}" ${OPENGL_LIBRARIES})
include(CMakePrintHelpers)
cmake_print_variables(GSTLIBS_LIBRARIES)