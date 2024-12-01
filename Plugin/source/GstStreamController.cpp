#include <utility>
#include <vector>
#include <cstring> 
#include "GstStreamController.h"


using namespace std;

static GstStreamController* s_GstStreamController = NULL;

GstStreamController::GstStreamController()
{
    bool success = true;
    string msg;
    int i;

    if (s_GstStreamController != NULL)
        delete s_GstStreamController;

    _err = NULL;
    LogMsg("Initializing GstUnityPlugin...", kUnityLogTypeLog);

    if(gst_init_check(0, 0, &_err))
    {
        _gstInitialized = true;

        if(s_GstStreamController != NULL && s_GstStreamController != this)
            delete s_GstStreamController;

        s_GstStreamController = this;
        msg = "GStreamer initialization succeeded! ";
        msg += "Debug threshold set to: " + LogVerbosity();
        LogMsg(msg, kUnityLogTypeLog);
    }
    else
    {
        msg = "GStreamer initialization failed! Err: " + string(_err->message);
        LogMsg(msg, kUnityLogTypeError);
        success = false;
    }

    _msgs = g_async_queue_new();

    if(!_msgs)
    {
        msg = "Unable to instantiate glib async queue!";
        LogMsg(msg, kUnityLogTypeError);
        success = false;
    }

    if(success)
        LogMsg("GstUnityPlugin initialized!", kUnityLogTypeLog);
    else
    {
        msg = "GstUnityPlugin initialization failed, shutting down.";
        LogMsg(msg, kUnityLogTypeWarning);
        ShutDownGst();
    }
}

GstStreamController::~GstStreamController()
{
    ShutDown();
    s_GstStreamController = NULL;
}

bool GstStreamController::Initialized()
{
    return s_GstStreamController == this;
}

GstStreamController* GstStreamController::Instance()
{
    return s_GstStreamController;
}

void GstStreamController::ShutDown()
{

    vector<string> keys;
    GstStreamController* instance = Instance();

    if(instance)
    {
        LogMsg("Shutting down GstUnityPlugin instance...", kUnityLogTypeLog);

        for (const pair<string, StreamData*>& entry : instance->_streams)
            keys.push_back(entry.first);

        for(string key : keys)
            instance->DestroyPipeline(key);

        if(instance->_msgs)
        {
            g_async_queue_unref(instance->_msgs);
            instance->_msgs = NULL;
        }

        LogMsg("GstUnityPlugin Shutdown complete.", kUnityLogTypeLog);
    }
}

bool GstStreamController::SetStreamState(string name, GstState state)
{
    bool success = true;
    StreamData* data = _streams[name];
    data->target = state;
    g_main_context_invoke(data->context, (GSourceFunc) ChangePipelineState, 
        (gpointer) data);
    return success;
}

gboolean GstStreamController::ChangePipelineState(gpointer userData) 
{
    StreamData* data = (StreamData*) userData;
    GstStateChangeReturn ret;
    string msg;
    UnityLogType logType;
    const char* nameStr;
    char* streamName;
    int nameLen;

    gst_element_set_state(data->bin, data->target);
    ret = gst_element_get_state(data->bin, NULL, NULL, Instance()->_timeoutNs);
    
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        nameStr = data->name.c_str();
        nameLen = strlen(nameStr) + 1;
        streamName = new char[nameLen];
        strcpy(streamName, nameStr);
        g_idle_add((GSourceFunc)OnStateChangeFailure, (gpointer)streamName);
    }
    else
    {
        data->current = data->target;
    }

    return G_SOURCE_REMOVE;
}

gboolean GstStreamController::OnStateChangeFailure(gpointer userData)
{
    char* streamName = (char*) userData;
    string name = string(streamName);
    StreamData* data = Instance()->_streams[name];
    string currentName = string(gst_element_state_get_name(data->current));
    string targetName = string(gst_element_state_get_name(data->target));
    string msg = "Failed to chage state from " + currentName;
    msg += " to " + targetName + " on stream " + name + ".";
    LogMsg(msg, kUnityLogTypeWarning);
    data->target = GstState::GST_STATE_NULL;
    delete streamName;
    return G_SOURCE_REMOVE;
}

bool GstStreamController::Play(string name)
{
    return SetStreamState(name, GST_STATE_PLAYING);
}

bool GstStreamController::Pause(string name)
{
    return SetStreamState(name, GST_STATE_PAUSED);
}

