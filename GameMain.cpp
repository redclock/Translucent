
// CRT's memory leak detection
#if defined(DEBUG) | defined(_DEBUG)
#include <crtdbg.h>
#endif
#include <stdio.h>

#include "GameEngine.h"
#include "StageState.h"
#include "PreviewState.h"
#include "RenderFinalState.h"
#include "CommandThread.h"
#include "MyGlobal.h"

D3DFORMAT SelectFormat(D3DFORMAT* preferList, int count)
{
    LPDIRECT3DDEVICE9 device = GetDevice();
    for (int i = 0; i < count; i++)
    {
        bool succ = SUCCEEDED(CheckResourceFormatSupport(
            device, preferList[i], D3DRTYPE_TEXTURE, D3DUSAGE_RENDERTARGET))
            && SUCCEEDED(CheckResourceFormatSupport(
            device, preferList[i], D3DRTYPE_TEXTURE, D3DUSAGE_DYNAMIC));
        if (succ)
        {
            return preferList[i];
        }
    }
    return D3DFMT_UNKNOWN;
}


bool CheckHardwareSupport()
{
    //检测Caps
    D3DCAPS9 caps;
    LPDIRECT3DDEVICE9 device = GetDevice();
    device->GetDeviceCaps(&caps);

    if( caps.VertexShaderVersion < D3DVS_VERSION(3, 0) )
    {
    	printf("Vertex shader 2.0 required\n");
    	return false;
    }	
    if( caps.PixelShaderVersion < D3DPS_VERSION(3, 0) )
    {
    	printf("Pixel shader 2.0 required\n");
    	return false;
    }
    
    D3DFORMAT posmapFormats[] = {D3DFMT_A2B10G10R10, D3DFMT_A2R10G10B10, D3DFMT_A8R8G8B8};
    g_PosmapFormat = SelectFormat(posmapFormats, 3);
    if (g_PosmapFormat == D3DFMT_UNKNOWN)
    {
        printf("Texture format not supported");
        return false;
    }

    D3DFORMAT shadowmapFormats[] = {D3DFMT_G32R32F, D3DFMT_G16R16F, g_PosmapFormat};
    g_ShadowmapFormat = SelectFormat(shadowmapFormats, 3);

    //g_PosmapFormat = D3DFMT_A8R8G8B8;
    //g_ShadowmapFormat = g_PosmapFormat;
    return true;
}


//--------------------------------------------------------------------------------------
// Entry point to the program.
//--------------------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	srand(timeGetTime());
	
	if (InitGame("Translucent rendering", 100, 100, 600, 600, true, true))
	{
        if (CheckHardwareSupport())
        {
		    //GetStateStack()->Push(new CStageState());
            GetStateStack()->Push(new CPreviewState());
            //GetStateStack()->Push(new CRenderFinalState(NULL, NULL, NULL));
            
            CCommandThread thread;
            thread.Start();
		    RunGame();
        }
        else
        {
            MessageBox(GetWindow(), "当前硬件环境无法满足运行要求!", "错误", 0);
            Cleanup();
        }
	}
	
	return 0;
}