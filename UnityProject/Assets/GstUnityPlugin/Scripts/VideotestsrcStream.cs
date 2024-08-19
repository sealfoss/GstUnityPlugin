using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class VideotestsrcStream : PipelineStreamController
{
    readonly string[] comps = 
        {"videotestsrc pattern=", "video/x-raw", "width=", 
        "height=", "format=","appsrc name=videoSink" }; 

    [Header("Videotestsrc Params")]

    [SerializeField]
    protected int _pattern = 0;

    [SerializeField]
    protected int _width = 1920;

    [SerializeField]
    protected int _height = 1080;

    [SerializeField]
    protected ColorFormat _format = ColorFormat.COLOR_RGBA;


    protected override void BuildPipeline()
    {
        string testsrc =  $"{comps[0]}{_pattern}";
        string caps = $"{comps[1]},{comps[2]}{_width},{comps[3]}{_height},{comps[4]}{FormatName(_format)}";
        _description = $"{testsrc} ! {caps} ! appsink name={_sinkName}";

        base.BuildPipeline();
    }
}
