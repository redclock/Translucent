#pragma once

#include "GameState.h"
#include "RenderTarget.h"

class Game2D;
class C3DSModel;

class CStageState: public CGameState
{
private:
    C3DSModel* m_model;
    CRenderTarget m_normalMap;
    CRenderTarget m_diffuseMap;
    CRenderTarget m_shadowMap;

    LPDIRECT3DTEXTURE9 m_stepTexture;

    LPD3DXEFFECT m_effect;
    D3DXMATRIXA16 m_lightMat;
    D3DXMATRIXA16 m_texMat;
    float m_counter;

    void RenderNormalMap();
    void RenderShadowMap();
public:
	CStageState(void);
	~CStageState(void);

	void Start(CGameState * prior);
	void Draw2D(CGame2D * g2d);
    void PreRender();
	void Draw3D();
	void Animate(float elapsedTime);
	void Update(float elapsedTime);
	void DeviceLost();
	void DeviceReset();
};
