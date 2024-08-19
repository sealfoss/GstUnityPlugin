using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System;
using System.IO;
using UnityEngine;

public abstract class VideoSurface : MonoBehaviour
{
    [Header("Video Surface Params")]

    [SerializeField]
    protected Material _loadingMaterial = null;

    [SerializeField]
    protected bool _saveMesh = true;

    [SerializeField]
    protected bool _loadMesh = false;

    [SerializeField]
    protected string _meshPath = "";

    [SerializeField]
    protected int _quadsWidth = 2;

    [SerializeField]
    protected int _quadsHeight = 2;

    protected Mesh _mesh = null;

    protected MeshFilter _filter = null;

    protected MeshRenderer _rend = null;

    private void Awake()
    {
        _filter = GetComponent<MeshFilter>();
        _rend = GetComponent<MeshRenderer>();
    }

    // Start is called before the first frame update
    protected virtual void Start()
    {
        
    }

    // Update is called once per frame
    protected virtual void Update()
    {
        
    }

    protected abstract void GenMesh();

    public void CreateMesh()
    {
        if (_loadMesh)
            LoadMesh(_meshPath);
        else
            GenMesh();

        _rend.material = _loadingMaterial;
        _filter.mesh = _mesh;
    }

    protected bool LoadMesh(string meshPath)
    {
        bool loaded = false;

        if (!string.IsNullOrEmpty(meshPath)
            && !string.IsNullOrWhiteSpace(meshPath))
        {

        }
        else
            Debug.LogError("Unable to load mesh without a valid mesh path!");

        return loaded;
    }

    protected bool SaveMesh(string name)
    {
        bool saved = false;
        string datetime = DateTime.Now.ToString("MMddyyhmmtt");
        string filename = $"{name}_{datetime}.obj";
        string filepath = Path.Combine(Application.persistentDataPath, filename);
        return saved;
    }
}
