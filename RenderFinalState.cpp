#include "RenderFinalState.h"
#include "GameEngine.h"
#include "3DSModel.h"
#include "bssrdf.h"
#include "MyGlobal.h"

CRenderFinalState::CRenderFinalState(LPD3DXEFFECT effectNormalMap, 
                                     LPD3DXEFFECT effectPosMap, 
                                     LPD3DXEFFECT effectPhongMap,
                                     UINT imageWidth,
                                     UINT imageHeight,
                                     VERTEX_XYZNORM *samples,
                                     size_t sampledCount,
                                     MeshList& meshes
                                     ) : CGameState("renderfinal"),
                                     m_width(imageWidth),
                                     m_height(imageHeight),
                                     m_octree(NULL),
                                     m_meshes(meshes)
{
    m_counter = 0;

    m_effectNormalmap = effectNormalMap;
    m_effectPosmap = effectPosMap;
    m_effectPhong = effectPhongMap;
    D3DXVECTOR3 eyepos = GetCamera()->GetEye();   
    m_eyePos = Point3(eyepos.x, eyepos.y, eyepos.z);

    m_worldBound = m_meshes[0]->GetBoundBox();
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        //m_model->GetMesh(i)->ComputeBoundBox(&mWorld);
        m_worldBound = BoundBox::unionBoxes(m_worldBound, m_meshes[i]->GetBoundBox());
    }

    m_drawPrior = false;
    m_curBlock = 0;
    m_blockWidth = 20;
    m_blockHeight = 20;

    BoundBox wb = m_worldBound;

    float dx = wb.maxPoint.x - wb.minPoint.x;
    float dy = wb.maxPoint.y - wb.minPoint.y;
    float dz = wb.maxPoint.z - wb.minPoint.z;
    wb.expandDelta((dx + dy + dz) * 0.1f);

    m_octree = new ScatterOctree(wb);

    float totalArea = 0;
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        totalArea += m_meshes[i]->ComputeArea();
    }

    float darea = totalArea / sampledCount;
    for (size_t i = 0; i < sampledCount; ++i)
    {
        ScatterSample sample;
        sample.area = darea;
        D3DXCOLOR c = samples[i].color;
        sample.irradiance = Color(c.r, c.g, c.b);
        sample.position = Point3(samples[i].x, samples[i].y, samples[i].z);
        m_octree->insert(sample);
    }    
    m_octree->config();

    m_target.CreateTexture(GetDevice(), m_width, m_height);
    m_normalMap.CreateTexture(GetDevice(), m_width, m_height);
    m_posMap.CreateTexture(GetDevice(), m_width, m_height, g_PosmapFormat);
    m_phong.CreateTexture(GetDevice(), m_width, m_height);

    CreateRenderTargets();
    RenderToTexture();

    float factor = sqrtf((sampledCount / totalArea)) * 0.3f * g_RenderOption.scale;
    m_bssrdf = g_Bssrdf;
    m_bssrdf.scatter = g_Bssrdf.scatter * factor;
    m_bssrdf.absorb = g_Bssrdf.absorb * factor;
    m_bssrdf.UpdateParams();
}

CRenderFinalState::~CRenderFinalState(void)
{
    if (m_octree)
        delete m_octree;
}

void CRenderFinalState::Start(CGameState * prior)
{
    CGameState::Start(prior);
    m_startTime = GetTickCount();
    m_bFinished = false;
    m_usedTime = 0;
}

void CRenderFinalState::Draw2D(CGame2D * g2d)
{
    CGameState::Draw2D(g2d);
    g2d->DrawTexture(m_target.GetTextureDefault(), 0, 0, (float)GetScreenWidth(), 
        (float)GetScreenHeight());

    if (!m_bFinished)
    {
        size_t nBlockInRow = (m_target.GetWidth() + m_blockWidth - 1) / m_blockWidth;

        size_t row = m_curBlock / nBlockInRow;
        size_t col = m_curBlock % nBlockInRow;

        g2d->DrawString(LoadFont("Arial", 12), "Rendering", (int)(col * m_blockWidth + 2), 
            (int)(row * m_blockHeight + (m_blockHeight - 12) / 2), 0xFFFFFFFF);
        m_usedTime = GetTickCount() - m_startTime;
    }
    else
    {
        g2d->DrawString(LoadFont("宋体", 14), "渲染已完成，按ESC返回", 10, 30, 0xFFAAFFAA);
    }

    char txt[100];
    wsprintf(txt, "已用时间:%3d秒%3d毫秒", m_usedTime / 1000, m_usedTime % 1000);
    g2d->DrawString(LoadFont("宋体", 14), txt, 10, 10, 0xFFFFAAAA);
}

