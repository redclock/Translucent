//--------------------------------------------------------------------------------------
// Phong.fx
// ��Ⱦ���ս��ͼ��
// By Yao Chunhui 2008.11
//--------------------------------------------------------------------------------------

#include "fresnel.fxh"

//--------------------------------------------------------------------------------------
// ȫ�ֱ���
//--------------------------------------------------------------------------------------

float3 g_LightDir;                         // ��������ϵ�¹ⷢ���ķ���
float3 g_eyePos;                           // �����ԭ���λ��
float4x4 g_mWorld;                         // ����任
float4x4 g_mWorldViewProjection;           // ����任 * �۲�任 * ͶӰ�任
float4x4 g_mProjectToLight;                // ͶӰ����Դ�ռ�ı任(���ȳ�����任)
float4x4 g_mProjectToLightNoWorld;         // ͶӰ����Դ�ռ�ı任(������任)
float3 g_LightBound;                       // ��Դ�����µ������Χ��
float g_LightPower;                        // ��Դǿ��
float g_Scale;

// ���ʲ���
float3 g_cDiffuse;                         // �趨��������ɫ  
float g_cSpecular;                         // ����ǿ��

// ��͸������
float3 g_SigmaT;
float3 g_SigmaTR;
float3 g_SigmaS;
float g_Zr;
float g_Zv;
float g_Eta;
float g_Phase;
float g_Fdr;

texture g_ShadowMap;
texture g_ShadowMapMipmaps;

// ��Mipmap��Shadow Map
sampler ShadowSampler = 
sampler_state
{
    Texture = <g_ShadowMap>;
    MipFilter = Linear;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Border;
    AddressV = Border;
    BorderColor = 0xFF000000;
};

// ��Mipmap��Shadow Map
sampler MipmapSampler = 
sampler_state
{
    Texture = <g_ShadowMapMipmaps>;
    MipFilter = Linear;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = Border;
    AddressV = Border;
    BorderColor = 0xFFFF0000;
};

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
    float4 Position    : POSITION;     // ����λ��  
    float4 Diffuse     : COLOR0;       // ��������ɫ
    float3 PosPixel    : TEXCOORD0;    // λ��
    float3 NormalPixel : TEXCOORD1;    // ����
    float3 EyeDirPixel : TEXCOORD2;    // ���۲�㷽��(���䷽��)
    float3 PosLightPixel: TEXCOORD3;   // ��Դ�ռ�λ��
};

//--------------------------------------------------------------------------------------
// Vertex shader: ������ת������Ļ����,����¼�����Ϣ
//--------------------------------------------------------------------------------------
VS_OUTPUT PhongVS(float4 vPos : POSITION,
                  float4 vNormal : NORMAL,
                  float4 Diffuse : COLOR0,
                  uniform bool useModelDiffuse
                  )
{
    VS_OUTPUT Output;
    float4 pos = mul(vPos, g_mWorldViewProjection);
    //float3 zero = mul(float4(0, 0, 0, 1), g_mWorld).xyz;
    float3 wnormal = mul(vNormal.xyz, (float3x3)g_mWorld);
    float3 wpos = mul(vPos, g_mWorld).xyz;
    float3 normal = normalize(wnormal);
    float diff = dot(g_LightDir, normal);
    if (useModelDiffuse)
        Output.Diffuse = diff * Diffuse;
    else
        Output.Diffuse = float4(diff * g_cDiffuse, 1);
    Output.Position = pos;
    Output.PosPixel = wpos;
    Output.NormalPixel = normal;
    Output.EyeDirPixel = normalize(g_eyePos - wpos);
    Output.PosLightPixel = mul(vPos, (float4x3)g_mProjectToLight);
    return Output;
}