bool GstStreamController::Stop(string name)
{
    bool success = true;
    StreamData* data = _streams[name];

    if (data)
    {
        gst_element_send_event(data->bin, gst_event_new_eos());
        LogMsg("Sent EOS to stream: " + name, kUnityLogTypeLog);
    }
    else
        success = false;

    return success;
}

bool GstStreamController::DestroyPipeline(string name)
{
    StreamData* data = _streams[name];
    bool success = data;
    
    if (success)
    {
        if (data->current != GST_STATE_NULL)
            Stop(name);

        DeleteStreamData(data);
        LogMsg("Destroyed pipeline stream: " + name, kUnityLogTypeLog);
    }
    else
        LogMsg("Unable to destroy stream: " + name, kUnityLogTypeError);

    return success;
}

void GstStreamController::DeleteStreamData(StreamData* data)
{
    if (data->thread)
    {
        LogMsg("Joining thread: " + data->name, kUnityLogTypeLog);

        g_mutex_lock(data->mutex);
        if (data->loopRunning)
            CleanUpThread(data);
        g_mutex_unlock(data->mutex);

        g_mutex_clear(data->mutex);
        g_mutex_free(data->mutex);
        g_thread_join(data->thread);
        g_thread_unref(data->thread);

        Instance()->_sinks.erase(data->sink);
        Instance()->_streams.erase(data->name);
        Instance()->_eventIds.erase(data->eventId);

        LogMsg("Thread joined: " + data->name, kUnityLogTypeLog);
    }

    UnrefAndDeleteGstElement(&(data->bin));
    UnrefAndUnmapGstSample(&(data->lastSample), &(data->lastBuffer), &(data->map));

    if (data->streamTex)
    {
        delete (guint8*)data->streamTex;
        data->streamTex = NULL;
    }

    delete data;
}

void GstStreamController::DeleteUdpStreamConfigData(StreamData* data)
{
    UdpConfig* configData = (UdpConfig*)data->config;
    delete configData;
    data->config = NULL;
}

void GstStreamController::DeletePipelineStreamConfigData(StreamData* data)
{
    char* configData = (char*)data->config;
    delete configData;
    data->config = NULL;
}

bool GstStreamController::SetTimeoutSeconds(float seconds)
{
    double nanoseconds = seconds * DEFAULT_GST_TIMEOUT;
    _timeoutNs = (guint64) nanoseconds;
    return true;
}

bool GstStreamController::NewFrameAvailable(string name)
{
    bool available = false;
    StreamData* data = _streams[name];

    if (data)
        available = data->newFrame;

    return available;
}

GstFlowReturn GstStreamController::NewUdpSample(GstElement* sink, gpointer data) 
{
    GstFlowReturn flow = GST_FLOW_OK;
    StreamData* streamData = NULL;

    if (Instance() != NULL)
    {
        streamData = Instance()->_sinks[sink];
        streamData->newFrame = true;
    }
    else
        flow = GST_FLOW_EOS;

    return flow;
}

bool GstStreamController::UnrefAndDeleteGstElement(GstElement** element)
{
    bool success = false;

    if (element != NULL && *element != NULL)
    {
        gst_object_unref(*element);
        *element = NULL;
        success = true;
    }

    return success;
}

bool GstStreamController::UnrefAndDeleteGstCaps(GstCaps** caps)
{
    bool success = false;

    if (caps != NULL && *caps != NULL)
    {
        gst_caps_unref(*caps);
        *caps = NULL;
        success = true;
    }

    return success;
}

bool GstStreamController::UnrefAndUnmapGstSample(GstSample** sample, GstBuffer** buffer, GstMapInfo** map)
{
    bool success = false;

    if (sample != NULL && *sample != NULL)
    {
        if (buffer != NULL && *buffer != NULL)
        {
            if (map != NULL && *map != NULL)
                gst_buffer_unmap(*buffer, *map);

            gst_buffer_unref(*buffer);
            success = true;
        }

        gst_sample_unref(*sample);
    }
    return success;
}

