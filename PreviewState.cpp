#include "PreviewState.h"
#include "GameEngine.h"
#include "3DSModel.h"
#include "RenderFinalState.h"
#include "MyGlobal.h"
#include "MyUtils.h"

CPreviewState::CPreviewState(void) : CGameState("preview")
{
    m_lastDown[0] = false;
    m_lastDown[1] = false;
    m_anglex = 0;
    m_anglez = 0;
    m_effectNormalmap = NULL;
    m_effectPosmap = NULL;
    m_effectPhong = NULL;
    m_sampleNormals = NULL;
    m_samplePoints = NULL;
    m_pointsBuf = NULL;
    m_model = NULL;
    m_dfMesh = NULL;
    m_counter = 0;
    m_lightPhi = 0;
    m_lightTheta = 0;

    m_radius = m_worldBound.diagonalLength() * 4;
    m_center = m_worldBound.center();

    m_bShowPoint = false;
    
    LoadModel();
    CreateSamples(g_RenderOption.sampleCount);

    // UpdateLight();

    m_targetShadow.CreateTexture(GetDevice(), g_RenderOption.shadowMapWidth, 
        g_RenderOption.shadowMapHeight, g_PosmapFormat);
    m_targetShadow2.CreateTexture(GetDevice(), g_RenderOption.shadowMapWidth, 
        g_RenderOption.shadowMapHeight, g_PosmapFormat);
    m_targetShadowMipmaps.CreateTexture(GetDevice(), g_RenderOption.shadowMapWidth, 
        g_RenderOption.shadowMapHeight, g_PosmapFormat, 0);

    m_shadowMap.CreateTexture(GetDevice(), g_RenderOption.shadowMapWidth, 
        g_RenderOption.shadowMapHeight, g_PosmapFormat);
    InitGUI();
}

CPreviewState::~CPreviewState(void)
{
    delete m_model;
    delete m_dfMesh;
    SAFE_RELEASE(m_effectNormalmap);
    SAFE_RELEASE(m_effectPosmap);
    SAFE_RELEASE(m_effectPhong);
    SAFE_RELEASE(m_effectShadowmap);
    delete[] m_samplePoints;
    delete[] m_sampleNormals;
    delete[] m_pointsBuf;
    delete m_dlgMain;
    delete m_dlgParams;
    delete m_dlgRender;
}

void CPreviewState::Start(CGameState * prior)
{
    CGameState::Start(prior);

    GetCamera()->SetEye(m_center.x, m_radius + m_center.y, m_center.z);
    GetCamera()->SetLookat(m_center.x, m_radius + m_center.y, m_center.z);
    GetCamera()->SetUp(0, 0, 1);

    m_effectNormalmap = CreateEffectFromFile("normalmap.fx");
    m_effectPosmap = CreateEffectFromFile("posmap.fx");
    m_effectPhong = CreateEffectFromFile("phong.fx");
    m_effectShadowmap = CreateEffectFromFile("shadowmap.fx");
}

void CPreviewState::Draw2D(CGame2D * g2d)
{
    CGameState::Draw2D(g2d);
    SetEffect(fxCopy);
    if (g_RenderOption.bShowShadowMap)
    {
        float ox = 0;
        for (int i = 1; i < MIPMAP_COUNT + 1; i++)
        {
            g2d->DrawTexture(m_targetShadowMipmaps.GetTexture(), ox, 0, 
                (float)GetScreenWidth() / (1 << i), (float)GetScreenHeight() / (1 << i));
            ox += (float)GetScreenWidth() / (1 << i);
        }
    }
    SetEffect(fxBlend);
    if (g_RenderOption.bShowUI)
    {
        m_dlgMain->Draw(g2d);
        if (m_curDialog)
            m_curDialog->Draw(g2d);
    }
    //char txt[100];
    //wsprintf(txt, "SM: %d, UP: %d, 3D: %d", times[0], times[1], times[2]);
    //g2d->DrawString(LoadFont("Arial", 14), txt, 100, 100, 0xFFFFFFFF);
}

void CPreviewState::PreRender()
{
    CGameState::PreRender();
}

