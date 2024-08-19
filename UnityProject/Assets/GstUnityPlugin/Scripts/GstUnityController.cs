using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices;
using System;
using System.Threading;
using Unity.VisualScripting;
using UnityEngine.UI;

public class GstUnityController : MonoBehaviour
{
    private const string _dllName = "GstUnityPlugin";

    [DllImport(_dllName)]
    protected static extern bool Initialized();

    [DllImport(_dllName)]
    protected static extern bool InitGst();

    [DllImport(_dllName)]
    protected static extern bool InitLog(int gstDebugLevel);

    [DllImport(_dllName)]
    protected static extern bool UdpStream(
        [MarshalAs(UnmanagedType.LPStr)] string streamName,
        int width, int height, int depth, int fps, int port,
        [MarshalAs(UnmanagedType.LPStr)] string mulitcastGroup,
        [MarshalAs(UnmanagedType.LPStr)] string decoderType,
        bool requestLinkDecode
    );

    [DllImport(_dllName)]
    protected static extern bool PipelineStream(
        [MarshalAs(UnmanagedType.LPStr)] string streamName,
        [MarshalAs(UnmanagedType.LPStr)] string description,
        int texWidth, int texHeight, int texDepth
    );

    [DllImport(_dllName)]
    protected static extern bool ShutDownGst();