StreamData* GstStreamController::CreateStreamData(string name, int width, 
    int height, int colorFormat, int fps)
{
    bool success = false;
    StreamData* data = NULL;
    string w, h, format, framerate, caps, msg;
    UnityLogType logType = kUnityLogTypeError;

    if (width > 0 && height > 0 && colorFormat > 0 && colorFormat <= 3)
    {
        data = new StreamData();
        w = ",width=" + to_string(width);
        h = ",height=" + to_string(height);
        format = ",format=";
       
        if (fps > 0)
            framerate = ",framerate=" + to_string(fps) + "/1";
        else framerate = "";

        switch (colorFormat)
        {
            case 1:
                format += "RGB";
                break;
            case 2:
                format += "RGBA";
                break;
            case 3:
                format += "NV12";
                break;
            case 4:
                format += "I420";
                break;
        }

        caps = "video/x-raw" + w + h + format + framerate;
        data->sinkCaps = gst_caps_from_string(caps.c_str());
        data->filter = gst_element_factory_make("capsfilter", "filter");
        data->sink = gst_element_factory_make("appsink", "videoSink");
        success = data->sinkCaps && data->filter && data->sink;

        if (success)
        {
            g_object_set(data->filter, "caps", data->sinkCaps, NULL);
            g_object_set(data->sink, "emit-signals", TRUE, "sync",
                FALSE, "drop", TRUE, "max-buffers", MAX_BUFFERS, NULL);
            g_signal_connect(data->sink, "new-sample", G_CALLBACK(NewPipelineSample), this);
            success = true;
        }
        else
            msg = "Could not instantiate stream data filter and sink elments.";
    }
    else
    {
        msg = "Invalid size parameters given. Width: " + to_string(width);
        msg += ", height: " + to_string(height) + ", depth: " + to_string(colorFormat);
    }

    if (success)
    {
        data->name = name;
        data->map = new GstMapInfo();
        //data->width = width;
        //data->height = height;
        data->format = (ColorFormat) colorFormat;
        msg = "Stream data object successfully created for stream " + name + ".";
        logType = kUnityLogTypeLog;
    }
    else
    {
        if (data != NULL)
        {
            UnrefAndDeleteGstCaps(&(data->sinkCaps));
            delete data;
            data = NULL;
        }

        msg = "Unable to create stream data for " + name + ". " + msg;
    }

    LogMsg(msg, logType);
    return data;
}


bool GstStreamController::CreatePipelineStream(string name, string description)
{
    bool success = false;
    StreamData* data = NULL;
    GstElement* src = NULL;
    UnityLogType logType = kUnityLogTypeError;
    string msg;

    data = new StreamData();
    data->config = new PipelineConfig{ description };
    _err = NULL;
    data->bin = gst_parse_launch(description.c_str(), &_err);

    if (data->bin)
    {
        data->sink = gst_bin_get_by_name(GST_BIN(data->bin), "videoSink");

        if (data->sink)
        {
            g_object_set(data->sink, "emit-signals", TRUE, "sync", FALSE, "drop",
                TRUE, "max-buffers", MAX_BUFFERS, "emit-signals", TRUE,  NULL);
            g_object_set(data->sink, "emit-signals", TRUE, NULL);
            g_signal_connect(data->sink, "new-sample", G_CALLBACK(NewPipelineSample), this);
            data->name = name;
            data->map = new GstMapInfo();

            if (StartStreamThread(data))
                success = true;
            else
                msg = "Could not start stream thread.";
        }
        else
            msg = "Unable to locate appsink from sink name \"videoSink\".";
    }
    else
        msg = "Could not instantiate bin.";

    if (success)
    {
        logType = kUnityLogTypeLog;
        msg = "Successfully created pipeline string " + name 
            + " from description: " + description;
    }
    else
    {
        if (data)
        {
            UnrefAndDeleteGstElement(&(data->bin));

            if (data->config)
                delete data->config;

            if (data->map)
                delete data->map;

            delete data;
        }

        msg += "Stream name: " + name + ", Description: " + description;
    }

    LogMsg(msg, logType);
    return success;
}

bool GstStreamController::CreateTestPipeline(string name, int pattern, int width, 
    int height, int format)
{
    bool success = false;
    StreamData* data = NULL;
    GstElement* src = NULL;
    UnityLogType logType = kUnityLogTypeError;
    string msg = "Unable to create Test Pipeline! ";

    data = CreateStreamData(name, width, height, format, 30);

    if (data != NULL)
    {
        src = gst_element_factory_make("videotestsrc", "source");
        data->bin = gst_pipeline_new(name.c_str());

        if (data->bin && src)
        {
            g_object_set(src, "pattern", pattern, NULL);
            gst_bin_add_many(GST_BIN(data->bin), src, data->filter, data->sink, NULL);

            if (data->bin)
            {
                if (gst_element_link_many(src, data->filter, data->sink, NULL))
                {
                    if (StartStreamThread(data))
                        success = true;
                    else
                        msg = "Could not start test stream thread.";
                }
                else
                    msg = "Could not link test stream elements.";
            }
            else
                msg = "Could not create pipeline bin from test stream elements.";
        }
        else
            msg = "Could not instantiate source and bin elements.";
    }

    if (success)
    {
        logType = kUnityLogTypeLog;
        msg = "VideoTestSrc video stream started successfully!";
    }
    else
    {
        if (data != NULL)
        {
            if(data->bin)
                UnrefAndDeleteGstElement(&(data->bin));
            else
            {
                UnrefAndDeleteGstElement(&src);
                UnrefAndDeleteGstElement(&(data->filter));
                UnrefAndDeleteGstElement(&(data->sink));
            }

            delete data;
        }

        msg = "Unable to create test video pipeline " + name + ". " + msg;
    }

    LogMsg(msg, logType);
    return success;
}

