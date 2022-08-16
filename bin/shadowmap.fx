//--------------------------------------------------------------------------------------
// ShadowMap.fx
// ��Ⱦ��Ӱͼ��Ч���ļ�
// By Yao Chunhui 2008.11
//--------------------------------------------------------------------------------------

#include "fresnel.fxh"

//--------------------------------------------------------------------------------------
// ȫ�ֱ���
//--------------------------------------------------------------------------------------

float4x4 g_mProjectToLight;                 // ͶӰ����Դ�ռ�ı任
float3 g_LightDir;                          // ���������µĹ�Դ����

//--------------------------------------------------------------------------------------
// Pixel shader ���
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 Color : COLOR0;    
};

//--------------------------------------------------------------------------------------
// Vertex shader ���
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position    : POSITION;           // ����λ�� 
    float2 PixelInfo   : TEXCOORD0;          // ����Դ��Zֵ�͹�ǿ
};

//--------------------------------------------------------------------------------------
// Vertex shader: ������ת������Դ����,������Zֵ
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
// Pixel shader: Rͨ����¼Zֵ, Gͨ����¼��ǿ
//--------------------------------------------------------------------------------------
PS_OUTPUT ShadowMapPS(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.Color = float4(In.PixelInfo, 0, 1);
    return Output;
}

//--------------------------------------------------------------------------------------
// ��Ⱦ shadow map
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