// ����Phongģ�͹���
PS_OUTPUT PhongPS(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;

    Output.Color = In.Diffuse * g_LightPower;

    float3 H = normalize(g_LightDir + In.EyeDirPixel);
    float cost = dot(H, normalize(In.NormalPixel));

    if (g_cSpecular)
    {
        Output.Color += 0.3 * pow(cost, g_cSpecular);
    }

    float4 d = tex2D(ShadowSampler, In.PosLightPixel.xy);
    float depth = In.PosLightPixel.z - 0.01f;
    float isShadow = (d.x < depth);
    Output.Color *= !isShadow;
    
    return Output;
}

PS_OUTPUT SpecularPS(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output = (PS_OUTPUT) 0;

    float3 H = normalize(g_LightDir + In.EyeDirPixel);
    float cost = dot(H, normalize(In.NormalPixel));

    if (g_cSpecular)
    {
        Output.Color = 0.3 * pow(cost, g_cSpecular);

        float4 d = tex2D(ShadowSampler, In.PosLightPixel.xy);
        float depth = In.PosLightPixel.z - 0.001f;
        float isShadow = (d.x < depth);
        Output.Color *= !isShadow;
    }
    
    return Output;
}


float3 SingleScattering(float3 position, float3 wo, float3 normal)
{
    // ���㵥��ɢ��    
    
    float3 color = {0, 0, 0}; 
    const int nSamples = 50; 
    float meanSigmaT = ((g_SigmaT.x + g_SigmaT.y + g_SigmaT.z) / 3);
    float phase = 1 / 3.14; //Phase(-dot(wo, g_LightDir), g_Phase);

    for (int i = 0; i < nSamples; i++)
    {
        float so = -log((i + 0.5) / nSamples) / meanSigmaT;
        float3 xs = position + wo * so;
        float3 xslight = mul(float4(xs, 1), g_mProjectToLightNoWorld).xyz ;
        float4 d = tex2D(ShadowSampler, xslight.xy);
        float si = abs(-d.x + xslight.z) * g_LightBound.z;
        
        si = si * d.y / sqrt(1 - 1.0 / (g_Eta * g_Eta) * (1 - d.y * d.y));
        color += (d.y * exp(-(si + so) * g_SigmaT)) / exp(-so * g_SigmaT);
    }
    
    color *= phase * g_SigmaS / nSamples / g_SigmaT;
    return color;
}

float3 Rd(float r)
{
    float dr = sqrt(g_Zr * g_Zr + r * r);
    float dv = sqrt(g_Zv * g_Zv + r * r);

    float3 rd1 = g_Zr * (1 + g_SigmaTR * dr) 
        * exp(g_SigmaTR * (-dr)) / (dr * dr * dr);
    float3 rd2 = g_Zv * (1 + g_SigmaTR * dv) 
        * exp(g_SigmaTR * (-dv)) / (dv * dv * dv);

    return (rd1 + rd2);
}

float3 SamplePoint(sampler samp, float3 pos, float2 offset, float lod)
{
    float2 pd = float2(1 / 512.0, 1 / 512.0);
    offset *= pd;
    float4 d = tex2Dlod(samp, float4(pos + offset, 0, lod));
    float3 vecTo = float3(offset, max(0, pos.z - d.x)) * g_LightBound;
    float len = length(vecTo);
    
    return Rd(len) * d.y;
}