float FresnelDielectric(float cosI, float eta)
{
    float cosineI = cosI; //clamp(cosI, -1.0, 1.0);
    bool isEntering = cosineI > 0.0;
    float ei = 1.0f, et = eta;
    if (!isEntering)
        swap(ei, et);
    if (cosineI == 1.0f)
        return 1.0f;
    float sinT = ei / et * sqrt(max(0.0f, 1.0f - cosineI * cosineI));
    if (sinT > 1.0f)
    {
        // total internal reflection
        return 1.0f;
    }
    else
    {
        float cosT = sqrt(max(0.0f, 1.0f - sinT * sinT));
        float tempCosI = fabs(cosineI);
        float parl = (et * tempCosI - ei * cosT)
            / (et * tempCosI + ei * cosT);
        float perp = (ei * tempCosI - et * cosT)
            / (ei * tempCosI + et * cosT);
        float value = (parl * parl + perp * perp) / 2.0f;
        return value;
    }
}

void CRenderFinalState::PreRender()
{
    CGameState::PreRender();
    
    if (!m_octree)
        return;

    DWORD oldTime = GetTickCount();

    while (GetTickCount() < oldTime + 1000)
    {
        size_t nBlockInRow = (m_target.GetWidth() + m_blockWidth - 1) / m_blockWidth;

        size_t row = m_curBlock / nBlockInRow;
        size_t col = m_curBlock % nBlockInRow;

        if (row * m_blockHeight >= m_target.GetHeight())
        {
            m_bFinished = true;
            break;
        }
        RECT rt;
        rt.left = (int)(col * m_blockWidth);
        rt.right = (int)((col + 1) * m_blockWidth);
        rt.top = (int)(row * m_blockHeight);
        rt.bottom = (int)((row + 1) * m_blockHeight);

        D3DLOCKED_RECT lrTarget, lrNormal, lrPos, lrPhong;
        if (! m_target.LockRect(&lrTarget, &rt, 0))
            return;
        if (! m_normalMap.LockRect(&lrNormal, &rt, D3DLOCK_READONLY))
            return;
        if (! m_posMap.LockRect(&lrPos, &rt, D3DLOCK_READONLY))
            return;
        if (! m_phong.LockRect(&lrPhong, &rt, D3DLOCK_READONLY))
            return;

        unsigned char* bufTarget = (unsigned char*) lrTarget.pBits;
        unsigned char* bufNormal = (unsigned char*) lrNormal.pBits;
        unsigned char* bufPos = (unsigned char*) lrPos.pBits;
        unsigned char* bufPhong = (unsigned char*) lrPhong.pBits;

        Point3 pos, norm, realpos;
        float xlen = m_worldBound.maxPoint.x - m_worldBound.minPoint.x;
        float ylen = m_worldBound.maxPoint.y - m_worldBound.minPoint.y;
        float zlen = m_worldBound.maxPoint.z - m_worldBound.minPoint.z;
        float minx = m_worldBound.minPoint.x;
        float miny = m_worldBound.minPoint.y;
        float minz = m_worldBound.minPoint.z;
        
        float factor = 2 * g_RenderOption.transFactor;

        size_t w = min(m_blockWidth, m_width - col * m_blockWidth);
        size_t h = min(m_blockHeight, m_height - row * m_blockHeight);
        UINT px, py, pz;
        float ChannelMax;
      
        for (size_t i = 0; i < h; i++)
        {
            for (size_t j = 0; j < w; j++)
            {
                switch(g_PosmapFormat)
                {
                case D3DFMT_A2B10G10R10:
                    {
                        DWORD dw = *(DWORD *) (bufPos + j * 4);
                        px = dw & 0x3FF;
                        py = (dw & (0x3FF << 10)) >> 10;
                        pz = (dw & (0x3FF << 20)) >> 20;
                        ChannelMax = 1024.0f;
                        break;
                    }
                case D3DFMT_A2R10G10B10:
                    {
                        DWORD dw = *(DWORD *) (bufPos + j * 4);
                        pz = dw & 0x3FF;
                        py = (dw & (0x3FF << 10)) >> 10;
                        px = (dw & (0x3FF << 20)) >> 20;
                        ChannelMax = 1024.0f;
                        break;
                    }
                case D3DFMT_A8R8G8B8:
                    {
                        px = bufPos[j * 4 + 2];
                        py = bufPos[j * 4 + 1];
                        pz = bufPos[j * 4 + 0];
                        ChannelMax = 256.0f;
                        break;
                    }
                default:
                    px = py = pz = 0;
                }
                pos.x = px * xlen / ChannelMax + minx;
                pos.y = py * ylen / ChannelMax + miny;
                pos.z = pz * zlen / ChannelMax + minz;

                norm.x = bufNormal[j* 4 + 2] / 256.0f;
                norm.y = bufNormal[j* 4 + 1] / 256.0f;
                norm.z = bufNormal[j* 4 + 0] / 256.0f;
                
                D3DXVECTOR4 out;
                D3DXVec3Transform(&out, (D3DXVECTOR3 *) &pos, &m_worldMat);
                realpos.x = out.x;
                realpos.y = out.y;
                realpos.z = out.z;

                float nlen = norm.GetLength();

                norm /= nlen;

                if (nlen < 0.5f)
                    continue;
                
                Point3 wo = (m_eyePos - realpos).Normalize();
                Color color(0.0f, 0.0f, 0.0f);
                
                color = m_octree->lookup(pos, g_RenderOption.maxErr, &m_bssrdf);
                //color *= factor / 3.1415926f * (1.0f - FresnelDielectric(wo * norm, m_bssrdf.eta)); 
                color.r += bufPhong[j* 4 + 2] / 256.0f;
                color.g += bufPhong[j* 4 + 1] / 256.0f;
                color.b += bufPhong[j* 4 + 0] / 256.0f;
                color *= g_RenderOption.lightPower;
                bufTarget[j* 4 + 0] = (unsigned char)(max(0, min(color.b, 1)) * 255);
                bufTarget[j* 4 + 1] = (unsigned char)(max(0, min(color.g, 1)) * 255);
                bufTarget[j* 4 + 2] = (unsigned char)(max(0, min(color.r, 1)) * 255);
                bufTarget[j* 4 + 3] = 1;
            }
            bufTarget += lrTarget.Pitch;
            bufNormal += lrNormal.Pitch;
            bufPos += lrPos.Pitch;
            bufPhong += lrPhong.Pitch;

        }
        m_target.UnlockRect();
        m_normalMap.UnlockRect();
        m_posMap.UnlockRect();
        m_phong.UnlockRect();
        m_curBlock++;
        // Sleep(1);
    }
    m_target.Update(NULL, NULL);
}

