using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using UnityEngine;

public class UdpStream : PipelineStreamController
{
    public enum DecoderType
    {
        DECODE_BIN = 0,
        DECODE_BIN3 = 1,
        OMX_QCOM = 2,
        OMX_QCOM_LOW = 3
    }

    public enum EncodingType
    {
        H264 = 0,
        H265 = 1
    }

    private readonly string[] _decoderNames =
    { "decodebin", "decodebin3", "amcviddec-omxqcomvideodecoderavc",
        "amcviddec-omxqcomvideodecoderavclowlatency"};
    private readonly string[] _encodingNames = { "H264", "H265" };
    readonly string _srcCaps = "application/x-rtp,media=video,playback=96,clock-rate=90000,encoding-name=";


    [Header("UDP Stream Params")]

    [SerializeField]
    protected bool _configureForPlatform = true;

    [SerializeField]
    protected int _port = 1337;

    [SerializeField]
    protected DecoderType _decoderType = DecoderType.DECODE_BIN;

    [SerializeField]
    protected EncodingType _encodeType = EncodingType.H264;

    [SerializeField]
    protected ColorFormat _udpFormat = ColorFormat.COLOR_NV12;

    [SerializeField]
    protected ColorFormat _sinkFormat = ColorFormat.COLOR_RGBA;

    [SerializeField]
    protected bool _parse = true;

    private void ConfigureForPlatform()
    {
#if UNITY_EDITOR
        _sinkFormat = ColorFormat.COLOR_RGBA;
        _decoderType = DecoderType.DECODE_BIN3;
#elif UNITY_ANDROID
        _sinkFormat = _udpFormat;
        _decoderType = DecoderType.OMX_QCOM_LOW;
#endif
    }

    protected override void BuildPipeline()
    {
        string encoding, decoder, sinkCaps, depay, parse, src, caps;

        if (_configureForPlatform)
            ConfigureForPlatform();

        encoding = _encodingNames[(int)_encodeType];
        decoder = _decoderNames[(int)_decoderType];
        sinkCaps = _udpFormat == _sinkFormat ? "" :
            $"video/x-raw,format={FormatName(_sinkFormat)}";
        depay = _encodeType == EncodingType.H264 ? "rtph264depay" : "rtph265depay";
        parse = _parse ? _encodeType == EncodingType.H264 ? "h264parse" : "h265parse" : "";
        src = $"udpsrc port={_port}";
        caps = $"{_srcCaps}{_encodingNames[(int)_encodeType]}";

        _description = $"{src} ! {caps} ! {depay}";

        if (_parse)
            _description += $" ! {parse}";

        _description += $" ! {decoder} ! ";

        if (_udpFormat != _sinkFormat)
            _description += $"videoconvert ! {sinkCaps} ! ";

        _description += $"appsink name={_sinkName}";
        base.BuildPipeline();
    }
}
