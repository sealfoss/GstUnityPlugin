using UnityEngine;

public class VideoPlanar : VideoSurface
{
    [Header("Planar Video Surface Params")]

    [SerializeField]
    protected float _planarWidth;

    [SerializeField]
    protected float _planarHeight;

    protected override void GenMesh()
    {

        int vertsW = _quadsWidth + 1;
        int vertsH = _quadsHeight + 1;
        int numVerts = vertsW * vertsH;
        Vector3[] verts = new Vector3[numVerts];
        Vector2[] uvs = new Vector2[numVerts];

        int numQuads = _quadsHeight * _quadsWidth;
        int numTris = numQuads * 2;
        int trisLen = numTris * 3;
        int[] tris = new int[trisLen];
        int triIndex = 0;
        int vertIndex = 0;
        float quadWidth = _planarWidth / _quadsWidth;
        float quadHeight = _planarHeight / _quadsHeight;
        float uvW = 1.0f / _quadsWidth;
        float uvH = 1.0f / _quadsHeight;
        float meshLeft = _planarHeight * -0.5f;
        float meshBottom = _planarWidth * -0.5f;
        float x, y, u, v;
        int indexA, indexB, indexC, indexD;

        for (int i = 0; i < vertsH; i++)
        {
            y = meshBottom + i * quadHeight;
            v = i * uvH;

            for (int j = 0; j < vertsW; j++)
            {
                x = meshLeft + j * quadWidth;
                u = j * uvW;
                verts[vertIndex] = new Vector3(x, y, 0);
                uvs[vertIndex] = new Vector2(u, v);

                if (j > 0 && i > 0)
                {
                    indexC = vertIndex;
                    indexB = vertIndex - 1;
                    indexD = indexC - vertsW;
                    indexA = indexD - 1;
                    tris[triIndex++] = indexA;
                    tris[triIndex++] = indexB;
                    tris[triIndex++] = indexC;
                    tris[triIndex++] = indexC;
                    tris[triIndex++] = indexD;
                    tris[triIndex++] = indexA;
                }

                vertIndex++;
            }
        }

        _mesh = new Mesh();
        _mesh.vertices = verts;
        _mesh.uv = uvs;
        _mesh.triangles = tris;
    }    
}