void CPreviewState::Draw3D()
{
    CGameState::Draw3D();
    DWORD t = GetTickCount();

    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 1, 1, 1), 1.0f, 0); 
    //Get2D()->DrawRect(NULL, 0xFF224488);
    GetCamera()->SetEye(m_center.x, 
                        m_radius + m_center.y, 
                        m_center.z);
    m_eyePos = Point3(m_center.x, 
                     m_radius + m_center.y, 
                     m_center.z);
    GetCamera()->SetLookat(m_center.x, m_center.y, m_center.z);
    GetCamera()->SetUp(0, 0, 1);
    GetCamera()->UpdateMatrix();

    GetDevice()->SetTransform(D3DTS_WORLD, &m_worldMat);

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    GetDevice()->GetTransform(D3DTS_WORLD, &mWorld);
    GetDevice()->GetTransform(D3DTS_VIEW, &mView);
    GetDevice()->GetTransform(D3DTS_PROJECTION, &mProj);
    D3DXMATRIXA16 mWVP = mWorld * mView * mProj;
    D3DXMATRIXA16 mWV = mWorld * mView;
    //mWVP = m_shadowMapMat;
    //m_worldMat = mWorld;

    //BoundBox worldBound = m_meshes[0]->GetBoundBox();
    if (g_RenderOption.bShowBoundBox)
    {
        for (size_t i = 0; i < m_meshes.size(); i++)
        {
            //m_model->GetMesh(i)->ComputeBoundBox(&mWorld);
            //worldBound = BoundBox::unionBoxes(worldBound, m_meshes[i]->GetBoundBox());
            DrawBoundBox(m_meshes[i]->GetBoundBox(), 0xFFFFFFFF);
        }
    }

    if (m_bShowPoint)
    {
        ComputeSampleIrradiance();
        DrawPoints();
    }
    else
    {
        float fOffsetX = 0.5f + (0.5f / g_RenderOption.shadowMapWidth);
        float fOffsetY = 0.5f + (0.5f / g_RenderOption.shadowMapHeight);
        D3DXMATRIX texScaleBiasMat(0.5f, 0.0f, 0.0f, 0.0f,
                                   0.0f, -0.5f, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   fOffsetX, fOffsetY, 0.0f, 1.0f );
        D3DXMATRIX mShadow = m_shadowMapMat * texScaleBiasMat;
        D3DXMATRIX mShadowNoWorld = m_shadowMapMatNoWorld * texScaleBiasMat;
        static float zeroDiff[3] = {0.0f, 0.0f, 0.0f};
        Bssrdf bssrdf = AdjustBssrdf();

        m_effectPhong->SetMatrix("g_mProjectToLight", &mShadow);
        m_effectPhong->SetMatrix("g_mProjectToLightNoWorld", &mShadowNoWorld);
        m_effectPhong->SetMatrix("g_mWorldViewProjection", &mWVP);
        m_effectPhong->SetMatrix("g_mWorld", &mWorld);

        m_effectPhong->SetFloatArray("g_LightDir", &m_lightDir.x, 3);
        m_effectPhong->SetFloatArray("g_eyePos", &m_eyePos.x, 3);
        m_effectPhong->SetFloat("g_LightPower", g_RenderOption.lightPower);
        m_effectPhong->SetFloat("g_cSpecular", g_Bssrdf.specluar);
        m_effectPhong->SetFloat("g_Scale", log(g_RenderOption.scale) + 5.1f);
        m_effectPhong->SetFloatArray("g_cDiffuse", &g_Bssrdf.diffuse.r, 3);
        m_effectPhong->SetTexture("g_ShadowMap", m_targetShadow.GetTexture());
        m_effectPhong->SetTexture("g_ShadowMapMipmaps", m_targetShadowMipmaps.GetTexture());
        m_effectPhong->SetFloatArray("g_LightBound", &m_lightBound.x, 3);
        m_effectPhong->SetFloatArray("g_SigmaT", &bssrdf.sigt.r, 3);
        m_effectPhong->SetFloatArray("g_SigmaS", &bssrdf.scatter.r, 3);
        m_effectPhong->SetFloatArray("g_SigmaTR", &bssrdf.sigtr.r, 3);
        m_effectPhong->SetFloat("g_Zr", bssrdf.zr);
        m_effectPhong->SetFloat("g_Zv", bssrdf.zv);
        m_effectPhong->SetFloat("g_Eta", bssrdf.eta);
        m_effectPhong->SetFloat("g_Phase", bssrdf.g);
        m_effectPhong->SetFloat("g_Fdr", bssrdf.fdr);

        switch (m_listTechnique->GetIndex())
        {
        case 0:
            m_effectPhong->SetTechnique("PreviewPhong");
            break;
        case 1:
            m_effectPhong->SetTechnique("PreviewSingleScattering");
            break;
        case 2:
            m_effectPhong->SetTechnique("PreviewMultiScattering");
            break;
        case 3:
            m_effectPhong->SetTechnique("PreviewFullScattering");
            break;
        default:
            m_effectPhong->SetTechnique("SpecularPhong");
            break;
        }
        //GetDevice()->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
        DX_BEGIN_EFFECT(m_effectPhong);        
        for (size_t i = 0; i < m_meshes.size(); i++)
            m_meshes[i]->Render();
        DX_END_EFFECT(m_effectPhong);        
    }

    times[2] = GetTickCount() - t;
}

void CPreviewState::Animate( float elapsedTime )
{
    CGameState::Animate(elapsedTime);
}