void CRenderFinalState::Draw3D()
{
    CGameState::Draw3D();
}

void CRenderFinalState::Animate( float elapsedTime )
{
    CGameState::Animate(elapsedTime);
    m_counter += elapsedTime;
}

void CRenderFinalState::Update( float elapsedTime )
{
    CGameState::Update(elapsedTime);
    if (GetFocus() != GetWindow())
        return;
    if (GetKeyboard()->IsJustPressed('D'))
    {
        m_startTime = GetTickCount();
        m_bFinished = false;
        m_usedTime = 0;
        m_curBlock = 0;
    }
    
    if (m_bFinished && GetKeyboard()->IsJustPressed('S'))
    {
        ShowSaveImageDialog();
    }

    if (GetKeyboard()->IsJustPressed(VK_ESCAPE))
        GetStateStack()->RemoveTop();
}

void CRenderFinalState::DeviceLost()
{
    CGameState::DeviceLost();
    m_target.OnLostDevice();
    m_phong.OnLostDevice();
    m_normalMap.OnLostDevice();
    m_posMap.OnLostDevice();

    ReleaseRenderTargets();
}

void CRenderFinalState::DeviceReset()
{
    CGameState::DeviceReset();
    m_target.OnResetDevice();
    m_phong.OnResetDevice();
    m_normalMap.OnResetDevice();
    m_posMap.OnResetDevice();
    CreateRenderTargets();
}

void CRenderFinalState::CreateRenderTargets()
{
    if (!m_targetNormalMap.CreateTexture(GetDevice(), m_width, m_height))
        return;

    if (!m_targetPosMap.CreateTexture(GetDevice(), m_width, m_height, g_PosmapFormat))
        return;

    if (!m_targetPhong.CreateTexture(GetDevice(), m_width, m_height))
        return;
}

void CRenderFinalState::ReleaseRenderTargets()
{
    m_targetNormalMap.ReleaseTexture();
    m_targetPhong.ReleaseTexture();
    m_targetPosMap.ReleaseTexture();
}

