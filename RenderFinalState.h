
#include "GameState.h"
#include "Point3.h"
#include "MeshList.h"
#include "RenderTarget.h"
#include "DynamicTexture.h"
#include "BoundBox.h"
#include "scatteroctree.h"
#include "MeshList.h"

class Game2D;
class C3DSModel;

class CRenderFinalState: public CGameState
{
private:
    float m_counter;
    
    CDynamicTexture m_target;
    CDynamicTexture m_normalMap;
    CDynamicTexture m_posMap;
    CDynamicTexture m_phong;

    size_t m_curBlock;
    size_t m_blockWidth;
    size_t m_blockHeight;

    BoundBox m_worldBound;

    UINT m_width;
    UINT m_height;

    ScatterOctree* m_octree;

    Point3 m_eyePos;

    D3DXMATRIX m_worldMat;

    bool m_bFinished;

    DWORD m_startTime;

    DWORD m_usedTime;

    MeshList& m_meshes;

    Bssrdf m_bssrdf; 

    LPD3DXEFFECT m_effectNormalmap;
    LPD3DXEFFECT m_effectPosmap;
    LPD3DXEFFECT m_effectPhong;

    CRenderTarget m_targetNormalMap;
    CRenderTarget m_targetPosMap;
    CRenderTarget m_targetPhong;

    void CreateRenderTargets();
    void ReleaseRenderTargets();
    void RenderToTexture();
    void ShowSaveImageDialog();
    void UpdateBssrdf();
public:
    CRenderFinalState(
        LPD3DXEFFECT effectNormalMap, 
        LPD3DXEFFECT effectPosMap, 
        LPD3DXEFFECT effectPhongMap,
        UINT imageWidth,
        UINT imageHeight,
        VERTEX_XYZNORM *samples,
        size_t sampledCount,
        MeshList& meshes);

    ~CRenderFinalState(void);

    void Start(CGameState * prior);
    void Draw2D(CGame2D * g2d);
    void PreRender();
    void Draw3D();
    void Animate(float elapsedTime);
    void Update(float elapsedTime);
    void DeviceLost();
    void DeviceReset();
};