void CPreviewState::Update( float elapsedTime )
{
    times[1] = 0;
    CGameState::Update(elapsedTime);
    g_StateInfo.isPreview = true;
    if (GetFocus() != GetWindow())
    {
        m_lastDown[0] = m_lastDown[1] = false;
        return;
    }
    CMouseManager* mouse = GetMouse();
    bool down[2] = {mouse->IsPressed(0), mouse->IsPressed(1)};
    int x = mouse->GetX();
    int y = mouse->GetY();

    int dx = x - m_lastx;
    int dy = y - m_lasty;
    
    bool needUpdate = false;

    if (m_lastDown[0])
    {
        m_anglex -= dy * 0.01f; // * elapsedTime;
        m_anglez -= dx * 0.01f;// * elapsedTime;
        needUpdate = true;
    }
    if (GetMouse()->GetWheel() != 0)
    {
        m_radius *= max(0.01f, 1 - GetMouse()->GetWheel() * 0.0003f);
        ResetProjectionMat();
    }
    if (m_lastDown[1])
    {
        m_lightTheta -= dy * 0.01f; // * elapsedTime;
        m_lightPhi -= dx * 0.01f;// * elapsedTime;
        needUpdate = true;
    }

    if (needUpdate || m_counter == 0.0f)
    {
        D3DXMATRIX mat1, mat2;
        D3DXMatrixTranslation(&mat1, -m_center.x, -m_center.y, -m_center.z);
        D3DXMatrixRotationX(&mat2, m_anglex);
        mat1 *= mat2;
        D3DXMatrixRotationZ(&mat2, m_anglez);
        mat1 *= mat2;
        D3DXMatrixTranslation(&mat2, m_center.x, m_center.y, m_center.z);
        m_worldMat = mat1 * mat2;

        UpdateLight();
    }


    m_lastDown[0] = down[0];
    m_lastDown[1] = down[1];
    m_lastx = x;
    m_lasty = y;

    if (g_RenderOption.bShowUI)
    {
        m_dlgMain->Update(elapsedTime);
        if(m_curDialog)
            m_curDialog->Update(elapsedTime);
    }

    if (m_curDialog != NULL 
        && m_curDialog->IsInBound((float)GetMouse()->GetX(), (float)GetMouse()->GetY())
        && m_curDialog->GetFocusControl() != NULL)
    {
        m_lastDown[0] = m_lastDown[1] = false;
    }
    if (m_btLoadModel->IsClicked())
    {
        ShowOpenModelDialog();
    }

    if (m_btRender->IsClicked())
    {
        m_curDialog = m_dlgRender;
        m_curDialog->Reset();
    }

    if (m_btParams->IsClicked())
    {
        UpdateGUI();
        m_curDialog = m_dlgParams;
        m_curDialog->Reset();
    }

    if (m_btBuiltin->IsClicked())
    {
        m_curDialog = m_dlgBuiltin;
        m_curDialog->Reset();
        m_listMaterials->SetIndex(-1);
    }

    if (m_curDialog == m_dlgParams)
    {
        if (m_btCloseParams->IsClicked())
        {
            m_curDialog = NULL;
        }
        g_RenderOption.scale = exp(m_slScale->GetValue());
        g_Bssrdf.absorb.r = m_slSigmaA[0]->GetValue();
        g_Bssrdf.absorb.g = m_slSigmaA[1]->GetValue();
        g_Bssrdf.absorb.b = m_slSigmaA[2]->GetValue();
        g_Bssrdf.scatter.r = m_slSigmaS[0]->GetValue();
        g_Bssrdf.scatter.g = m_slSigmaS[1]->GetValue();
        g_Bssrdf.scatter.b = m_slSigmaS[2]->GetValue();
        g_Bssrdf.diffuse.r = m_slDiffuse[0]->GetValue();
        g_Bssrdf.diffuse.g = m_slDiffuse[1]->GetValue();
        g_Bssrdf.diffuse.b = m_slDiffuse[2]->GetValue();
        g_Bssrdf.eta = m_slEta->GetValue();
    }
    if (m_curDialog == m_dlgRender)
    {
        if (m_btCloseRender->IsClicked())
        {
            m_curDialog = NULL;
        }
        if (m_btCPURender->IsClicked())
        {
            ComputeSampleIrradiance();

            GetStateStack()->Push(new CRenderFinalState(m_effectNormalmap, 
                m_effectPosmap, m_effectPhong,
                GetScreenWidth(), GetScreenHeight(), m_pointsBuf, m_sampled, m_meshes));
            g_StateInfo.isPreview = false;
        }
        g_RenderOption.lightPower = exp(m_slLightPower->GetValue());
        g_RenderOption.maxErr = m_slMaxErr->GetValue();
    }

    if (m_curDialog == m_dlgBuiltin)
    {
        if (m_btCloseBuiltin->IsClicked())
        {
            m_curDialog = NULL;
        }
        int index = m_listMaterials->GetIndex();
        if (index >= 0)
            g_Bssrdf = g_BuiltinBssrdfs[index];
    }

    if (GetKeyboard()->IsJustPressed('1'))
        m_bShowPoint = ! m_bShowPoint;
    if (GetKeyboard()->IsJustPressed('2'))
    {
        g_RenderOption.bShowShadowMap = !g_RenderOption.bShowShadowMap;
    }
    if (GetKeyboard()->IsJustPressed('3'))
    {
        g_RenderOption.bShowBoundBox = !g_RenderOption.bShowBoundBox;
    }
    if (GetKeyboard()->IsJustPressed('4'))
    {
        g_RenderOption.bShowUI = !g_RenderOption.bShowUI;
    }
    if (GetKeyboard()->IsPressed('C'))
    {
        g_RenderOption.scale -= 0.8f * g_RenderOption.scale * elapsedTime;
    }

    if (GetKeyboard()->IsPressed('V'))
    {
        g_RenderOption.scale += 0.8f * g_RenderOption.scale * elapsedTime;
    }
    
    if (GetKeyboard()->IsJustPressed('S'))
    {
        ShowSaveImageDialog();
    }
    if (GetKeyboard()->IsJustPressed('G'))
    {
        const char* filename = getFileName(g_StateInfo.modelFile);
        char path[200] = "D:\\MyWorks\\VC\\3d\\Translucent\\bin\\meshes\\";
        strcat(path, filename);
        CopyFile(g_StateInfo.modelFile, path, TRUE);
    }

    m_counter += elapsedTime;

}

void CPreviewState::DeviceLost()
{
    CGameState::DeviceLost();
    m_effectNormalmap->OnLostDevice();
    m_effectPosmap->OnLostDevice();
    m_effectPhong->OnLostDevice();
    m_effectShadowmap->OnLostDevice();
    m_shadowMap.OnLostDevice();
}

void CPreviewState::DeviceReset()
{
    CGameState::DeviceReset();
    m_effectNormalmap->OnResetDevice();
    m_effectPosmap->OnResetDevice();
    m_effectPhong->OnResetDevice();
    m_effectShadowmap->OnLostDevice();
    m_shadowMap.OnResetDevice();

    ResetProjectionMat();
}

