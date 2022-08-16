#include "MyGlobal.h"

D3DFORMAT g_PosmapFormat;
D3DFORMAT g_ShadowmapFormat;

static const Bssrdf jade =
{
    100.0f, 
    Color(0.43f, 0.49f, 0.45f) * 0,
    Color(3.00f, 2.50f, 1.00f), 
    Color(0.001f, 0.0031f, 0.0021f), 
    1.2f, 0.9f
}; 

static const Bssrdf jade_green =
{
    100.0f,
    Color(0.83f, 0.79f, 0.75f) * 0.0,
    Color(2.00f, 0.50f, 1.50f), 
    Color(0.007f, 0.001f, 0.005f), 
    1.3f, 0.7f
}; 

static const Bssrdf marble =
{
    80.0f, 
    Color(0.83f, 0.79f, 0.75f) * 0.0f,
    Color(2.19f, 2.62f, 3.00f), 
    Color(0.0021f, 0.0041f, 0.0071f), 
    1.5f, 0.7f
}; 

static const Bssrdf skin1 =
{
    0.0f, 
    Color(0.44f, 0.22f, 0.13f),
    Color(0.74f, 0.88f, 1.01f), 
    Color(0.032f, 0.17f, 0.48f), 
    1.3f, 0.5f
}; 
static const Bssrdf skin2 =
{
    0.0f, 
    Color(0.58f, 0.44f, 0.34f),
    Color(1.09f, 1.59f, 1.79f), 
    Color(0.013f, 0.070f, 0.145f), 
    1.35f, 0.5f
}; 

static const Bssrdf wholemilk =
{
    10.0f, 
    Color(0.91f, 0.88f, 0.76f) * 0.3f,
    Color(2.55f, 3.21f, 3.77f),
    Color(0.0011f, 0.0024f, 0.014f), 
    1.3f, 0.9f
}; 

static const Bssrdf skimmilk =
{
    10.0f, 
    Color(0.91f, 0.88f, 0.76f) * 0.3f,
    Color(0.70f, 1.22f, 1.90f),
    Color(0.0014f, 0.0025f, 0.0142f), 
    1.3f, 0.9f
}; 

static const Bssrdf ketchup =
{
    10.0f, 
    Color(0.16f, 0.01f, 0.00f) * 0.5f,
    Color(0.18f, 0.07f, 0.03f),
    Color(0.061f, 0.97f, 1.45f), 
    1.3f, 0.9f
}; 


Bssrdf g_BuiltinBssrdfs[] =
{
    jade, jade_green, marble, skin1, skin2, wholemilk, skimmilk, ketchup 
};

Bssrdf g_Bssrdf = marble;

//size_t sampleCount;
//UINT shadowMapWidth;
//UINT shadowMapHeight;
//float maxErr;
//float transFactor;
//float scale;
//float lightPower;
//bool bShowSamplePoints;
//bool bShowBoundBox;
//bool bShowShadowMap;
//bool bShowUI;

extern RenderOption g_RenderOption = 
{ 
    100000,
    512,
    512,
    0.3f, 
    1.0f,
    1.0f,
    1.0f,
    false,
    false,
    false,
    true,
};
extern StateInfo g_StateInfo = 
{ 
    "jingwu2.3DS",
    true,
};
