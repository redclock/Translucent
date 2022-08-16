#pragma once
#include <d3dx9.h>
#include "bssrdf.h"

struct RenderOption
{
    size_t sampleCount;
    UINT shadowMapWidth;
    UINT shadowMapHeight;
    float maxErr;
    float transFactor;
    float scale;
    float lightPower;
    bool bShowSamplePoints;
    bool bShowBoundBox;
    bool bShowShadowMap;
    bool bShowUI;
};

struct StateInfo
{
    char modelFile[MAX_PATH];
    bool isPreview;
};

extern D3DFORMAT g_PosmapFormat;
extern D3DFORMAT g_ShadowmapFormat;
extern Bssrdf g_Bssrdf;
extern RenderOption g_RenderOption;
extern StateInfo g_StateInfo;
extern Bssrdf g_BuiltinBssrdfs[];