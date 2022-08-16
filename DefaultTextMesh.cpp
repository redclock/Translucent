#include <d3dx9.h>
#include "DefaultTextMesh.h"
#include "defaulttextreader.h"
#include "GameEngine.h"

CDefaultTextMesh::CDefaultTextMesh(void)
{
    m_vertexSize = sizeof(VERTEX_MESH_DEFAULT);
    m_indexSize = 4;
}

CDefaultTextMesh::~CDefaultTextMesh( void )
{
    Destroy();
}

bool CDefaultTextMesh::LoadFromFile( LPDIRECT3DDEVICE9 device, const char * filename )
{
    Init(device);
    DefaultTextReader reader;
    if (!reader.readFile(filename))
        return false;
    m_vertexCount = reader.vertexCount;
    DWORD SizeVertices = (DWORD)m_vertexCount * sizeof(VERTEX_MESH_DEFAULT);
    if ( FAILED(m_device->CreateVertexBuffer(
        SizeVertices, 
        0,
        VERTEX_MESH_DEFAULT::FVF,
        D3DPOOL_MANAGED,
        &m_pVB,
        NULL)))
    {
        MessageBox(GetWindow(), "Mesh DefaultText CreateVB Error", "error", 0);
        return false;
    }

    VERTEX_MESH_DEFAULT* pVertices;
    if ( FAILED(m_pVB->Lock( 0, SizeVertices, (VOID **)&pVertices, 0 ) ) ) 
    {
        MessageBox(GetWindow(), "Mesh DefaultText LockVB Error", "error", 0);
        return false;
    }

    for (size_t i = 0; i < reader.vertexCount; i++)
    {
        pVertices[i].pos.x = (float)reader.vertices[i * 3];
        pVertices[i].pos.y = (float)reader.vertices[i * 3 + 1];
        pVertices[i].pos.z = (float)reader.vertices[i * 3 + 2];
        if (reader.normals)
        {
            pVertices[i].norm.x = (float)reader.normals[i * 3];
            pVertices[i].norm.y = (float)reader.normals[i * 3 + 1];
            pVertices[i].norm.z = (float)reader.normals[i * 3 + 2];
        }
        if (reader.tex)
        {
            pVertices[i].tu = (float)reader.tex[i * 2];
            pVertices[i].tv = (float)reader.tex[i * 2 + 1];
        }
        pVertices[i].color = 0xFFFFFFFF;
    }

    m_indexCount = reader.faceCount * 3;

    DWORD SizeIndices = (DWORD)m_indexCount * sizeof(DWORD);
    if ( FAILED(m_device->CreateIndexBuffer(SizeIndices, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_pIB, NULL) ) )
    {
        MessageBox(GetWindow(), "Mesh DefaultText CreateIB Error", "error", 0);
        return false;
    }	

    DWORD * pIndices;
    if ( FAILED(m_pIB->Lock( 0, SizeIndices, (VOID **)&pIndices, 0 ) ) )
    {
        MessageBox(GetWindow(), "Mesh DefaultText LockIB Error", "error", 0);
        return false;
    }
    for (size_t i = 0; i < reader.faceCount; i++)
    {
        if (reader.inversed)
        {
            pIndices[i * 3] = (DWORD)reader.vertexIndices[i * 3];
            pIndices[i * 3 + 2] = (DWORD)reader.vertexIndices[i * 3 + 1];
            pIndices[i * 3 + 1] = (DWORD)reader.vertexIndices[i * 3 + 2];
        }
        else
        {
            pIndices[i * 3] = (DWORD)reader.vertexIndices[i * 3];
            pIndices[i * 3 + 1] = (DWORD)reader.vertexIndices[i * 3 + 1];
            pIndices[i * 3 + 2] = (DWORD)reader.vertexIndices[i * 3 + 2];
        }
    }
    if (!reader.normals)
        CalcNormals(pVertices, pIndices);
    m_pIB->Unlock();
    m_pVB->Unlock();
    ComputeBoundBox();

    return true;
}

void CDefaultTextMesh::CalcNormals(VERTEX_MESH_DEFAULT* vertices, DWORD* indices)
{
    for (size_t i = 0; i < m_indexCount / 3; i++)
    {
        Point3 norm = ((vertices[indices[i * 3]].pos - vertices[indices[i * 3 + 1]].pos)
            % (vertices[indices[i * 3]].pos - vertices[indices[i * 3 + 2]].pos));

        vertices[indices[i * 3]].norm += norm;
        vertices[indices[i * 3 + 1]].norm += norm;
        vertices[indices[i * 3 + 2]].norm += norm;
    }
    for (UINT i = 0; i < m_vertexCount; i++)
    {
        vertices[i].norm = vertices[i].norm.Normalize();
    }
}

void CDefaultTextMesh::Render()
{
    m_device->SetStreamSource(0, m_pVB, 0, sizeof(VERTEX_MESH_DEFAULT));
    m_device->SetIndices(m_pIB);
    m_device->SetFVF(VERTEX_MESH_DEFAULT::FVF);
    m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
        0,
        0,
        (UINT) m_vertexCount,
        0,
        (UINT) m_indexCount / 3);

}