float3 MultiScattering(float3 position)
{
    float3 poslight = mul(float4(position, 1), g_mProjectToLightNoWorld).xyz ;
    float3 color = {0, 0, 0}; 
    float area = g_LightBound.x * g_LightBound.y / (512 * 512);
    const int size = 4;
    for (int i = -size; i < size; i++)
    for (int j = -size; j < size; j++)
    {
        float n = max(1, abs(i) + abs(j)) / (g_Scale) * 2.0;
        if (n > 0 && n < 0.01) continue;
        float lod = clamp(n - 1, 0, 7);
        float sizelod = pow(2, lod);
        float arealod = sizelod * sizelod; 

        color += arealod * SamplePoint(MipmapSampler, poslight, sizelod * float2(i, j), lod);
    }

     
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, 0), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, 1), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, 2), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(1, 0), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(2, 0), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, -1), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, -2), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-1, 0), 0);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-2, 0), 0);
   	//
    //color += SamplePoint(ShadowSampler, poslight, float2(1.5, -1.5), 1);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-1.5, -1.5), 1);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-1.5, 1.5), 1);
   	//color += SamplePoint(ShadowSampler, poslight, float2(1.5, 1.5), 1);
   
   	//color += SamplePoint(ShadowSampler, poslight, float2(4, -4), 2);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-4, -4), 2);
    //color += SamplePoint(ShadowSampler, poslight, float2(-4, 4), 2);
   	//color += SamplePoint(ShadowSampler, poslight, float2(4, 4), 2);
   
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, 8), 3);
   	//color += SamplePoint(ShadowSampler, poslight, float2(0, 8), 3);
   	//color += SamplePoint(ShadowSampler, poslight, float2(8, 0), 3);
   	//color += SamplePoint(ShadowSampler, poslight, float2(-8, 0), 3);

    return color * area * (1 - g_Fdr) / g_Fdr;// / 49;
}

PS_OUTPUT SingleScatterPS(VS_OUTPUT In) 
{
    PS_OUTPUT Output = (PS_OUTPUT)0;

    float3 eyeDir = normalize(In.EyeDirPixel);
    float3 normal = normalize(In.NormalPixel);
    float3 wo = normalize(refract (-eyeDir, normal, 1 / g_Eta));

    // ���㵥��ɢ��    
    Output.Color.rgb += SingleScattering(In.PosPixel, wo, normal);
    Output.Color += saturate(PhongPS(In).Color);
    Output.Color.rgb *= g_LightPower;
    Output.Color.a = 1;
    
    return Output;
}

PS_OUTPUT MultiScatterPS(VS_OUTPUT In) 
{
    PS_OUTPUT Output = (PS_OUTPUT)0;

    // ������ɢ��    
    Output.Color.rgb += MultiScattering(In.PosPixel);
    Output.Color += saturate(PhongPS(In).Color);
    Output.Color.rgb *= g_LightPower;
    Output.Color.a = 1;
   
    return Output;
}

PS_OUTPUT FullScatterPS(VS_OUTPUT In) 
{
    PS_OUTPUT Output = (PS_OUTPUT)0;

    float3 eyeDir = normalize(In.EyeDirPixel);
    float3 normal = normalize(In.NormalPixel);
    float3 wo = normalize(refract (-eyeDir, normal, 1 / g_Eta));
    
    // ���㵥��ɢ��    
    Output.Color.rgb += SingleScattering(In.PosPixel, wo, normal);

    // ������ɢ��    
    Output.Color.rgb += MultiScattering(In.PosPixel);
    Output.Color.rgb *= g_LightPower;
    Output.Color += saturate(PhongPS(In).Color);
    Output.Color.rgb = pow(Output.Color.rgb, 1/2.2);
    Output.Color.a = 1;
    
    return Output;
}

//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique PreviewPhong
{
    pass P0
    {          
        VertexShader = compile vs_1_1 PhongVS(true);
        PixelShader  = compile ps_2_0 PhongPS();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}

technique PreviewSingleScattering
{
    pass P0
    {         
        VertexShader = compile vs_3_0 PhongVS(false);
        PixelShader  = compile ps_3_0 SingleScatterPS();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}

technique PreviewMultiScattering
{
      pass P0
      {         
          VertexShader = compile vs_3_0 PhongVS(false);
          PixelShader  = compile ps_3_0 MultiScatterPS();
          AlphaBlendEnable = False;
          AlphaTestEnable = False;
      }
}

technique PreviewFullScattering
{
    pass P0
    {         
        VertexShader = compile vs_3_0 PhongVS(false);
        PixelShader  = compile ps_3_0 FullScatterPS();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}

technique SpecularPhong
{
    pass P0
    {          
        VertexShader = compile vs_1_1 PhongVS(false);
        PixelShader  = compile ps_2_0 PhongPS();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}