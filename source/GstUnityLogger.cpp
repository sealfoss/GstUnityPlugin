#include "GstUnityLogger.h"

using namespace std;

static GstUnityLogger* s_GstUnityLogger = NULL;

GstUnityLogger::GstUnityLogger(int gstDebugLevel)
{
    string debugLevels[NUM_DEBUG] = DEBUG_LEVELS;
    string verbosity = to_string(gstDebugLevel);
    string msg;
    
    _lastLog = "";
    s_GstUnityLogger = this;
    _gstDebugLevel = gstDebugLevel;
    _gstDebugLevelName = debugLevels[gstDebugLevel];
    gst_debug_set_default_threshold((GstDebugLevel)gstDebugLevel);
    gst_debug_add_log_function(LogGst, NULL, NULL);
    msg = "GstUnityLogger Initialized at log level " + verbosity + "!";
    LogMsg(msg, kUnityLogTypeLog);
}

GstUnityLogger::~GstUnityLogger() 
{ 
    s_GstUnityLogger = NULL;
}

GstUnityLogger* GstUnityLogger::Instance()
{
    return s_GstUnityLogger;
}

bool GstUnityLogger::Initialized()
{
    return s_GstUnityLogger == this;
}

void GstUnityLogger::LogMsg(string msg, UnityLogType logType)
{
    Instance()->PrintLog(msg, logType);
}

void G_GNUC_NO_INSTRUMENT GstUnityLogger::LogGst(GstDebugCategory* category, 
    GstDebugLevel level, const gchar* file, const gchar* function, 
    gint line, GObject* object, GstDebugMessage* message, gpointer userData)
{
    const gchar *levelStr = gst_debug_level_get_name(level);
    const gchar *messageStr = gst_debug_message_get(message);
    const gchar *stackTrace = NULL;
    string msg = "GStreamer Log: " + string(messageStr);

    if(level >= 4)
        Instance()->PrintLog(msg, kUnityLogTypeLog);
    else if(level >= 2)
        Instance()->PrintLog(msg, kUnityLogTypeWarning);
    else
    {
        stackTrace = gst_debug_get_stack_trace(GST_STACK_TRACE_SHOW_FULL);
        
        if(stackTrace)
            msg += "\nTrace:\n" + string(stackTrace);

        Instance()->PrintLog(msg, kUnityLogTypeError);
    }
}

void GstUnityLogger::PrintLog(string msg, UnityLogType logType)
{
    const char* str = NULL;

    if (msg.compare(_lastLog))
    {
        str = msg.c_str();

        if (logType == UnityLogType::kUnityLogTypeLog)
            UNITY_LOG(UnityLogInterface(), str);
        else if (logType == UnityLogType::kUnityLogTypeWarning)
            UNITY_LOG_WARNING(UnityLogInterface(), str);
        else
            UNITY_LOG_ERROR(UnityLogInterface(), str);
        
        _lastLog = msg;
    }
}

string GstUnityLogger::Verbosity()
{
    return Instance()->_gstDebugLevelName;
}