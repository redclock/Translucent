//--------------------------------------------------------------------------------------
// File: BasicHLSL.fx
//
// The effect file for the BasicHLSL sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

float4x4 g_mWorld;                          // World matrix for object
float4x4 g_mWorldViewProjection;                 // World * View * Projection matrix

//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 Color : COLOR0;  // Pixel color    
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float3 NormalPixel   : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT vsMain(float4 vPos : POSITION,
                      float4 vNormal : NORMAL,
                      float4 Diffuse : COLOR0
					 )
{
    VS_OUTPUT Output;
    float4 pos = mul(vPos, g_mWorldViewProjection);
    float3 zero = mul(float4(0, 0, 0, 1), (float4x3)g_mWorld);
    float3 normal = normalize(mul(vNormal, (float4x3)g_mWorld) - zero);
    Output.Position = pos;
    Output.NormalPixel = normal;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT psMain(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    Output.Color.rgb = normalize(float3(0.5 * In.NormalPixel.x + 0.5,
									0.5 * In.NormalPixel.y + 0.5,
									0.5 * In.NormalPixel.z + 0.5));
    
    Output.Color.a = 1;
    //float w = In.NormalPixel.w; 
    //Output.Color.a = In.NormalPixel;   
	//Output.ColorDif = In.Diffuse;
    return Output;
}

technique GenNormalMap
{
    pass P0
    {          
        VertexShader = compile vs_1_1 vsMain();
        PixelShader  = compile ps_2_0 psMain();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}