void GstStreamController::PrintPadTemplates(GstElement* element)
{
    GstElementFactory* factory;
    const GList* pads;
    const GList* pad;
    gchar* name = gst_element_get_name(element);
    string msg = "Getting templates for element " + string(name);

    g_free(name);
    factory = gst_element_get_factory(element);
    pads = gst_element_factory_get_static_pad_templates(factory);

    for (pad = pads; pad != NULL; pad = pad->next) 
    {
        GstStaticPadTemplate* pad_template = (GstStaticPadTemplate*)pad->data;
        msg += "\nPad template: " + string(pad_template->name_template);
    }

    LogMsg(msg, kUnityLogTypeLog);
}
   
bool GstStreamController::CreateUdpStream(string name, int width, int height, 
    int depth, int fps, int port, string multicastGroup, string decoderType,
    bool requestLinkDecode)
{
    bool success = false;
    string msg;
    GstElement* src = NULL;
    GstElement* srcFilter = NULL;
    GstElement* depay = NULL;
    GstElement* parse = NULL;
    GstElement* decode = NULL;
    GstElement* convert = NULL;
    GstPad* decodeSrc = NULL;
    GstPad* convertSink = NULL;
    GstCaps* srcCaps = gst_caps_from_string(UDP_CAPS);
    GCallback padAdded = G_CALLBACK(PadAdded);
    UnityLogType logType = kUnityLogTypeError;
    StreamData* data = CreateStreamData(name, width, height, depth, fps);
    string linkStep = "";

    if (data)
    {
        decoderType = decoderType.size() > 0 ? decoderType : "decodebin";
        src = gst_element_factory_make("udpsrc", "source");
        srcFilter = gst_element_factory_make("capsfilter", "source_filter");
        depay = gst_element_factory_make("rtph264depay", "depay");
        parse = gst_element_factory_make("h264parse", "parse");
        decode = gst_element_factory_make(decoderType.c_str(), "decoder");
        convert = gst_element_factory_make("videoconvert", "converter");
        data->bin = gst_pipeline_new(name.c_str());

        if (data->bin && src && srcFilter && depay && parse && convert && decode)
        {
            if (multicastGroup.size() > 0)
            {
                g_object_set(src, "port", port, "multicast-group", 
                    multicastGroup.c_str(), "auto-multicast", TRUE, NULL);
            }
            else
                g_object_set(src, "port", port, NULL);

            g_object_set(srcFilter, "caps", srcCaps, NULL);
            g_object_set(decode, "emit-signals", TRUE, NULL);
            g_signal_connect(decode, "pad-added", padAdded, NULL);
            gst_bin_add_many(GST_BIN(data->bin), src, srcFilter, depay, parse, 
                decode, convert, data->filter, data->sink, NULL);
            success = data->bin;
            success = success ? gst_element_link_filtered(src, depay, srcCaps) : false;
            linkStep = linkStep.size() == 0 && !success ? "SRC->DEPAY" : linkStep;
            success = success ? gst_element_link(depay, parse) : false;
            linkStep = linkStep.size() == 0 && !success ? "DEPAY->PARSE" : linkStep;
            success = success ? gst_element_link(parse, decode) : false;
            linkStep = linkStep.size() == 0 && !success ? "PARSE->DECODE" : linkStep;

            if (success && requestLinkDecode)
            {
                decodeSrc = gst_element_request_pad_simple(decode, "src_%u");
                convertSink = gst_element_get_static_pad(convert, "sink");

                if (decodeSrc == NULL)
                {
                    PrintPadTemplates(decode);
                    decodeSrc = gst_element_get_compatible_pad(decode, convertSink, NULL);
                }

                success = gst_pad_link(decodeSrc, convertSink) == GST_PAD_LINK_OK;
                linkStep = linkStep.size() == 0 && !success ? "DECODE->CONVERT (REQUEST)" : linkStep;
                gst_object_unref(decodeSrc);
                gst_object_unref(convertSink);
            }
            else if (success)
            {
                success = success ? gst_element_link(decode, convert) : false;
                linkStep = linkStep.size() == 0 && !success ? "DECODE->CONVERT" : linkStep;
            }

            success = success ? 
                gst_element_link_filtered(convert, data->sink, data->sinkCaps) : false;
            linkStep = linkStep.size() == 0 && !success ? "CONVERT->SINK" : linkStep;

            if (success)
            {
                if (StartStreamThread(data))
                    success = true;
                else
                    msg = "Could not start udp stream thread.";
            }
            else
                msg = "Could not link udp stream elements. Link failed at: " + linkStep;
        }
        else
        {
            msg = "Could not instantiate GStreamer elements for stream " + name + ".";

            if (!data->bin)
                msg += " Could not create pipeline bin.";

            if (!src)
                msg += " Could not create udpsrc.";

            if (!srcFilter)
                msg += " Could not create source caps filter.";

            if (!depay)
                msg += " Could not create h264 depay.";

            if (!parse)
                msg += " Could not create h264 parse.";

            if (!convert)
                msg += " Could not create video convert.";

            if (!decode)
                msg += " Could not create " + decoderType + ".";
        }
    }

    if (success)
    {
        logType = kUnityLogTypeLog;
        msg = "VideoTestSrc video stream started successfully!";
    }
    else
    {
        if (data != NULL)
        {
            if(data->bin)
                UnrefAndDeleteGstElement(&(data->bin));
            else
            {
                gst_object_unref(srcCaps);
                UnrefAndDeleteGstElement(&src);
                UnrefAndDeleteGstElement(&srcFilter);
                UnrefAndDeleteGstElement(&depay);
                UnrefAndDeleteGstElement(&parse);
                UnrefAndDeleteGstElement(&decode);
                UnrefAndDeleteGstElement(&convert);
                UnrefAndDeleteGstElement(&(data->filter));
                UnrefAndDeleteGstElement(&(data->sink));
            }

            delete data;
        }

        msg = "Unable to create udp pipeline " + name + ". " + msg;
    }

    LogMsg(msg, logType);
    return success;
}