void CPreviewState::DrawBoundBox(const BoundBox& box, DWORD color)
{
    VERTEX_XYZC vertices[] =
    {
        box.minPoint.x, box.minPoint.y, box.minPoint.z, color,
        box.maxPoint.x, box.minPoint.y, box.minPoint.z, color,

        box.maxPoint.x, box.minPoint.y, box.minPoint.z, color,
        box.maxPoint.x, box.maxPoint.y, box.minPoint.z, color,

        box.maxPoint.x, box.maxPoint.y, box.minPoint.z, color,
        box.minPoint.x, box.maxPoint.y, box.minPoint.z, color,

        box.minPoint.x, box.maxPoint.y, box.minPoint.z, color,
        box.minPoint.x, box.minPoint.y, box.minPoint.z, color,


        box.minPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.minPoint.y, box.maxPoint.z, color,

        box.maxPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.maxPoint.y, box.maxPoint.z, color,

        box.maxPoint.x, box.maxPoint.y, box.maxPoint.z, color,
        box.minPoint.x, box.maxPoint.y, box.maxPoint.z, color,

        box.minPoint.x, box.maxPoint.y, box.maxPoint.z, color,
        box.minPoint.x, box.minPoint.y, box.maxPoint.z, color,


        box.minPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.minPoint.x, box.minPoint.y, box.minPoint.z, color,

        box.maxPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.minPoint.y, box.minPoint.z, color,

        box.maxPoint.x, box.maxPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.maxPoint.y, box.minPoint.z, color,

        box.minPoint.x, box.maxPoint.y, box.maxPoint.z, color,
        box.minPoint.x, box.maxPoint.y, box.minPoint.z, color,
    };
    //D3DXMATRIXA16 mWorld;
    //D3DXMatrixIdentity(&mWorld);
    //GetDevice()->SetTransform(D3DTS_WORLD, &mWorld);
    GetDevice()->SetFVF(VERTEX_XYZC::FVF);
    GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 12, vertices, sizeof(VERTEX_XYZC));
}

void CPreviewState::DrawSceneBox(const BoundBox& box, DWORD color)
{
    VERTEX_XYZC vertices[] =
    {
        box.minPoint.x, box.minPoint.y, box.minPoint.z, color,
        box.maxPoint.x, box.minPoint.y, box.minPoint.z, color,
        box.minPoint.x, box.maxPoint.y, box.minPoint.z, color,
        box.maxPoint.x, box.maxPoint.y, box.minPoint.z, color,
        box.minPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.minPoint.y, box.maxPoint.z, color,
        box.minPoint.x, box.maxPoint.y, box.maxPoint.z, color,
        box.maxPoint.x, box.maxPoint.y, box.maxPoint.z, color,

    };
    //WORD indices[] =
    //{
    //    0, 1, 2, 
    //}
    //D3DXMATRIXA16 mWorld;
    //D3DXMatrixIdentity(&mWorld);
    //GetDevice()->SetTransform(D3DTS_WORLD, &mWorld);
    GetDevice()->SetFVF(VERTEX_XYZC::FVF);
    GetDevice()->DrawPrimitiveUP(D3DPT_LINELIST, 12, vertices, sizeof(VERTEX_XYZC));
}

void CPreviewState::DrawPoints()
{
    SetEffect(fxAdd);
    GetDevice()->SetFVF(VERTEX_XYZNORM::FVF);
    GetDevice()->DrawPrimitiveUP(D3DPT_POINTLIST, (UINT)m_sampled, m_pointsBuf, sizeof(VERTEX_XYZNORM));
    SetEffect(fxCopy);
}

/**
* Returns sample count by weight.
*/
inline size_t sampleCount(size_t count, float weight, float u)
{
    float d = count * weight;
    size_t scount = static_cast<size_t>(d);
    d -= scount;
    if (u < d)
        ++scount;
    return scount;
}


void CPreviewState::CreateSamples( size_t count )
{
    if (m_sampleNormals)
    {
        delete [] m_sampleNormals;
        m_sampleNormals = NULL;
    }
    if (m_samplePoints)
    {
        delete [] m_samplePoints;
        m_samplePoints = NULL;
    }
    if (m_pointsBuf)
    {
        delete [] m_pointsBuf;
        m_pointsBuf = NULL;
    }

    size_t max = count * 2;
    m_samplePoints = new Point3[max];
    m_sampleNormals = new Point3[count * 2];
    
    m_totalArea = 0;
    m_sampled = 0;
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_totalArea += m_meshes[i]->ComputeArea();
    }
    printf("total area = %f\n", m_totalArea);
    printf("box volume = %f\n", m_worldBound.volume());
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        float area = m_meshes[i]->ComputeArea();
        size_t countSamples = sampleCount(count, area / m_totalArea, RandomRange(0, 1));
        size_t sampled = m_meshes[i]->SamplePoints(countSamples, max - m_sampled, 
                                                  m_samplePoints + m_sampled, 
                                                  m_sampleNormals + m_sampled);
        m_sampled += sampled;
    }
    m_pointsBuf= new VERTEX_XYZNORM[m_sampled];
    
    for (size_t i = 0; i < m_sampled; i++)
    {
        m_pointsBuf[i].x = m_samplePoints[i].x;
        m_pointsBuf[i].y = m_samplePoints[i].y;
        m_pointsBuf[i].z = m_samplePoints[i].z;
        m_pointsBuf[i].nx = m_sampleNormals[i].x;
        m_pointsBuf[i].ny = m_sampleNormals[i].y;
        m_pointsBuf[i].nz = m_sampleNormals[i].z;
        m_pointsBuf[i].color = D3DXCOLOR(1, 1, 1, 1) * (m_lightDir * m_sampleNormals[i]);
    }
    printf("m_sampled = %d\n", m_sampled);
}