    [DllImport(_dllName)]
    protected static extern bool PlayPipeline(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern bool PausePipeline(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern bool StopPipeline(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern bool NewFrameAvailable(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    public static extern bool RemovePipeline(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern bool SendFrameToTexture(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern bool LogThreadMessages();

    [DllImport(_dllName)]
    protected static extern bool StreamPlaying(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern int StreamWidth(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern int StreamHeight(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern int StreamColorFormat(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    public bool GetStreamWidthHeightFormat(string streamName, 
        out int width, out int height, out int format)
    {
        width = StreamWidth(streamName);
        height = StreamHeight(streamName);
        format = StreamColorFormat(streamName);
        return width > 0 && height > 0 && format > 0;
    }

    [DllImport(_dllName)]
    protected static extern bool SetUnityTexture(
        System.IntPtr unityTexture,
        int id,
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern IntPtr GetRenderEvent();

    [DllImport(_dllName)]
    protected static extern int GetEventId(
        [MarshalAs(UnmanagedType.LPStr)] string streamName
    );

    [DllImport(_dllName)]
    protected static extern IntPtr GetDebugRenderEvent();

    [DllImport(_dllName)]
    protected static extern bool SetDebugTexture(
        int w, int h, int d, int r, int g, int b, int a
    );

    [DllImport(_dllName)]
    protected static extern IntPtr CreateTexture(
        int w, int h, int d
    );

    [DllImport(_dllName)]
    protected static extern bool DeleteTexture(IntPtr tex);

    [DllImport(_dllName)]
    protected static extern bool WriteDebugTextureToFile(
        [MarshalAs(UnmanagedType.LPStr)] string filepath
    );

    [DllImport(_dllName)]
    protected static extern bool CopyLastFrameToDebug(int eventId);

    [DllImport(_dllName)]
    protected static extern bool SetEnvVar(
        [MarshalAs(UnmanagedType.LPStr)] string var,
        [MarshalAs(UnmanagedType.LPStr)] string val
    );

    [DllImport(_dllName)]
    protected static extern void ListDecoders();

    [DllImport(_dllName)]
    protected static extern bool ListFeatures(
        [MarshalAs(UnmanagedType.LPStr)] string elementName
    );

    [DllImport(_dllName)]
    protected static extern bool TestPipeline(
        [MarshalAs(UnmanagedType.LPStr)] string streamName,
        int pattern, int width, int height, int format
    );

    [SerializeField]
    protected bool _gstInitialized = false;

    public bool GstInitialized { get => _gstInitialized; }

    [SerializeField]
    protected int _gstDebugLevel = 3;

    [SerializeField]
    protected bool _ignoreUnknownColorFormats = false;

    [SerializeField]
    private List<StreamController> _streams;

    protected IntPtr _renderEvent = IntPtr.Zero;
    protected int _streamEventId = 0;
    private bool _updateTexture = false;
    private bool _streamPlaying = false;
    public delegate void TextureUpdateDelegate();
    public event TextureUpdateDelegate OnTextureUpdate;
    public static GstUnityController Instance { get; private set; }

    private void Awake()
    {
        if (Instance != null && Instance != this)
            Destroy(this);
        else
            Instance = this;

        InitializeGst();
    }

    protected virtual void OnApplicationQuit()
    {
        ShutDownGstUnity();
    }

    private void InitializeGst()
    {
        bool gstInit = false;
        bool logInit = InitLog(_gstDebugLevel);

        if (_ignoreUnknownColorFormats)
        {
#if UNITY_EDITOR

#elif UNITY_ANDROID
            string var = "GST_AMC_IGNORE_UNKNOWN_COLOR_FORMATS";
            string val = "yes";
            SetEnvVar(var, val);
#endif
        }

        gstInit = InitGst();
        _gstInitialized = logInit && gstInit;

        if (_gstInitialized)
        {
            ListDecoders();
#if UNITY_EDITOR_WIN
            ListFeatures("d3d11h264dec");
#elif UNITY_ANDROID
            ListFeatures("amcviddec-omxqcomvideodecoderavclowlatency");
            ListFeatures("amcviddec-omxqcomvideodecoderavc");
#endif
            _streams = new List<StreamController>();
            StartCoroutine(UpdateFrames());
            InitializeStreams();
        }
    }

    protected void InitializeStreams()
    {
        StreamController[] streams = FindObjectsOfType<StreamController>();

        foreach (StreamController stream in streams)
        {
            if (stream.gameObject.activeSelf)
                stream.InitializeStream();
        }
    }

    protected void ShutDownGstUnity()
    {
        Debug.Log("Shutting down GstPlugin...");

        foreach (StreamController stream in _streams)
            stream.EndStream();

        _updateTexture = false;


        ShutDownGst();
        _gstInitialized = false;
    }

    public bool NewFrame(string streamName)
    {
        return NewFrameAvailable(streamName);
    }

    public bool SetupDebugTex(int w, int h, int d, int r, 
        int g, int b, int a)
    {
        return SetDebugTexture(w, h, d, r, g, b, a);
    }

    public int GetStreamEventId(string streamName)
    {
        return GetEventId(streamName);
    }

    public bool CreateTestPipeline(string streamName, int pattern, int width, int height, int format)
    {
        return TestPipeline(streamName, pattern, width, height, format);
    }

    public bool CreatePipelineStream(string streamName, string description, int texWidth, 
        int texHeight, int texDepth, StreamController stream)
    {
        _streams.Add(stream);

        return PipelineStream(streamName, description, texWidth,texHeight, texDepth);
    }

    public bool CreateUdpStream(string streamName, int width,
        int height, int depth, int fps, int port, string multicastGroup,
        string decoderType, bool requestLinkDecode, StreamController stream)
    {
        bool success = false;

        ListFeatures(decoderType);
        success = UdpStream(streamName, width, height, depth, fps,
            port, multicastGroup, decoderType, requestLinkDecode);

        if(success)
            _streams.Add(stream);

        return success;
    }

    public IntPtr GetPluginDebugEvent()
    {
        return GetDebugRenderEvent();
    }

    public IntPtr GetPluginRenderEvent()
    {
        return GetRenderEvent();
    }
    public bool RemovePipelineByName(string streamName)
    {
        return RemovePipeline(streamName);
    }

    public bool WriteDebugTexture(string filepath)
    {
        return WriteDebugTextureToFile(filepath);
    }

    public bool CopyLastFrameToDebugTexture(int eventId)
    {
        return CopyLastFrameToDebug(eventId);
    }

    protected IEnumerator UpdateFrames()
    {
        _updateTexture = true;

        Debug.Log("Starting Frame Update Coroutine...");

        while (_updateTexture)
        {
            yield return new WaitForEndOfFrame();
            OnTextureUpdate?.Invoke();
        }

        yield break;
    }

    public bool SetTexture(Texture2D tex, int id, string name)
    {
        IntPtr texPtr = tex.GetNativeTexturePtr();
        bool texSet = SetUnityTexture(texPtr, id, name);
        return texSet;
    }
}
