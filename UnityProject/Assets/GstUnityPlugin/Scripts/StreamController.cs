using System;
using UnityEngine;

public abstract class StreamController : MonoBehaviour
{
    public enum ColorFormat 
    {
        COLOR_NONE = 0,
        COLOR_RGB = 1,
        COLOR_RGBA = 2,
        COLOR_NV12 = 3,
        COLOR_I420 = 4,
    }

    private readonly string[] _colorFormatNames =
        {"NONE", "RGB", "RGBA", "NV12", "I420"};

    protected readonly string _sinkName = "videoSink";


    [Header("Video Stream Params")]

    [SerializeField]
    protected bool _playOnInitialize = false;

    [SerializeField]
    protected bool _streamCreated = false;

    [SerializeField]
    protected string _streamName = "test_stream_01";

    [SerializeField]
    protected int _streamEventId = 0;


    [Header("Texture Params")]

    [SerializeField]
    protected ColorFormat _streamFormat;

    [SerializeField]
    protected bool _generateMips = false;

    [SerializeField]
    protected bool _linearColor = false;

    [SerializeField]
    protected TextureWrapMode _wrapMode = TextureWrapMode.Clamp;

    [SerializeField]
    protected FilterMode _filterMode = FilterMode.Point;

    [SerializeField]
    protected int _streamWidth = 0;

    [SerializeField]
    protected int _streamHeight = 0;

    [SerializeField]
    protected Material _rgb;

    [SerializeField]
    protected Material _nv12;

    [SerializeField]
    protected float _fade = 0;

    [SerializeField]
    protected float _fadeScalar = 10;

    [SerializeField]
    protected float _ditherSize = 0.5f;

    protected VideoSurface _surface = null;
    protected bool _texturesCreated = false;
    protected Material _current;
    protected Texture2D _rgbTex;
    protected Texture2D _yTex;
    protected Texture2D _uvTex;
    protected int _halfWidth = 0;
    protected int _halfHeight = 0;
    private bool _updateTexture = false;
    protected IntPtr _renderEvent = IntPtr.Zero;
    private Renderer _rend;
    public delegate void TextureUpdateDelegate();
    public event TextureUpdateDelegate OnTextureUpdate;


    // Update is called once per frame
    protected virtual void Update() { }

    // Start is called before the first frame update
    protected virtual void Start() { }

    public void InitializeStream()
    {
        Debug.Log("Calling initialize on " + _streamName);
        _surface = GetComponent<VideoSurface>();

        if(_surface != null)
            _surface.CreateMesh();
        
        BuildPipeline();

        if (_playOnInitialize)
            StartStream();
    }

    protected abstract void BuildPipeline();

    protected virtual void UpdateTexture()
    {
        if(_texturesCreated)
            GL.IssuePluginEvent(_renderEvent, _streamEventId);
        else
            _texturesCreated = CreateStreamTextures();
    }

    private void CheckNewFrame()
    {
        if (GstUnityController.Instance.NewFrame(_streamName))
            UpdateTexture();
    }

    public virtual void StartStream()
    {
        GstUnityController instance = GstUnityController.Instance;
        _streamEventId = instance.GetStreamEventId(_streamName);
        _renderEvent = instance.GetPluginRenderEvent();
        instance.OnTextureUpdate += CheckNewFrame;
    }

    public virtual void EndStream()
    {
        GstUnityController instance = GstUnityController.Instance;
        _updateTexture = false;
        instance.OnTextureUpdate -= CheckNewFrame;
        instance.RemovePipelineByName(_streamName);
        _renderEvent = IntPtr.Zero;
    }

    protected bool CreateStreamTextures()
    {
        bool success = false;
        int streamFormat = 0;
        GstUnityController instance = GstUnityController.Instance;

        if (instance.GetStreamWidthHeightFormat(_streamName, out _streamWidth, 
            out _streamHeight, out streamFormat))
        {
            _streamFormat = (ColorFormat)streamFormat;
            _halfWidth = _streamWidth / 2;
            _halfHeight = _streamHeight / 2;
                
            switch (_streamFormat)
            {
                case ColorFormat.COLOR_RGB:
                    success = ConfigMatRgb(TextureFormat.RGB24);
                    break;
                case ColorFormat.COLOR_RGBA:
                    success = ConfigMatRgb(TextureFormat.RGBA32);
                    break;
                case ColorFormat.COLOR_NV12:
                    success = ConfigMatNv12();
                    break;
                case ColorFormat.COLOR_I420:
                    // TO DO: Implement I420 materal.
                    Debug.LogError("I420 not yet implemented!");
                    break;
            }
        }
        else
            Debug.LogError("Unable to get stream meta from plugin!");

        return success;
    }

    private bool ConfigMatRgb(TextureFormat texFormat)
    {
        bool success = false;
        GstUnityController instance = GstUnityController.Instance;

        _rgbTex = new Texture2D(_streamWidth, _streamHeight, 
            texFormat, _generateMips, _linearColor);
        _rgbTex.filterMode = _filterMode;
        _rgbTex.wrapMode = _wrapMode;
        _current = new Material(_rgb);
        _rend = GetComponent<Renderer>();
        _rend.material = _current;
        _current.SetTexture("_rgb", _rgbTex);
        _current.SetFloat("_fade", _fade);
        _current.SetFloat("_fadeScalar", _fadeScalar);
        _current.SetFloat("_ditherSize", _ditherSize);
        _rgbTex.Apply();
        success = instance.SetTexture(_rgbTex, 0, _streamName);
        return success;
    }

    private bool ConfigMatNv12()
    {
        bool success = false;
        GstUnityController instance = GstUnityController.Instance;

        _yTex = new Texture2D(_streamWidth, _streamHeight, TextureFormat.R8, _generateMips, _linearColor);
        _uvTex = new Texture2D(_halfWidth, _halfHeight, TextureFormat.RG16, _generateMips, _linearColor);
        _yTex.filterMode = _filterMode;
        _yTex.wrapMode = _wrapMode;
        _uvTex.filterMode = _filterMode;
        _uvTex.wrapMode = _wrapMode;
        _current = new Material(_nv12);
        _rend = GetComponent<Renderer>();
        _rend.material = _current;
        _current.SetTexture("_y", _yTex);
        _current.SetTexture("_uv", _uvTex);
        _current.SetFloat("_fade", _fade);
        _current.SetFloat("_fadeScalar", _fadeScalar);
        _current.SetFloat("_ditherSize", _ditherSize);
        _yTex.Apply();
        _uvTex.Apply();
        success = (instance.SetTexture(_yTex, 1, _streamName)
            && instance.SetTexture(_uvTex, 2, _streamName));
        return success;
    }

    protected string FormatName(ColorFormat format)
    {
        return _colorFormatNames[(int)format];
    }
}