void CPreviewState::LoadModel()
{
    if (m_model)
    {
        delete m_model;
        m_model = NULL;
    }
    if (m_dfMesh)
    {
        delete m_dfMesh;
        m_dfMesh = NULL;
    }
    m_meshes.clear();
    const char* ext = getExtName(g_StateInfo.modelFile);
    if (stricmp(ext, "3ds") == 0)
    {
        m_model = new C3DSModel();
        m_model->LoadFromFile(g_StateInfo.modelFile);
        for (size_t i = 0; i < m_model->GetMeshCount(); i++)
        {
            m_meshes.push_back(m_model->GetMesh(i));
        }
    }
    else if (stricmp(ext, "defaulttext") == 0)
    {
        m_dfMesh = new CDefaultTextMesh();
        if (!m_dfMesh->LoadFromFile(GetDevice(), g_StateInfo.modelFile))
            MessageBox(GetWindow(), "Can not load mesh", "Error", 0);
        m_meshes.push_back(m_dfMesh);
    }
    else
    {
        return;
    }
    m_worldBound = m_meshes[0]->GetBoundBox();
    size_t totalFactCount = 0, totalVetrtexCount = 0;
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_worldBound = BoundBox::unionBoxes(m_worldBound, m_meshes[i]->GetBoundBox());
        size_t face = m_meshes[i]->GetFaceCount();
        size_t vertex = m_meshes[i]->GetVertexCount();
        printf("Part#%d, area = %f, face = %d, vertex = %d\n", 
            i, m_meshes[i]->ComputeArea(), face, vertex);
        totalFactCount += face;
        totalVetrtexCount += vertex;
    }

    printf("Total face = %d, vertex = %d\n", totalFactCount, totalVetrtexCount);
    m_radius = m_worldBound.diagonalLength() * 3;
    m_center = m_worldBound.center();
    ResetProjectionMat();
}

void CPreviewState::ComputeSampleIrradiance()
{
    if (FAILED(GetDevice()->GetRenderTargetData(m_targetShadow.GetSurface(), m_shadowMap.GetSurfaceSystem())))
    {
        MessageBox(GetWindow(), "error get render target data", "error", 0);
        return;
    }
    D3DLOCKED_RECT lrShadow; 
    if (!m_shadowMap.LockRect(&lrShadow, NULL, D3DLOCK_READONLY))
        return;

    unsigned char* bufShadow = (unsigned char*) lrShadow.pBits;
    
    D3DXVECTOR4 outzero;
    D3DXVECTOR3 zero(0, 0, 0);
    D3DXVec3Transform(&outzero, &zero, &m_worldMat);
    
    for (size_t i = 0; i < m_sampled; i++)
    {
        D3DXVECTOR4 out;
        D3DXVec3Transform(&out, (D3DXVECTOR3 *) &m_samplePoints[i], &m_shadowMapMat);
        out.x = (out.x + 1 + 1.0f /  g_RenderOption.shadowMapWidth) / 2;
        out.y = (-out.y + 1 + 1.0f /  g_RenderOption.shadowMapHeight) / 2;
        int texX = (int)(out.x * g_RenderOption.shadowMapWidth);
        int texY = (int)(out.y * g_RenderOption.shadowMapHeight);
        //if (texX > 255 || texX < 0)
        //    MessageBox(GetWindow(), "!!!!!", "", 0);
        //if (texY > 255 || texY < 0)
        //    MessageBox(GetWindow(), "!!!!!", "", 0);
        texX = max(0, min(texX, (int)g_RenderOption.shadowMapWidth - 1));
        texY = max(0, min(texY, (int)g_RenderOption.shadowMapHeight - 1));
        float depth;

        switch(g_PosmapFormat)
        {
        case D3DFMT_G32R32F:
            {
                DWORD r = *(DWORD *) (bufShadow + texY * lrShadow.Pitch + texX * 8);
                depth = (r) / 4294967295.0f;
                //py = (dw & (0x3FF << 10)) >> 10;
                //depth = ((dw & (0x3FF << 20)) >> 20) / 1023.0f;
                break;
            }
        case D3DFMT_G16R16F:
            {
                WORD r = *(WORD *) (bufShadow + texY * lrShadow.Pitch + texX * 4);
                depth = (r) / 65535.0f;
                //py = (dw & (0x3FF << 10)) >> 10;
                //depth = ((dw & (0x3FF << 20)) >> 20) / 1023.0f;
                break;
            }
        case D3DFMT_A2B10G10R10:
            {
                DWORD dw = *(DWORD *) (bufShadow + texY * lrShadow.Pitch + texX * 4);
                depth = (dw & 0x3FF) / 1023.0f;
                //py = (dw & (0x3FF << 10)) >> 10;
                //depth = ((dw & (0x3FF << 20)) >> 20) / 1023.0f;
                break;
            }
        case D3DFMT_A2R10G10B10:
            {
                DWORD dw = *(DWORD *) (bufShadow + texY * lrShadow.Pitch + texX * 4);
                //depth = (dw & 0x3FF) / 1023.0f;
                //py = (dw & (0x3FF << 10)) >> 10;
                depth =  ((dw & (0x3FF << 20)) >> 20) / 1023.0f;
                break;
            }
        case D3DFMT_A8R8G8B8:
            {
                depth = bufShadow[texY * lrShadow.Pitch + texX * 4 + 2] / 256.0f;
                //py = bufPos[j * 4 + 1];
                //depth = bufShadow[texY * lrShadow.Pitch + texX * 4] / 256.0f;
                break;
            }
        default:
            depth = 1;
        }
        bool isShadow = out.z > depth + 0.001f;
        if (!isShadow)
        {
            D3DXVec3Transform(&out, (D3DXVECTOR3 *) &m_sampleNormals[i], &m_worldMat);
            out -= outzero;
            D3DXVec4Normalize(&out, &out);
            m_pointsBuf[i].nx = out.x;
            m_pointsBuf[i].ny = out.y;
            m_pointsBuf[i].nz = out.z;
            float c = m_lightDir * *(Point3 *)&out;
            if (c < 0)
                m_pointsBuf[i].color = 0xFF000000;
            else
                m_pointsBuf[i].color = D3DXCOLOR(1, 1, 1, 1) * c;
        }
        else
        {
            m_pointsBuf[i].color = 0xFF000000;
        }

    }
    m_shadowMap.UnlockRect();
}