GstFlowReturn GstStreamController::NewPipelineSample(GstElement *sink, 
    gpointer data)
{
    GstFlowReturn flow = GST_FLOW_OK;
    StreamData* streamData = NULL;
    
    if (Instance() != NULL)
    {
        streamData = Instance()->_sinks[sink];
        streamData->newFrame = true;
    }
    else
        flow = GST_FLOW_EOS;

    return flow;
}

int GstStreamController::GetEventId(string name)
{
    StreamData* data = _streams[name];
    return data ? data->eventId : -1;
}

bool GstStreamController::StartStreamThread(StreamData* data)
{
    string msg;

    data->thread = g_thread_new(data->name.c_str(), 
        StreamRoutine, (gpointer)data);

    if(data->thread)
    {
        data->eventId = ++_eventIdMaster;
        _streams[data->name] = data;
        _sinks[data->sink] = data;
        _eventIds[data->eventId] = data;
        msg = "Created thread for stream: ";
        msg += data->name;
        LogMsg(msg, kUnityLogTypeLog);
    }
    else
    {
        msg = "Failed to create thread for stream: ";
        msg += data->name;
        LogMsg(msg, UnityLogType::kUnityLogTypeError);
    }

    return data->thread;
}

gpointer GstStreamController::StreamRoutine(gpointer userData)
{ 
    StreamData* data = (StreamData*) userData;
    GError* err;
    ThreadMsg* message;
    guint busId;

    data->mutex = g_mutex_new();
    data->loop = g_main_loop_new(NULL, FALSE);
    data->bus = gst_element_get_bus(data->bin);
    busId = gst_bus_add_watch(data->bus, BusCall, userData);
    data->target = GST_STATE_PLAYING;
    ChangePipelineState(data);

    SendThreadMessage(
        "Starting loop on stream " + data->name + "...",
        kUnityLogTypeLog
    );

    g_mutex_lock(data->mutex);
    data->loopRunning = true;
    g_mutex_unlock(data->mutex);
   
    g_main_loop_run(data->loop);
    
    g_mutex_lock(data->mutex);
    if (data->loopRunning)
        CleanUpThread(data);
    g_mutex_unlock(data->mutex);
    
    return NULL;
}

