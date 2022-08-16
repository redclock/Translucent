//--------------------------------------------------------------------------------------
// ShadowMap.fx
// 渲染阴影图的效果文件
// By Yao Chunhui 2008.11
//--------------------------------------------------------------------------------------

#include "fresnel.fxh"

//--------------------------------------------------------------------------------------
// 全局变量
//--------------------------------------------------------------------------------------

float4x4 g_mProjectToLight;                 // 投影到光源空间的变换
float3 g_LightDir;                          // 世界坐标下的光源方向

//--------------------------------------------------------------------------------------
// Pixel shader 输出
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 Color : COLOR0;    
};

//--------------------------------------------------------------------------------------
// Vertex shader 输出
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position    : POSITION;           // 顶点位置 
    float2 PixelInfo   : TEXCOORD0;          // 到光源的Z值和光强
};

//--------------------------------------------------------------------------------------
// Vertex shader: 将顶点转化到光源坐标,并计算Z值
//--------------------------------------------------------------------------------------
VS_OUTPUT ShadowMapVS(float4 vPos : POSITION, float4 vNormal : NORMAL)
{
    VS_OUTPUT Output;
    float4 pos = mul(vPos, g_mProjectToLight);
    float3 wnormal = mul(vNormal.xyz, (float3x3)g_mProjectToLight);
    float3 normal = normalize(wnormal);
    float diff = -normal.z;
    //diff *= (1 - Fresnel(diff, 1 / 1.3));
    Output.Position = pos;
    Output.PixelInfo = float2(pos.z, diff);
    return Output;    
}

//--------------------------------------------------------------------------------------
// Pixel shader: R通道记录Z值, G通道记录光强
//--------------------------------------------------------------------------------------
PS_OUTPUT ShadowMapPS(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.Color = float4(In.PixelInfo, 0, 1);
    return Output;
}

//--------------------------------------------------------------------------------------
// 渲染 shadow map
//--------------------------------------------------------------------------------------
technique Shadowmap
{
    pass P0
    {          
        VertexShader = compile vs_2_0 ShadowMapVS();
        PixelShader  = compile ps_2_0 ShadowMapPS();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}