void CPreviewState::ComputeRotateMatrix(D3DXMATRIX* out, float dx, float dy, float dz)
{
    DWORD t = GetTickCount();
    Point3 vecZ(-dx, -dy, -dz);
    Point3 vecX, vecY;
    coordinateSystem(vecZ, &vecX, &vecY);
    D3DXMATRIX matView
        (
        vecX.x, vecY.x, vecZ.x, 0,
        vecX.y, vecY.y, vecZ.y, 0,
        vecX.z, vecY.z, vecZ.z, 0,
        0, 0, 0, 1
        );
    D3DXMATRIX mWorldView = m_worldMat * matView; 
    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i]->ComputeBoundBox(&mWorldView);
    }

    BoundBox wb = m_meshes[0]->GetBoundBox();

    for (size_t i = 1; i < m_meshes.size(); i++)
    {
        wb = BoundBox::unionBoxes(m_meshes[i]->GetBoundBox(), wb);
    }

    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i]->ComputeBoundBox();
    }

    float xlen = wb.maxPoint.x - wb.minPoint.x;
    float ylen = wb.maxPoint.y - wb.minPoint.y;
    float zlen = wb.maxPoint.z - wb.minPoint.z;

    wb.expandDelta(xlen * 0.01f, ylen * 0.01f, 0);

    xlen = wb.maxPoint.x - wb.minPoint.x;
    ylen = wb.maxPoint.y - wb.minPoint.y;
    zlen = wb.maxPoint.z - wb.minPoint.z;

    float minx = wb.minPoint.x;
    float miny = wb.minPoint.y;
    float minz = wb.minPoint.z;
    m_lightBound = Point3(xlen, ylen, zlen);
    
    D3DXMATRIX matProj
        (
        2.0f / xlen, 0, 0, 0,
        0, 2.0f / ylen, 0, 0,
        0, 0, 1.0f / zlen, 0,
        (-2 * minx / xlen - 1), (-2 * miny / ylen - 1), -minz / zlen, 1
        );

    *out = matView * matProj;
    times[1] = GetTickCount() - t;
}

void CPreviewState::ShowSaveImageDialog()
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
        GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(1, 1, 1, 1), 1.0f, 0); 
        if (SUCCEEDED(GetDevice()->BeginScene()))
        {
            Draw3D();
            GetDevice()->EndScene();
            Get2D()->ScreenShot(ofn.lpstrFile); 
        }
    }
}

void CPreviewState::ShowOpenModelDialog()
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
    ofn.lpstrFilter = "模型文件\0*.3ds;*.defaulttext\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // Display the Open dialog box. 

    if (GetOpenFileName(&ofn)==TRUE) 
    {
        strcpy(g_StateInfo.modelFile, ofn.lpstrFile);
        LoadModel();
        CreateSamples(g_RenderOption.sampleCount);
        UpdateLight();
    }
}

void CPreviewState::ResetProjectionMat()
{
    //Projection Transform
    float fAspectRatio = GetScreenWidth() / (FLOAT)GetScreenHeight();
    D3DXMATRIX matProj;
    float len = m_worldBound.diagonalLength();
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/8, fAspectRatio, 
        max(m_radius - len / 2, 0.1f), m_radius + len / 2 );
    GetDevice()->SetTransform( D3DTS_PROJECTION, &matProj );
}

void CPreviewState::UpdateLight()
{

    m_lightDir = Point3(sin(m_lightPhi) * cos(m_lightTheta), 
        cos(m_lightPhi) * cos(m_lightTheta), 
        sin(m_lightTheta));

    ComputeRotateMatrix(&m_shadowMapMatNoWorld, m_lightDir.x, m_lightDir.y, m_lightDir.z);
    m_shadowMapMat = m_worldMat * m_shadowMapMatNoWorld;

    RenderShadowmap();
}

Bssrdf CPreviewState::AdjustBssrdf()
{
    Bssrdf bssrdf = g_Bssrdf;
    float factor = sqrtf((m_sampled / m_totalArea)) * 0.3f * g_RenderOption.scale;
    bssrdf.scatter = g_Bssrdf.scatter * factor;
    bssrdf.absorb = g_Bssrdf.absorb * factor;
    bssrdf.UpdateParams();
    return bssrdf;
}

