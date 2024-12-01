#pragma once


#include <gst/app/gstappsink.h>
#include <unordered_map>
#include "GstUnityPlugin.h"

#define UDP_CAPS "application/x-rtp,media=video,payload=96,clock-rate=90000,encoding-name=H264"
#define DEFAULT_GST_TIMEOUT 1000000000
#define MAIN_THREAD_NAME "GstUnityPlugin Main Thread"
#define MAX_BUFFERS 1
#define AMC_VAR "GST_AMC_IGNORE_UNKNOWN_COLOR_FORMATS"
#define AMC_VAL "yes"


enum ColorFormat
{
    COLOR_NONE = 0,
    COLOR_RGB = 1,
    COLOR_RGBA = 2,
    COLOR_NV12 = 3,
    COLOR_I420 = 4,
};

enum StreamType
{
    STREAM_NONE = 0,
    STREAM_PIPLINE = 1,
    STREAM_UDP = 2
};

struct ThreadMsg
{
    std::string msg;
    UnityLogType msgType;
};

struct UdpConfig
{
    int port = 0;
    std::string multicast = "";
    std::string iface = "";
};

struct PipelineConfig
{
    std::string description;
};

struct StreamData
{
    bool newFrame = false;
    bool loopRunning = false;
    bool dimsSet = false;
    int width = -1;
    int height = -1;
    int halfWidth = -1;
    int halfHeight = -1;
    int numPix = -1;
    ColorFormat format = COLOR_NONE;
    int eventId = -1;
    StreamType type = StreamType::STREAM_NONE;
    GMutex* mutex;
    GstElement* bin = NULL;
    GstElement* filter = NULL;
    GstElement* sink = NULL;
    void* unityTex = NULL;
    void* yTex = NULL;
    void* uvTex = NULL;
    void* uTex = NULL;
    void* vTex = NULL;
    void* streamTex = NULL;
    void* config = NULL;
    GThread* thread = NULL;
    GstBus * bus = NULL;
    GstMapInfo* map = NULL;
    GstBuffer* lastBuffer = NULL;
    GstSample* lastSample = NULL;
    GMainContext* context = NULL;
    GMainLoop* loop = NULL;
    std::string name = "";
    std::string formatName = "";
    GstAppSinkCallbacks callbacks;
    GstState target = GST_STATE_NULL;
    GstState current = GST_STATE_NULL;
    GstCaps* sinkCaps;
};


class GstStreamController
{
public:

    GstStreamController();

    ~GstStreamController();

    bool Initialized();

    static GstStreamController* Instance();

    static void ShutDown();

    static void* GetLastFrameData(StreamData* data);

    static void HandleBusMessage();
    
    bool Play(std::string name);

    bool Pause(std::string name);

    bool Stop(std::string name);

    bool NewFrameAvailable(std::string name);

    bool SetUnityTexture(std::string name, void* unityTexture, int width, 
        int height, int depth);

    bool SetTimeoutSeconds(float seconds);

    bool CreatePipelineStream(std::string name, std::string description);

    bool CreateUdpStream(std::string name, int width, int height, int depth,
        int fps, int port, std::string multicastGroup, std::string decoderType,
        bool requestLinkDecode);

    bool ProcessThreadMessages();

    bool DestroyPipeline(std::string name);

    GMainLoop* GetMainLoop();

    int GetStreamWidth(std::string name);

    int GetStreamHeight(std::string name);

    int GetStreamFormat(std::string name);

    bool StreamPlaying(std::string name);

    int GetEventId(std::string name);

    StreamData* StreamFromEventId(int eventId);

    StreamData* StreamFromName(std::string name);

    void ListDecoders();

    void ListFeatures(std::string elementName);

    bool CreateTestPipeline(std::string name, int pattern, int width, 
        int height, int format);

protected:

// Functions

    static inline void DeleteUdpStreamConfigData(StreamData* data);
    
    static inline void DeletePipelineStreamConfigData(StreamData* data);
    
    static inline void DeleteStreamData(StreamData* data);

    static gpointer StreamRoutine(gpointer data);

    static gboolean BusCall(GstBus* bus, GstMessage* msg, gpointer data);

    static gboolean BusMsg(GstMessage *msg);

    static gboolean OnStateChangeFailure(gpointer userData);

    static GstFlowReturn NewUdpSample(GstElement *sink, gpointer data);

    static GstFlowReturn NewPipelineSample(GstElement *sink, gpointer data);

    static void PadAdded(GstElement* src, GstPad* newPad, gpointer data);

    static void GetStreamWidthAndHeightFromSample(StreamData *data, 
        GstSample *sample);

    static void GetStreamWidthAndHeight(StreamData* data);

    static void SendThreadMessage(std::string message, UnityLogType logType);

    static gboolean ChangePipelineState(gpointer userData);

    static void CleanUpThread(StreamData* data);

    static void PrintPadTemplates(GstElement* element);

    bool SetStreamState(std::string name, GstState state);

    bool StartStreamThread(StreamData* data);

    inline StreamData* CreateStreamData(std::string name, int width, int height, 
        int depth, int fps);

    inline static bool UnrefAndDeleteGstElement(GstElement** element);

    inline static bool UnrefAndDeleteGstCaps(GstCaps** caps);

    inline static bool UnrefAndUnmapGstSample(GstSample** sample, 
        GstBuffer** buffer, GstMapInfo** map);


// Members

    std::unordered_map<std::string, StreamData*> _streams;

    std::unordered_map<GstElement*, StreamData*> _sinks;

    std::unordered_map<int, StreamData*> _eventIds;

    guint64 _timeoutNs = DEFAULT_GST_TIMEOUT;

    GError* _err = NULL;

    GAsyncQueue* _msgs = NULL;

    int _eventIdMaster = 0;

    bool _gstInitialized = false;
};