void CRenderFinalState::RenderToTexture()
{
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    GetDevice()->GetTransform(D3DTS_WORLD, &mWorld);
    GetDevice()->GetTransform(D3DTS_VIEW, &mView);
    GetDevice()->GetTransform(D3DTS_PROJECTION, &mProj);
    D3DXMATRIXA16 mWVP = mWorld * mView * mProj;
    D3DXMATRIXA16 mWV = mWorld * mView;

    float xlen = m_worldBound.maxPoint.x - m_worldBound.minPoint.x;
    float ylen = m_worldBound.maxPoint.y - m_worldBound.minPoint.y;
    float zlen = m_worldBound.maxPoint.z - m_worldBound.minPoint.z;
    float minx = m_worldBound.minPoint.x;
    float miny = m_worldBound.minPoint.y;
    float minz = m_worldBound.minPoint.z;
    D3DXMATRIX tMat
        (
        1.0f / xlen, 0, 0, 0,
        0, 1.0f / ylen, 0, 0,
        0, 0, 1.0f / zlen, 0,
        -minx / xlen, -miny / ylen, -minz / zlen, 1
        );
    // tMat = mWorld * tMat;

    
    m_effectPhong->SetFloat("g_cSpecular", g_Bssrdf.specluar);
    m_effectPhong->SetFloatArray("g_cDiffuse", &g_Bssrdf.diffuse.r, 3);
    m_effectPhong->SetTechnique("SingleScatter");

    m_effectNormalmap->SetMatrix("g_mWorldViewProjection", &mWVP);
    m_effectNormalmap->SetMatrix("g_mWorld", &mWorld);

    m_effectPosmap->SetMatrix("g_mWorldViewProjection", &mWVP);
    m_effectPosmap->SetMatrix("g_mPosmapTransform", &tMat);

    // Begin render to targets
    LPDIRECT3DSURFACE9 backSurface;
    GetDevice()->GetRenderTarget(0, &backSurface);

    // Render normal map
    m_targetNormalMap.ApplyRenderTarget(GetDevice());
    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        DX_BEGIN_EFFECT(m_effectNormalmap);
        for (size_t i = 0; i < m_meshes.size(); i++)
            m_meshes[i]->Render();
        DX_END_EFFECT(m_effectNormalmap);        
        GetDevice()->EndScene();
    }


    // Render position map
    m_targetPosMap.ApplyRenderTarget(GetDevice());
    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        DX_BEGIN_EFFECT(m_effectPosmap);        
        for (size_t i = 0; i < m_meshes.size(); i++)
            m_meshes[i]->Render();
        DX_END_EFFECT(m_effectPosmap);        
        GetDevice()->EndScene();
    }

    // Render direct lighting
    m_targetPhong.ApplyRenderTarget(GetDevice());
    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        DX_BEGIN_EFFECT(m_effectPhong);        
        for (size_t i = 0; i < m_meshes.size(); i++)
            m_meshes[i]->Render();
        DX_END_EFFECT(m_effectPhong);        
        GetDevice()->EndScene();
    }

    GetDevice()->SetRenderTarget(0, backSurface);
    backSurface->Release();

    if (FAILED(GetDevice()->GetRenderTargetData(m_targetNormalMap.GetSurface(), m_normalMap.GetSurfaceSystem())))
    {
        MessageBox(GetWindow(), "error get render target data", "error", 0);
        return;
    }
    if (FAILED(GetDevice()->GetRenderTargetData(m_targetPosMap.GetSurface(), m_posMap.GetSurfaceSystem())))
    {
        MessageBox(GetWindow(), "error get render target data", "error", 0);
        return;
    }
    if (FAILED(GetDevice()->GetRenderTargetData(m_targetPhong.GetSurface(), m_phong.GetSurfaceSystem())))
    {
        MessageBox(GetWindow(), "error get render target data", "error", 0);
        return;
    }
    Sleep(100);
    D3DXSaveTextureToFile("posmap.png", D3DXIFF_PNG, m_targetPosMap.GetTexture(), NULL);
    D3DXSaveTextureToFile("normmap.png", D3DXIFF_PNG, m_targetNormalMap.GetTexture(), NULL);
}

void CRenderFinalState::ShowSaveImageDialog()
{
    OPENFILENAME ofn;       // common dialog box structure
    char szFile[260];       // buffer for file name

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetWindow();
    ofn.lpstrFile = szFile;
    //
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    //
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "PNG\0*.png\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "png";
    // Display the Open dialog box. 

    if (GetSaveFileName(&ofn)==TRUE) 
    {
        D3DXSaveTextureToFile(ofn.lpstrFile, D3DXIFF_PNG, m_target.GetTextureSystem(), NULL); 
    }
}

void CRenderFinalState::UpdateBssrdf()
{

}