void CPreviewState::RenderShadowmap()
{
    DWORD t = GetTickCount();
    LPDIRECT3DSURFACE9 backSurface;
    GetDevice()->GetRenderTarget(0, &backSurface);

    // Render shadow map
    m_targetShadow.ApplyRenderTarget(GetDevice());

    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(1, 1, 1, 1), 1.0f, 0); 
    GetDevice()->ColorFill(m_targetShadow.GetSurface(), NULL, 0xFFFF0000);
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        m_effectShadowmap->SetMatrix("g_mProjectToLight", &m_shadowMapMat);
        m_effectShadowmap->SetFloatArray("g_LightDir", &m_lightDir.x, 3);

        m_effectShadowmap->SetTechnique("Shadowmap");

        DX_BEGIN_EFFECT(m_effectShadowmap);        
        for (size_t i = 0; i < m_meshes.size(); i++)
            m_meshes[i]->Render();
        DX_END_EFFECT(m_effectShadowmap);        

        GetDevice()->EndScene();
    }

    SetEffect(fxCopy);
    m_targetShadowMipmaps.ApplyRenderTarget(GetDevice());
    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(1, 1, 1, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        CGame2D* g2d = Get2D();
        float texSize[2] = { (float)(g_RenderOption.shadowMapWidth), (float)g_RenderOption.shadowMapHeight };
        g2d->BeginDraw();
        g2d->DrawTexture(m_targetShadow.GetTexture(), 0, 0, 
            (float)g_RenderOption.shadowMapWidth, (float)g_RenderOption.shadowMapHeight);
        g2d->EndDraw();
        GetDevice()->EndScene();
    }
    GetDevice()->SetRenderTarget(0, backSurface);
    backSurface->Release();

    times[0] = GetTickCount() - t;
}

