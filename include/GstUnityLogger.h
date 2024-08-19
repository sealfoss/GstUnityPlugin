#include "GstUnityPlugin.h"



#define DEBUG_LEVELS {"NONE", "ERROR", "WARNING", "FIXME", "INFO", "DEBUG", \
"LOG", "TRACE", "MEMDUMP"}
#define NUM_DEBUG 9


class GstUnityLogger
{
public:

GstUnityLogger(int gstDebugLevel);

~GstUnityLogger();

static GstUnityLogger* Instance();

static void LogMsg(std::string msg, UnityLogType logType);

static void LogGst(GstDebugCategory* category, GstDebugLevel level,
    const gchar* file, const gchar* function, gint line, GObject* object,
    GstDebugMessage* message, gpointer userData);

static std::string Verbosity();

bool Initialized();

private:
    int _gstDebugLevel;

    std::string _gstDebugLevelName;

    void PrintLog(std::string msg, UnityLogType logType);

    std::string _lastLog;
};