void GstStreamController::CleanUpThread(StreamData* data)
{
    if (data->loopRunning)
    {
        g_main_loop_quit(data->loop);
        data->loopRunning = false;
    }

    if (data->bus)
    {
        gst_object_unref(data->bus);
        data->bus = NULL;
    }

    if (data->loop)
    {
        g_main_loop_unref(data->loop);
        data->loop = NULL;
    }

    if (data->bin)
    {
        gst_element_set_state(data->bin, GST_STATE_NULL);
        gst_object_unref(data->bin);
        data->bin = NULL;
    }
}

gboolean GstStreamController::BusCall(GstBus* bus, GstMessage* message, 
    gpointer userData)
{
    StreamData* data = (StreamData*) userData;
    GstMessageType messageType = GST_MESSAGE_TYPE(message);
    const gchar* prevName = NULL;
    const gchar* currName = NULL;
    gchar* streamName = NULL;
    GError *err = NULL;
    gchar *debugInfo = NULL;
    GstState old, curr, pending;
    string msg;

    switch (messageType)
    {
        case GST_MESSAGE_ERROR:
        {
            gst_message_parse_error (message, &err, &debugInfo);
            msg = "Error received from element ";
            msg += string(GST_OBJECT_NAME(message->src));
            msg += ": " + string(err->message);
            
            if(debugInfo)
                msg += "\nDebug info: " + string(debugInfo);
            
            LogMsg(msg, kUnityLogTypeError);
            g_main_loop_quit(data->loop);
            g_clear_error (&err);
            g_free (debugInfo);
            break;
        }
        case GST_MESSAGE_EOS:
        {
            g_main_loop_quit(data->loop);
            break;
        }
    }

    return TRUE;
}

void GstStreamController::SendThreadMessage(string message, UnityLogType logType)
{
    g_async_queue_push(
        Instance()->_msgs,
        new ThreadMsg {
            message,
            kUnityLogTypeLog
        }
    );
}

bool GstStreamController::ProcessThreadMessages()
{
    int count = 0;
    bool messagesReceived = false;
    ThreadMsg* message = NULL;
    int i;

    if (Initialized())
    {
        count = g_async_queue_length(_msgs);
        messagesReceived = count > 0;

        if (messagesReceived)
        {
            for (i = 0; i < count; i++)
            {
                message = (ThreadMsg*)g_async_queue_pop(_msgs);

                if (message)
                {
                    LogMsg("From Stream Thread: " + string(message->msg),
                            message->msgType);
                    delete message;
                }
            }
        }
    }

    return messagesReceived;
}

void GstStreamController::PadAdded(GstElement* src, GstPad* newPad, 
    gpointer data)
{
    string msg, newPadName, sourceName, sinkName, sink;
    GstCaps* caps = NULL;
    GstPad* sinkPad = NULL;
    GstElement* linked = NULL;
    gchar* capsStr = NULL;
    UnityLogType logType = kUnityLogTypeLog;

    newPadName = GST_PAD_NAME(newPad);
    sourceName = GST_ELEMENT_NAME(src);
    msg = "Received new pad: " + newPadName;
    msg += ", Source Element " + sourceName;
    caps = gst_pad_query_caps(newPad, NULL);
    capsStr = gst_caps_to_string(caps);
    msg += ", Pad caps: " + string(capsStr);
    sinkPad = gst_pad_get_peer(newPad);

    if (sinkPad)
    {
        linked = gst_pad_get_parent_element(sinkPad);
        sinkName = GST_ELEMENT_NAME(linked);
        msg += ", Linked Element: " + sinkName + ".";
        gst_object_unref(sinkPad);
        gst_object_unref(linked);
    }
    else
    {
        msg += ", Linked Element Unidentified!";
        logType = kUnityLogTypeWarning;
    }
    
    gst_caps_unref(caps);
    g_free(capsStr);
    LogMsg(msg, logType);
}

bool GstStreamController::SetUnityTexture(string name, void* unityTexture, 
    int width, int height, int format)
{
    bool success = false;
    StreamData* data = _streams[name];
    string msg = data ? "Unable to set Unity texture, bad dims."
        : "Unable to set Unity Texture, no data by name " + name;
   
    UnityLogType logType;

    if (data)
    {
        data->unityTex = unityTexture;
        success = data->width == width
            && data->height == height
            && data->format == (ColorFormat)format;
    }

    msg = success ? "Set Unity texture on stream " + name : msg;
    logType = success ? kUnityLogTypeLog : kUnityLogTypeError;
    LogMsg(msg, logType);
    return success;
}

