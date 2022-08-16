#include "StageState.h"
#include "3DSModel.h"
#include "GameEngine.h"

CStageState::CStageState(void) : CGameState("stage")
{
    m_model = new C3DSModel();
    m_model->LoadFromFile("bunny.3ds");
    m_effect = NULL;
    m_counter = 0;
    m_stepTexture = LoadGameImage("step.png")->GetTexture();
}

CStageState::~CStageState(void)
{
    delete m_model;
    if (m_effect)
        m_effect->Release();
}

void CStageState::Start(CGameState * prior)
{
    CGameState::Start(prior);
    LPDIRECT3DDEVICE9 device = GetDevice();

    //if(FAILED(CheckResourceFormatSupport(device, D3DFMT_D24S8, D3DRTYPE_TEXTURE, D3DUSAGE_DEPTHSTENCIL)))
    //{
    //    MessageBox(NULL, "Device/driver does not support hardware shadow maps!", 
    //        "ERROR", MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    //    exit(1);
    //    return;
    //}
    
    if (!m_normalMap.CreateTexture(GetDevice(), GetScreenWidth(), GetScreenHeight()))
        return;

    if (!m_diffuseMap.CreateTexture(GetDevice(), GetScreenWidth(), GetScreenHeight()))
        return;

    if (!m_shadowMap.CreateTexture(GetDevice(), GetScreenWidth(), GetScreenHeight()))
        return;

     float fOffsetX = 0.5f + (0.5f / 640.0f);
     float fOffsetY = 0.5f + (0.5f / 480.0f);
     D3DXMATRIX texScaleBiasMat(0.5f, 0.0f, 0.0f, 0.0f,
                                0.0f, -0.5f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                fOffsetX, fOffsetY, 0.0f, 1.0f );

    memcpy(&m_texMat, &texScaleBiasMat, sizeof(D3DXMATRIX));
    m_effect = CreateEffectFromFile("a.fx");
}

void CStageState::Draw2D(CGame2D * g2d)
{
    CGameState::Draw2D(g2d);
    SetEffect(fxBlend);
    m_effect->SetTexture("g_NormalMap", m_normalMap.GetTexture());
    m_effect->SetTexture("g_DiffuseMap", m_diffuseMap.GetTexture());
    m_effect->SetTexture("g_StepMap", m_stepTexture);
    m_effect->SetTexture("g_ShadowMap", m_shadowMap.GetTexture());
    m_effect->SetTechnique("RenderLine");
    //m_effect->SetTechnique("RenderNormal");
    
    // Apply the technique contained in the effect 
    DX_BEGIN_EFFECT(m_effect);
        g2d->DrawTexture(m_normalMap.GetTexture(), 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight());
        //g2d->DrawTexture(m_normalMap.GetTexture(), 0, 0, 
        //    (float)GetScreenWidth() / 2, (float)GetScreenHeight());
        //g2d->DrawTexture(m_diffuseMap.GetTexture(), (float)GetScreenWidth() / 2, 0, 
        //    (float)GetScreenWidth() / 2, (float)GetScreenHeight());
        g2d->Flush();
    DX_END_EFFECT(m_effect);

    LPD3DXFONT font = LoadFont("Arial", 20);
    g2d->DrawString(font, "dsadsa", 100, 300, 0xFF00FFFF);
}


void CStageState::PreRender()
{
    LPDIRECT3DSURFACE9 backSurface;
    GetDevice()->GetRenderTarget(0, &backSurface);
    RenderShadowMap();
    RenderNormalMap();
    GetDevice()->SetRenderTarget(0, backSurface);
}

void CStageState::Draw3D()
{
    CGameState::Draw3D();
    GetCamera()->SetEye(150.0f, 150.0f, 100.0f);
    GetCamera()->SetLookat(0, 0, 30);
    GetCamera()->SetUp(0, 0, 1);
    GetCamera()->UpdateMatrix();
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    GetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    GetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    GetDevice()->SetTransform(D3DTS_WORLD, &mat);
   // TextureSelect(m_target);
   // m_model->Render();
}

void CStageState::Animate(float elapsedTime)
{
    CGameState::Animate(elapsedTime);
    m_counter += elapsedTime;
}

void CStageState::Update( float elapsedTime )
{
    CGameState::Update(elapsedTime);
}

void CStageState::DeviceLost()
{

}

void CStageState::DeviceReset()
{
    GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );
    GetDevice()->SetRenderState( D3DRS_DITHERENABLE , TRUE );
    GetDevice()->SetRenderState( D3DRS_ZWRITEENABLE , TRUE );

}

