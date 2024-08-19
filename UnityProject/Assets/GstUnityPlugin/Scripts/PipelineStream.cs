using UnityEngine;

public abstract class PipelineStreamController : StreamController
{
    [Header("Pipeline Description String")]

    [SerializeField]
    protected string _description = "";

    protected override void BuildPipeline()
    {
        if (!string.IsNullOrEmpty(_description))
        {
            GstUnityController.Instance.CreatePipelineStream(
                _streamName, _description, _streamWidth,
                _streamHeight, (int)_streamFormat, this
            );
        }
    }
}