void* GstStreamController::GetLastFrameData(StreamData* data)
{
    bool success = false;
    GstElement* sink = data->sink;
    GstSample* sample = NULL;
    void* frameData = NULL;
    string msg;

    g_mutex_lock(data->mutex);
    sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
    g_mutex_unlock(data->mutex);

    if (sample)
    {
        UnrefAndUnmapGstSample(&(data->lastSample), &(data->lastBuffer), &(data->map));
        data->lastSample = sample;
        data->lastBuffer = gst_sample_get_buffer(sample);

        if (data->lastBuffer)
        {
            gst_buffer_ref(data->lastBuffer);
            gst_buffer_map(data->lastBuffer, data->map, GST_MAP_READ);
            frameData = (void*)data->map->data;
        }
        else
        {
            msg = "Unable to get buffer from sample!";
            LogMsg(msg, kUnityLogTypeWarning);
        }

        data->newFrame = false;
    }
    else
    {
        msg = "Unable to pull sample!";
        LogMsg(msg, kUnityLogTypeWarning);
    }

    return frameData;
}

void GstStreamController::GetStreamWidthAndHeightFromSample(StreamData *data, 
    GstSample *sample)
{
    GstStructure *structure;
    GstCaps *caps;
    const gchar* format;

    caps = gst_sample_get_caps(sample);
    structure = gst_caps_get_structure(caps, 0);
    gst_structure_get_int(structure, "width", &(data->width));
    gst_structure_get_int(structure, "height", &(data->height));
    format = gst_structure_get_string(structure, "format");
    data->halfWidth = data->width / 2;
    data->halfHeight = data->height / 2;
    data->numPix = data->width * data->height;

    if (format)
    {
        data->formatName = string(format);

        if (!data->formatName.compare("RGB"))
            data->format = COLOR_RGB;
        else if (!data->formatName.compare("RGBA"))
            data->format = COLOR_RGBA;
        else if (!data->formatName.compare("NV12"))
            data->format = COLOR_NV12;
        else if (!data->formatName.compare("I420"))
            data->format = COLOR_I420;
        else
            data->format = COLOR_NONE;
    }
}

void GstStreamController::GetStreamWidthAndHeight(StreamData* data)
{
    GstSample* sample = NULL;

    g_mutex_lock(data->mutex);
    sample = gst_app_sink_pull_sample(GST_APP_SINK(data->sink));
    g_mutex_unlock(data->mutex);

    if (sample)
    {
        GetStreamWidthAndHeightFromSample(data, sample);
        gst_sample_unref(sample);
    }
}

int GstStreamController::GetStreamWidth(string name)
{
    StreamData* data = _streams[name];
    int width = -1;

    if (data)
    {
        if (data->width == -1 && data->newFrame)
            GetStreamWidthAndHeight(data);

        width = data->width;
    }

    return width;
}

int GstStreamController::GetStreamFormat(string name)
{
    StreamData* data = _streams[name];
    int format = -1;

    if (data)
    {
        if (data->format == COLOR_NONE && data->newFrame)
            GetStreamWidthAndHeight(data);

        format = (int)data->format;
    }

    return format;
}

int GstStreamController::GetStreamHeight(string name)
{
    StreamData* data = _streams[name];
    int height = -1;

    if (data)
    {
        if (data->height == -1 && data->newFrame)
            GetStreamWidthAndHeight(data);

        height = data->height;
    }

    return height;
}

bool GstStreamController::StreamPlaying(string name)
{
    StreamData* data = _streams[name];
    return data ? data->current == GST_STATE_PLAYING : false;
}

StreamData* GstStreamController::StreamFromEventId(int eventId)
{
    return _eventIds[eventId];
}

StreamData* GstStreamController::StreamFromName(string name)
{
    return _streams[name];
}

void GstStreamController::ListDecoders()
{
    string decoderList;
    GList* decoders = gst_element_factory_list_get_elements(
        GST_ELEMENT_FACTORY_TYPE_DECODABLE, GST_RANK_MARGINAL
    );

    for (GList* iter = decoders; iter != NULL; iter = iter->next) 
    {
        GstElementFactory* factory = (GstElementFactory*)iter->data;
        const gchar* name = gst_element_get_name(factory);
        decoderList += string(name) + "\n";
    }

    LogMsg("GStreamer decoder list:\n" + decoderList, kUnityLogTypeLog);
}

