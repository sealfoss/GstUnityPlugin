#if UNITY_ANDROID
#include <cstdlib>
#elif UNITY_WIN
#include <stdlib.h>
#endif

#include "GstUnityPlugin.h"
#include "GstStreamController.h"
#include "UnityTextureController.h"
#include "GstUnityLogger.h"

using namespace std;


static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;
static UnityGfxRenderer s_RendererType = kUnityGfxRendererNull;
static IUnityLog* s_UnityLog = NULL;


IUnityInterfaces* UnityInterfaces()
{
    return s_UnityInterfaces;
}

IUnityLog* UnityLogInterface()
{
    return s_UnityLog;
}

IUnityGraphics* UnityGraphicsInterface()
{
    return s_Graphics;
}

void LogMsg(string msg, UnityLogType logType)
{
    GstUnityLogger::LogMsg(msg, logType);
}

string LogVerbosity()
{
    return GstUnityLogger::Verbosity();
}

extern "C" 
{
    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API 
    UnityPluginLoad(IUnityInterfaces* unityInterfaces)
    {
        s_UnityInterfaces = unityInterfaces;
        s_UnityLog = unityInterfaces->Get<IUnityLog>();
        s_Graphics = unityInterfaces->Get<IUnityGraphics>();
        s_Graphics->
            RegisterDeviceEventCallback(UnityTextureController::OnGraphicsDeviceEvent);

#if SUPPORT_VULKAN
        if (s_Graphics->GetRenderer() == kUnityGfxRendererNull)
            VkTextureController_OnPluginLoad(unityInterfaces);
#endif

        UnityTextureController::OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API UnityPluginUnload()
    {
        ShutDownGst();
        s_Graphics->
            UnregisterDeviceEventCallback(UnityTextureController::OnGraphicsDeviceEvent);
        s_Graphics = NULL;
        s_UnityLog = NULL;
        s_UnityInterfaces = NULL;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API Initialized()
    {
        return GstStreamController::Instance() != NULL;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API  InitLog(int gstDebugLevel)
    {
        GstUnityLogger* logger = new GstUnityLogger(gstDebugLevel);
        return logger->Initialized();
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API InitGst()
    {
        bool success = false;
        GstStreamController* controller = new GstStreamController();
        string texName;
        string msg;

        if(!controller->Initialized())
            ShutDownGst();
        else
        {
            texName = UnityTextureController::Instance()->GetTypeName();
            msg = "GStreamer initialized for Unity! ";
            msg += "Texture Controller Type: " + texName;
            LogMsg(msg, kUnityLogTypeLog);
            success = true;
        }

        return success;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    UdpStream(const char* streamName, int width, int height, int depth,
        int fps, int port, const char* multicastGroup,
        const char* decoderType, bool requestLinkDecode)
    {
        string name(streamName);
        string multicast(multicastGroup);
        string decoder(decoderType);

        return GstStreamController::Instance()->CreateUdpStream(
            name, width, height, depth, fps, port, 
            multicast, decoder, requestLinkDecode
        );
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    PipelineStream(const char* streamName, const char* description)
    {
        return GstStreamController::Instance()->
            CreatePipelineStream(string(streamName), string(description));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API ShutDownGst()
    {
        GstStreamController* instance = GstStreamController::Instance();


        if (instance)
            delete instance;

        return GstStreamController::Instance() == NULL;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    SetUnityTexture(void* tex, int id, const char* streamName)
    {
        string msg;
        string name(streamName);
        UnityLogType logType = kUnityLogTypeLog;
        StreamData* data = GstStreamController::Instance() ? 
            GstStreamController::Instance()->StreamFromName(name) : NULL;
        bool success = data;

        if(success)
        {
            switch(id)
            {
                case 0:
                    data->unityTex = tex;
                    msg = "Set Unity RGB texture for stream " + name;
                    break;
                case 1:
                    data->yTex = tex;
                    msg = "Set Unity Y-component texture for stream " + name;
                    break;
                case 2:
                    data->uvTex = tex;
                    msg = "Set Unity UV-component texture for stream " + name;
                    break;
                case 3:
                    data->uTex = tex;
                    msg = "Set Unity U-component texture for stream " + name;
                    break;
                case 4:
                    data->vTex = tex;
                    msg = "Set Unity V-component texture for stream " + name;
                    break;
                default:
                    msg = "Tried to set Unity texture on invalid id for stream " + name;
                    logType = kUnityLogTypeError;
                    success = false;
                    break;
            }

            LogMsg(msg, logType);
        }

        return success;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    NewFrameAvailable(const char* streamName)
    {
        GstStreamController* instance = GstStreamController::Instance();
        return instance ? instance->NewFrameAvailable(string(streamName)) : false;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API 
    PlayPipeline(const char* streamName)
    {
        return GstStreamController::Instance()->Play(string(streamName));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    PausePipeline(const char* streamName)
    {
        return GstStreamController::Instance()->Pause(string(streamName));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    StopPipeline(const char* streamName)
    {
        return GstStreamController::Instance()->Stop(string(streamName));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    SetGstOpTimeoutSeconds(float seconds)
    {
        return GstStreamController::Instance()->
            SetTimeoutSeconds(seconds);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    RemovePipeline(const char* streamName)
    {
        return GstStreamController::Instance()->
            DestroyPipeline(string(streamName));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    LogThreadMessages()
    {
        return Initialized() ? GstStreamController::Instance()->
            ProcessThreadMessages() : false;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamWidth(const char* streamName)
    {
        return GstStreamController::Instance()->
            GetStreamWidth(string(streamName));
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamHeight(const char* streamName)
    {
        return GstStreamController::Instance()->
            GetStreamHeight(string(streamName));
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    StreamColorFormat(const char* streamName)
    {
        return GstStreamController::Instance()->
            GetStreamFormat(string(streamName));
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    StreamPlaying(const char* streamName)
    {
        GstStreamController* instance = GstStreamController::Instance();
        return instance ? 
            instance->StreamPlaying(string(streamName)) : false;
    }

    UNITY_INTERFACE_EXPORT int UNITY_INTERFACE_API
    GetEventId(const char* streamName)
    {
        return GstStreamController::Instance()->
            GetEventId(string(streamName));
    }

    UNITY_INTERFACE_EXPORT 
    UnityRenderingEvent UNITY_INTERFACE_API GetRenderEvent()
    {
        return SendFrameToTexture;
    }

    UNITY_INTERFACE_EXPORT
    TextureCreationEvent UNITY_INTERFACE_API GetTextureCreationEvent()
    {
        return CreateTexture;
    }

    UNITY_INTERFACE_EXPORT void* UNITY_INTERFACE_API
    CreateTexture(int w, int h, int d)
    {
        bool success = true;
        void* tex = NULL;
        UnityTextureController* controller
            = UnityTextureController::Instance();

        if (controller)
            tex = controller->CreateTexture(w, h, d);

        return tex;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    DeleteTexture(void* tex)
    {
        bool success = false;
        UnityTextureController* controller
            = UnityTextureController::Instance();

        if (controller)
            success = controller->DeleteTexture(tex);

        return success;
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    SetEnvVar(const char* var, const char* val)
    {
        bool success = false;
        int result = 0;
        string msg;

#if UNITY_ANDROID
        result = setenv(var, val, 1);
#elif UNITY_WIN
        msg = string(var) + "=" + string(val);
        result = putenv(msg.c_str());
#endif

        if (result == 0)
        {
            msg = "Set Env Var " + string(var) + " to " + string(val) + ".";
            LogMsg(msg, kUnityLogTypeLog);
            success = true;
        }
        else
        {
            msg = "Unable to set value of env var " + string(var) + ".";
            LogMsg(msg, kUnityLogTypeWarning);
        }

        return success;
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API ListDecoders()
    {
        GstStreamController::Instance()->ListDecoders();
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API
    ListFeatures(const char* elementName)
    {
        string name(elementName);
        GstStreamController::Instance()->ListFeatures(name);
    }

    UNITY_INTERFACE_EXPORT bool UNITY_INTERFACE_API
    TestPipeline(const char* streamName, int pattern, int width, int height, int format)
    {
        return GstStreamController::Instance()->CreateTestPipeline(
            string(streamName), pattern, width, height, format
        );
    }
}

static void UNITY_INTERFACE_API SendFrameToTexture(int eventId)
{
    
    GstStreamController* instance = GstStreamController::Instance();
    StreamData* data = instance ?
        instance->StreamFromEventId(eventId) : NULL;
    void* lastFrame = data ? 
        GstStreamController::GetLastFrameData(data) : NULL;
    unsigned char* texPtr = NULL;
    string msg;

    if (lastFrame)
    {
        switch(data->format)
        {
            case COLOR_RGB:
                if(data->unityTex)
                    UnityTextureController::Instance()->SetTextureData(
                        data->eventId, data->unityTex, data->width, data->height, 3, lastFrame
                    );
                break;
            case COLOR_RGBA:
                if (data->unityTex)
                    UnityTextureController::Instance()->SetTextureData(
                        data->eventId, data->unityTex, data->width, data->height, 4, lastFrame
                    );
                break;
            case COLOR_NV12:
                texPtr = (unsigned char*)lastFrame;
                UnityTextureController::Instance()->SetTextureData(
                    data->eventId, data->yTex, data->width, data->height, 1, (void*)texPtr
                );
                texPtr += data->numPix;
                UnityTextureController::Instance()->SetTextureData(
                    data->eventId, data->uvTex, data->halfWidth, data->halfHeight, 2, (void*)texPtr
                );
                break;
            case COLOR_I420:
                msg = "Invalid color format found on stream " + data->name + ". ";
                msg = "I420 Color format not yet implemented! Try NV12 or RGB instead.";
                LogMsg(msg, kUnityLogTypeError);
                break;
            default:
                msg = "Invalid color format found on stream " + data->name + ". ";
                msg += "Format: " + to_string((int)data->format);
                msg += "How did this even happen ???";
                LogMsg(msg, kUnityLogTypeError);
                break;
        }
    }
}

