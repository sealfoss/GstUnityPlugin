using UnityEngine;

public class VideoSpherical : VideoSurface
{
    [Header("Spherical Video Surface Params")]

    [SerializeField]
    protected float _radius = 1.0f;

    [SerializeField]
    protected float _fovHorizontalDegrees = 90.0f;

    [SerializeField]
    protected float _fovVerticalDegrees = 90.0f;

    [SerializeField]
    protected float _bearingHorizontalDegrees = 0.0f;

    [SerializeField]
    protected float _bearingVerticalDegrees = 0.0f;

    [SerializeField]
    bool _centerMesh = false;


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
        float uvW = 1.0f / _quadsWidth;
        float uvH = 1.0f / _quadsHeight;

        float fovV = Mathf.Deg2Rad * _fovVerticalDegrees;
        float fovH = Mathf.Deg2Rad * _fovHorizontalDegrees;
        float halfVertical = fovV / 2.0f;
        float halfHorizontal = fovH / 2.0f;

        float angleStepVertical = fovV / _quadsHeight;
        float angleStepHorizontal = fovH / _quadsWidth;
        float archBottom = (_bearingVerticalDegrees * Mathf.Deg2Rad) - halfVertical;
        float archLeft = (_bearingHorizontalDegrees * Mathf.Deg2Rad) - halfHorizontal;

        float x, y, z, u, v, alpha, beta, cosBeta, sinBeta, cosAlpha, sinAlpha;
        int indexA, indexB, indexC, indexD;
        float sumZ = 0;
        Vector3 offsetZ = Vector3.zero;

        for (int i = 0; i < vertsH; i++)
        {
            beta = archBottom + i * angleStepVertical;
            sinBeta = Mathf.Sin(beta);
            cosBeta = Mathf.Cos(beta);
            y = sinBeta;
            v = i * uvH;

            for (int j = 0; j < vertsW; j++)
            {
                u = j * uvW;
                alpha = archLeft + j * angleStepHorizontal;
                sinAlpha = Mathf.Sin(alpha);
                cosAlpha = Mathf.Cos(alpha);
                x = cosBeta * sinAlpha;
                z = cosBeta * cosAlpha;
                verts[vertIndex] = new Vector3(x,y,z) * _radius;
                uvs[vertIndex] = new Vector2(u, v);

                if (_centerMesh)
                    sumZ += verts[vertIndex].z;

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

        if(_centerMesh)
        {
            offsetZ = new Vector3(0, 0, (sumZ / numVerts));

            for(int i = 0; i < verts.Length; i++)
                verts[i] -= offsetZ;
        }

        _mesh = new Mesh();
        _mesh.vertices = verts;
        _mesh.uv = uvs;
        _mesh.triangles = tris;
    }
}
