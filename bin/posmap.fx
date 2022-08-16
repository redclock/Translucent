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

float4x4 g_mWorldViewProjection;                 // World * View * Projection matrix
float4x4 g_mPosmapTransform;                 // Convert position to  0..1

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
    float4 Position : POSITION;   // vertex position 
    float3 PosPixel : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT vsMain (float4 vPos : POSITION)
{
    VS_OUTPUT Output;
    float4 pos = mul(vPos, g_mWorldViewProjection);
    float3 wpos = mul(vPos, g_mPosmapTransform).xyz;
    Output.Position = pos;
    Output.PosPixel = wpos;
    return Output;    
}

PS_OUTPUT psMain(VS_OUTPUT In) 
{ 
    PS_OUTPUT Output;
    
    Output.Color = float4(In.PosPixel, 1);
    
    return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------
technique Phong
{
    pass P0
    {          
        VertexShader = compile vs_1_1 vsMain();
        PixelShader  = compile ps_2_0 psMain();
        AlphaBlendEnable = False;
        AlphaTestEnable = False;
    }
}
