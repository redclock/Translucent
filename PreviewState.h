
#include "GameState.h"
#include "Point3.h"
#include "MeshList.h"
#include "RenderTarget.h"
#include "BoundBox.h"
#include "DynamicTexture.h"
#include "bssrdf.h"
#include "GameUI.h"
#include "DefaultTextMesh.h"

class Game2D;
class C3DSModel;

class CPreviewState: public CGameState
{
private:
    
    CGUIDialog* m_dlgMain;
    CGUIDialog* m_dlgParams;
    CGUIDialog* m_dlgRender;
    CGUIDialog* m_dlgBuiltin;

    CGUIDialog* m_curDialog;

    CGUIButton* m_btCloseParams;
    CGUIButton* m_btLoadModel;
    CGUIButton* m_btRender;
    CGUIButton* m_btParams;
    CGUIButton* m_btBuiltin;

    CGUIButton* m_btCloseRender;
    CGUIButton* m_btCPURender;
    CGUIList* m_listTechnique;

    CGUISlider* m_slSigmaS[3];
    CGUISlider* m_slSigmaA[3];
    CGUISlider* m_slDiffuse[3];
    CGUISlider* m_slEta;
    CGUISlider* m_slScale;
    CGUISlider* m_slLightPower;
    CGUISlider* m_slMaxErr;

    CGUIButton* m_btCloseBuiltin;
    CGUIList* m_listMaterials;

    C3DSModel* m_model;
    CDefaultTextMesh* m_dfMesh;
    float m_counter;
    float m_radius;
    UINT m_lastx, m_lasty;
    bool m_lastDown[2];
    D3DXMATRIX m_viewMat;
    Point3 m_center;
    Point3 m_lightDir;
    float m_anglex;
    float m_anglez;
    float m_lightTheta;
    float m_lightPhi;

    DWORD times[4];

    LPD3DXEFFECT m_effectNormalmap;
    LPD3DXEFFECT m_effectPosmap;
    LPD3DXEFFECT m_effectPhong;
    LPD3DXEFFECT m_effectShadowmap;

    CRenderTarget m_targetShadow;

    const static int MIPMAP_COUNT = 8;
    
    CRenderTarget m_targetShadowMipmaps;

    CRenderTarget m_targetShadow2;
    CDynamicTexture m_shadowMap;

    MeshList m_meshes;

    Point3* m_samplePoints;
    Point3* m_sampleNormals;

    BoundBox m_worldBound;

    VERTEX_XYZNORM *m_pointsBuf;
    size_t m_sampled;

    Point3 m_eyePos;
    bool m_bShowPoint;

    float m_totalArea;
    D3DXMATRIX m_worldMat;
    D3DXMATRIX m_shadowMapMat;
    D3DXMATRIX m_shadowMapMatNoWorld;

    Point3 m_lightBound;

    void DrawBoundBox(const BoundBox& box, DWORD color);
    void DrawSceneBox(const BoundBox& box, DWORD color);
    void DrawPoints();
    void CreateSamples(size_t count);
    void ComputeSampleIrradiance();
    void LoadModel();
    void ShowOpenModelDialog();
    void ShowSaveImageDialog();
    void ResetProjectionMat();
    void UpdateLight();
    Bssrdf AdjustBssrdf();
    void ComputeRotateMatrix(D3DXMATRIX* out, float dx, float dy, float dz);
    void RenderShadowmap();
    void InitGUI();
    void UpdateGUI();

public:
    CPreviewState(void);
    ~CPreviewState(void);

    void Start(CGameState * prior);
    void Draw2D(CGame2D * g2d);
    void PreRender();
    void Draw3D();
    void Animate(float elapsedTime);
    void Update(float elapsedTime);
    void DeviceLost();
    void DeviceReset();
};