void GstStreamController::ListFeatures(string elementName)
{
    GList* featureList = NULL;
    GList* iter = NULL;
    string templatePresense, templateDirection, templateName, templateCaps;
    string msg, gstElementName, elementKlass, propName, propBlurb;
    string features;
    GstRegistry* registry = gst_registry_get();
    const GList* padTemplates;
    GstStaticPadTemplate* padTemplate = NULL;
    GstElement* element = NULL;
    GstElementFactory* factory = NULL;
    GstCaps* caps = NULL;
    gchar* capsStr = NULL;
    GParamSpec** properties = NULL;
    guint nProps, i;
    GstPluginFeature* feature = 
        gst_registry_lookup_feature(registry, elementName.c_str());

    if (feature)
    {
        msg = "Listing props and caps for element: " + elementName;
        LogMsg(msg, kUnityLogTypeLog);
        factory = GST_ELEMENT_FACTORY(feature);
        gstElementName = string(gst_element_factory_get_longname(factory));
        elementKlass = string(gst_element_factory_get_klass(factory));
        element = gst_element_factory_make(elementName.c_str(), "temp");
        properties = NULL;
        msg = "Element: " + gstElementName + "(" + elementKlass + ")";
        msg += "\nCapabilities: ";
        padTemplates = gst_element_factory_get_static_pad_templates(factory);
        LogMsg(msg, kUnityLogTypeLog);

        // Iterate through pad templates and print information
        for (const GList* padIter = padTemplates; padIter != NULL; padIter = padIter->next) 
        {
            padTemplate = (GstStaticPadTemplate*)padIter->data;
            templateName = string(padTemplate->name_template);

            switch(padTemplate->direction)
            {
                case GST_PAD_SRC:
                    templateDirection = "SRC"; 
                    break;
                case GST_PAD_SINK:
                    templateDirection = "SINK"; 
                    break;
                case GST_PAD_UNKNOWN:
                    templateDirection = "UNKNOWN"; 
                    break;
                default:
                    templateDirection = "NONE"; 
            }

            switch(padTemplate->presence)
            {
                case GST_PAD_ALWAYS:
                    templatePresense = "Always"; 
                    break;
                case GST_PAD_SOMETIMES:
                    templatePresense = "Sometimes"; 
                    break;
                case GST_PAD_REQUEST:
                    templatePresense = "Request"; 
                    break;
                default:
                    templatePresense = "NONE"; 
            }

            msg = "Template: " + templateName;
            msg += "\n\tDirection: " + templateDirection; 
            msg += "\n\tPresense: " + templatePresense;

            // Get and print the capabilities
            caps = NULL;
            capsStr = NULL;
            caps = gst_static_pad_template_get_caps(padTemplate);

            if(caps)
            {
                capsStr = gst_caps_to_string(caps);

                if(capsStr)
                {
                    templateCaps = "\n\tCapabilities:\n\t\t" + string(capsStr);
                    msg += templateCaps;
                    LogMsg(msg, kUnityLogTypeLog);
                    g_free(capsStr);

                }

                gst_caps_unref(caps);
            }
            else
                LogMsg(msg, kUnityLogTypeLog);
        }

        LogMsg(msg, kUnityLogTypeLog);
        msg = "\tElement: " + gstElementName + "(" + elementKlass + ")";
        msg += "\n\tProperties: ";

        if (element)
        {
            LogMsg(msg, kUnityLogTypeLog);

            properties = g_object_class_list_properties(
                G_OBJECT_GET_CLASS(element), &nProps
            );

            if(properties)
            {
                for (i = 0; i < nProps; i++)
                {
                    propName = string(g_param_spec_get_name(properties[i]));
                    propBlurb = string(g_param_spec_get_blurb(properties[i]));
                    msg = propName + "(" + propBlurb + ")\n";
                    LogMsg(msg, kUnityLogTypeLog);
                }

                g_free(properties);
            }
            else
                msg+= "NONE FOUND";

            gst_object_unref(element);
        }
        else
        {
            msg += "Could not instantiate temp " + gstElementName;
            LogMsg(msg, kUnityLogTypeLog);
        }
    }
    else
    {
        msg = "Unable to list properties for feature: " + elementName;
        LogMsg(msg, kUnityLogTypeWarning);
    }
}