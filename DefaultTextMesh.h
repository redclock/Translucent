#pragma once
#include <vector>
#include <map>
#include <string>
#include "Object3D.h"
#include "3DSLoader.h"
#include "Point3.h"

struct VERTEX_MESH_DEFAULT
{
    Point3 pos;
    Point3 norm;

    DWORD color;
    FLOAT  tu, tv;   // Vertex texture coordinates
    static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL| D3DFVF_DIFFUSE  | D3DFVF_TEX1;
};

class CDefaultTextMesh: public CObject3D
{
public:
    bool LoadFromFile(LPDIRECT3DDEVICE9 device, const char * filename);

    void CalcNormals(VERTEX_MESH_DEFAULT* vertices, DWORD* indices);
    void Render();

    CDefaultTextMesh(void);
    ~CDefaultTextMesh(void);
};