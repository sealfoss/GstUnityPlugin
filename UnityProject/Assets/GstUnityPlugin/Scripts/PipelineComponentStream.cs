using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PipelineComponentStream : PipelineStreamController
{
    [Header("Pipeline Components")]

    [SerializeField]
    private string[] _pipelineComponents;

    protected override void BuildPipeline()
    {
        _description = "";

        for (int i = 0; i < _pipelineComponents.Length; i++)
        {
            _description += _pipelineComponents[i];

            if (i < _pipelineComponents.Length - 1)
                _description += " ! ";
        }

        base.BuildPipeline();
    }
}
