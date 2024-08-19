#pragma once

#include <string>
#include <gst/gst.h>
#include <glib/gmessages.h>
#include "Unity/IUnityInterface.h"
#include "Unity/IUnityLog.h"
#include "Unity/IUnityGraphics.h"
#include "PlatformBase.h"


IUnityInterfaces* UnityInterfaces();
IUnityLog* UnityLogInterface();
IUnityGraphics* UnityGraphicsInterface();
void LogMsg(std::string msg, UnityLogType logType);
std::string LogVerbosity();


extern "C"
{ 
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API
    UnityPluginLoad(IUnityInterfaces* unityInterfaces);

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API
    UnityPluginUnload();

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API Initialized();

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    InitLog(int gstDebugLevel);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    InitGst();

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    UdpStream(const char* streamName, int width, int height, int depth,
        int fps, int port, const char* multicastGroup,
        const char* decoderType, bool requestLinkDecode);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    PipelineStream(const char* streamName, const char* description);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API ShutDownGst();

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    SetUnityTexture(void* tex, int id, const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    PlayPipeline(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    PausePipeline(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    StopPipeline(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    NewFrameAvailable(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    RemovePipeline(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    LogThreadMessages();

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamWidth(const char* streamName);

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamHeight(const char* streamName);

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamColorFormat(const char* streamName);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    StreamPlaying(const char* streamName);

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    GetEventId(const char* streamName);

    UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
    GetRenderEvent();

    UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API 
    GetDebugRenderEvent();

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    GetEventId(const char* streamName);

    UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API 
    CreateTexture(int w, int h, int d);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    DeleteTexture(void* tex);

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    SetEnvVar(const char* var, const char* val);

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API
    ListDecoders();

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API
    ListFeatures(const char* elementName);


    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    TestPipeline(const char* streamName, int pattern, 
        int width, int height, int format);
}

static void UNITY_INTERFACE_API SendFrameToTexture(int eventId);