void CPreviewState::InitGUI()
{
    m_dlgMain = new CGUIDialog(10.0f, 0.0f, 
        100, 30, false, 100, NULL);
    m_btLoadModel = m_dlgMain->AddButton(10, 10, 80, 26, 
        LoadGameImage("btLoadModel.png"));
    m_btParams = m_dlgMain->AddButton(10, 40, 80, 26, 
        LoadGameImage("btParams.png"));
    m_btRender = m_dlgMain->AddButton(10, 70, 80, 26, 
        LoadGameImage("btRender.png"));
    m_btBuiltin = m_dlgMain->AddButton(10, 100, 80, 26, 
        LoadGameImage("btBuiltin.png"));
    m_btLoadModel->SetFadeSpeed(10);
    m_btRender->SetFadeSpeed(10);
    m_btParams->SetFadeSpeed(10);
    m_btBuiltin->SetFadeSpeed(10);

    m_dlgParams = new CGUIDialog(120.0f, 0.0f, 
        300, 400, false, 100, LoadGameImage("dlgbk.png"));
    m_btCloseParams = m_dlgParams->AddButton(260, 25, 16, 16, 
        LoadGameImage("btClose.png"));
    LPD3DXFONT font1 = LoadFont("宋体", 12);
    LPD3DXFONT font2 = LoadFont("Arial", 14, FW_BOLD);
    CImage2D* imgSliderBar = LoadGameImage("sliderBar.png");
    CImage2D* imgSliderBlock = LoadGameImage("sliderBlock.png");

    float offsetX = 25, offsetY = 30;
    m_dlgParams->AddLabel(font1, offsetX, offsetY, "散射系数", 0xFFFFFFFF);
    for (int i = 0; i < 3; i++)
    {
        m_dlgParams->AddLabel(font2, offsetX + 10, offsetY + i * 20 + 15, 
            "R\0G\0B" + i * 2, 0xFFFFFF00);
        m_slSigmaS[i] = m_dlgParams->AddSlider(offsetX + 50, offsetY + i * 20 + 15, 
            160, 15, 12, 18, imgSliderBar, imgSliderBlock, 0.01f, 5);
        m_slSigmaS[i]->SetOffset(5, 3);
        m_slSigmaS[i]->SetValue(*(&g_Bssrdf.scatter.r + i));
    }

    offsetY += 80;
    m_dlgParams->AddLabel(font1, offsetX, offsetY, "吸收系数", 0xFFFFFFFF);
    for (int i = 0; i < 3; i++)
    {
        m_dlgParams->AddLabel(font2, offsetX + 10, offsetY + i * 20 + 15, 
            "R\0G\0B" + i * 2, 0xFFFFFF00);
        m_slSigmaA[i] = m_dlgParams->AddSlider(offsetX + 50, offsetY + i * 20 + 15, 
            160, 15, 12, 18, imgSliderBar, imgSliderBlock, 0, 0.1f);
        m_slSigmaA[i]->SetOffset(5, 3);
        m_slSigmaA[i]->SetValue(*(&g_Bssrdf.absorb.r + i));
    }


    offsetY += 80;
    m_dlgParams->AddLabel(font1, offsetX, offsetY, "漫反射", 0xFFFFFFFF);
    for (int i = 0; i < 3; i++)
    {
        m_dlgParams->AddLabel(font2, offsetX + 10, offsetY + i * 20 + 15, 
            "R\0G\0B" + i * 2, 0xFFFFFF00);
        m_slDiffuse[i] = m_dlgParams->AddSlider(offsetX + 50, offsetY + i * 20 + 15, 
            160, 15, 12, 18, imgSliderBar, imgSliderBlock, 0, 1);
        m_slDiffuse[i]->SetOffset(5, 3);
        m_slDiffuse[i]->SetValue(*(&g_Bssrdf.diffuse.r + i));
    }

    offsetY += 80;
    m_dlgParams->AddLabel(font1, offsetX, offsetY, "折射率", 0xFFFFFFFF);
    m_dlgParams->AddLabel(font2, offsetX + 10, offsetY + 15, "η", 0xFFFFFF00);
    m_slEta = m_dlgParams->AddSlider(offsetX + 50, offsetY + 15, 
        160, 15, 12, 18, imgSliderBar, imgSliderBlock, 1.1f, 1.7f);
    m_slEta->SetOffset(5, 3);
    m_slEta->SetValue(g_Bssrdf.eta);

    offsetY += 40;
    m_dlgParams->AddLabel(font1, offsetX, offsetY, "体积缩放", 0xFFFFFFFF);
    m_dlgParams->AddLabel(font2, offsetX + 10, offsetY + 15, "S", 0xFFFFFF00);
    m_slScale = m_dlgParams->AddSlider(offsetX + 50, offsetY + 15, 
        160, 15, 12, 18, imgSliderBar, imgSliderBlock, -5, 1);
    m_slScale->SetOffset(5, 3);
    m_slScale->SetValue(log(g_RenderOption.scale));


    m_dlgRender = new CGUIDialog(120.0f, 0.0f, 
        300, 300, false, 100, LoadGameImage("dlgbk.png"));
    m_btCloseRender = m_dlgRender->AddButton(260, 18, 16, 16, 
        LoadGameImage("btClose.png"));
    offsetX = 30;
    offsetY = 25;
    m_dlgRender->AddLabel(font1, offsetX, offsetY, "光照强度", 0xFFFFFFFF);
    m_dlgRender->AddLabel(font2, offsetX + 10, offsetY + 15, "P", 0xFFFFFF00);
    m_slLightPower = m_dlgRender->AddSlider(offsetX + 50, offsetY + 15, 
        160, 15, 12, 18, imgSliderBar, imgSliderBlock, -1.5f, 1.5f);
    m_slLightPower->SetOffset(5, 3);
    m_slLightPower->SetValue(log(g_RenderOption.lightPower));

    offsetY += 30;

    m_dlgRender->AddLabel(font1, offsetX, offsetY, "查找误差", 0xFFFFFFFF);
    m_dlgRender->AddLabel(font2, offsetX + 10, offsetY + 15, "W", 0xFFFFFF00);
    m_slMaxErr = m_dlgRender->AddSlider(offsetX + 50, offsetY + 15, 
        160, 15, 12, 18, imgSliderBar, imgSliderBlock, -0.1f, 2.5f);
    m_slMaxErr->SetOffset(5, 3);
    m_slMaxErr->SetValue(g_RenderOption.maxErr);

    offsetY += 30;

    m_btCPURender = m_dlgRender->AddButton(20, offsetY + 10, 80, 26, 
        LoadGameImage("btCPURender.png"));

    offsetY += 45;
    m_dlgRender->AddLabel(font1, offsetX, offsetY, "预览类型", 0xFFFFFFFF);
    m_listTechnique = m_dlgRender->AddList(offsetX, offsetY + 20, 200, 100, 4, LoadGameImage("dlgbk.png"), 
        LoadGameImage("list_sel.png"), NULL, NULL);
    m_listTechnique->Items.push_back("No Scattering");
    m_listTechnique->Items.push_back("Single Scattering Only");
    m_listTechnique->Items.push_back("Multiple Scattering Only");
    m_listTechnique->Items.push_back("Full Scatterings");
    m_listTechnique->SetFont(font1);
    m_listTechnique->SetItemHeight(18);
    m_listTechnique->SetOffset(15, 8);
    m_listTechnique->SetIndex(3);

    m_dlgBuiltin = new CGUIDialog(120.0f, 0.0f, 
        300, 300, false, 100, LoadGameImage("dlgbk.png"));

    offsetX = 30;
    offsetY = 25;

    m_btCloseBuiltin = m_dlgBuiltin->AddButton(260, 18, 16, 16, 
        LoadGameImage("btClose.png"));
    m_dlgBuiltin->AddLabel(font1, offsetX, offsetY, "内置标准材质列表", 0xFFFFFFFF);
    m_listMaterials = m_dlgBuiltin->AddList(offsetX, offsetY + 20, 200, 170, 8, LoadGameImage("dlgbk.png"), 
        LoadGameImage("list_sel.png"), NULL, NULL);

    m_listMaterials->Items.push_back("玉");
    m_listMaterials->Items.push_back("翡翠");
    m_listMaterials->Items.push_back("大理石");
    m_listMaterials->Items.push_back("皮肤1");
    m_listMaterials->Items.push_back("皮肤2");
    m_listMaterials->Items.push_back("全脂牛奶");
    m_listMaterials->Items.push_back("脱脂牛奶");
    m_listMaterials->Items.push_back("番茄酱");
    m_listMaterials->SetFont(font1);
    m_listMaterials->SetItemHeight(18);
    m_listMaterials->SetOffset(15, 10);
    m_listMaterials->SetIndex(-1);


    m_curDialog = NULL;
}

void CPreviewState::UpdateGUI()
{
    for (int i = 0; i < 3; i++)
    {
        m_slSigmaS[i]->SetValue(*(&g_Bssrdf.scatter.r + i));
        m_slSigmaA[i]->SetValue(*(&g_Bssrdf.absorb.r + i));
        m_slDiffuse[i]->SetValue(*(&g_Bssrdf.diffuse.r + i));
    }
    m_slEta->SetValue(g_Bssrdf.eta);
    m_slScale->SetValue(log(g_RenderOption.scale));
    m_slLightPower->SetValue(log(g_RenderOption.lightPower));
    m_slMaxErr->SetValue(g_RenderOption.maxErr);
}