void CStageState::RenderNormalMap()
{
    SetEffect(fxCopy);

    GetCamera()->SetEye(558.0f * sin(-m_counter / 4), 558.0f * cos(-m_counter/ 4), 100.0f);
    GetCamera()->SetLookat(0, 0, 0);
    GetCamera()->SetUp(0, 0, 1);
    GetCamera()->UpdateMatrix();

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    D3DXMATRIX mat;
    D3DXMatrixRotationZ(&mat, 0);
    GetDevice()->SetTransform(D3DTS_WORLD, &mat);

    GetDevice()->GetTransform(D3DTS_WORLD, &mWorld);
    GetDevice()->GetTransform(D3DTS_VIEW, &mView);
    GetDevice()->GetTransform(D3DTS_PROJECTION, &mProj);
    D3DXMATRIXA16 mWVP = mWorld * mView * mProj;
    D3DXMATRIXA16 mWV = mWorld * mView;
    D3DXMATRIXA16 mLT = m_lightMat * m_texMat;
    m_effect->SetMatrix("g_mWorldViewProjection", &mWVP);
    m_effect->SetMatrix("g_mWorldView", &mWV);
    m_effect->SetMatrix("g_mWorld", &mWorld);

    m_effect->SetMatrix("g_mLightWorldViewProjection", &mLT);

    D3DXVECTOR3 Vec(0, 0, 0);
    D3DXVECTOR4 Vec2;
    D3DXVec3Transform(&Vec2, &Vec, &mLT);
    m_effect->SetTexture("g_ShadowMap", m_shadowMap.GetTexture());

    float offset[2] = {1.0f / 640.0f, 1.0f / 480.0f};
    m_effect->SetFloatArray("g_fOffset", offset, 2);
    
    m_normalMap.ApplyRenderTarget(GetDevice());

    m_effect->SetTechnique("GenNormalMap");

    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        DX_BEGIN_EFFECT(m_effect);        
            m_model->Render();
        DX_END_EFFECT(m_effect);        
        GetDevice()->EndScene();
    }

    m_diffuseMap.ApplyRenderTarget(GetDevice());
    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.0f, 0); 
    m_effect->SetTechnique("GenDiffuseMap");

    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        DX_BEGIN_EFFECT(m_effect);        
            m_model->Render();
        DX_END_EFFECT(m_effect);        
        GetDevice()->EndScene();
    }
}

void CStageState::RenderShadowMap()
{
    SetEffect(fxCopy);

    //GetCamera()->SetEye(358.0f, 358.0f, 100.0f);
    //GetCamera()->SetLookat(0, 0, 0);
    //GetCamera()->SetUp(0, 0, 1);
    //GetCamera()->UpdateMatrix();

    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    D3DXMATRIX mat;
    
    D3DXMatrixRotationZ(&mat, 0);
    GetDevice()->SetTransform(D3DTS_WORLD, &mat);

    GetDevice()->GetTransform(D3DTS_WORLD, &mWorld);
    //GetDevice()->GetTransform(D3DTS_VIEW, &mView);
    D3DXVECTOR3 EyePos (358.0f * sin(m_counter / 3), 358.0f * cos(m_counter/ 3), 100.0f);
    D3DXVECTOR3 EyeLookat (0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 EyeUp (0.0f, 0.0f, 1.0f);
    D3DXMatrixLookAtLH(&mView, &EyePos, &EyeLookat, &EyeUp);
    GetDevice()->GetTransform(D3DTS_PROJECTION, &mProj);
    
    m_lightMat = mWorld * mView * mProj;

    m_effect->SetMatrix("g_mLightWorldViewProjection", &m_lightMat);
    
    m_effect->SetFloatArray("g_LightDir", &EyePos.x, 3);
//    LPDIRECT3DSURFACE9 depthSurface;
//    GetDevice()->GetDepthStencilSurface(&depthSurface);
    m_shadowMap.ApplyRenderTarget(GetDevice());
//    GetDevice()->SetDepthStencilSurface(m_surfaceZ);
    m_effect->SetTechnique("GenShadowMap");

    GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(1, 1, 1, 1), 1.0f, 0); 
    if (SUCCEEDED(GetDevice()->BeginScene()))
    {
        // Apply the technique contained in the effect 
        UINT cPasses;
        if (SUCCEEDED(m_effect->Begin(&cPasses, 0)))
        {
            for (UINT iPass = 0; iPass < cPasses; iPass++)
            {
                if (SUCCEEDED(m_effect->BeginPass(iPass)))
                {
                    m_model->Render();
                    m_effect->EndPass();
                }
            }
            m_effect->End();
        }
        GetDevice()->EndScene();
    }
//    GetDevice()->SetDepthStencilSurface(depthSurface);
}
