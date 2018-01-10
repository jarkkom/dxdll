/* Copyright (C) 2004-2018 DXDLL authors, see included COPYING file */

#ifndef _PROXYDLL_H_
#define _PROXYDLL_H_

#include "D3D8.h"
#include <stdlib.h>
#include <windows.h>
#include <d3dx8.h>

#include "DXUtil.h"
#include <D3DFont.h>
#include "shaders.h"

/*
#include "D3DApp.h"
#include "D3DX8.h"
#include "D3DUtil.h"
#include "DXUtil.h"
*/
static HHOOK hkb=NULL;
#define VK_APPLICATION		0x5D // Application key (extended)
bool keyModPressed = FALSE;	// Application key pressed?

#define WIN32_LEAN_AND_MEAN
#define NUM_TEX 32

// Detail textures (multitextured)
#define TEX_DET_GROUND 0
#define TEX_DET_WATER 1
#define TEX_DET_GROUNDU 16
#define TEX_DET_WATERU 17

#define TEX_MAPBACK 21
#define TEX_TRACER 27
#define TEX_RAIN 28

// nvgoggles
#define TEX_NVG1 22
#define TEX_NVG2 23
#define TEX_NVG3 24
#define TEX_NVG4 25
#define TEX_NVG5 26

// Sky textures
#define TEX_SKY1 18
#define TEX_SKY2 19
#define TEX_SKY3 20

// Original water textures as loaded
#define TEX_WATER1 2
#define TEX_WATER2 3
#define TEX_WATER3 4
#define TEX_WATER4 5
#define TEX_WATER5 6
#define TEX_WATER6 7
#define TEX_WATER7 8

// Final water textures from UpdateTextures
#define TEX_WATERU1 9
#define TEX_WATERU2 10
#define TEX_WATERU3 11
#define TEX_WATERU4 12
#define TEX_WATERU5 13
#define TEX_WATERU6 14
#define TEX_WATERU7 15

// Texture CRC's
#define CRC_DET_GROUND 1318925501
#define CRC_DET_WATER  913095462
#define CRC_WATER1	1216204625
#define CRC_WATER2	162975636
#define CRC_WATER3	-324810851
#define CRC_WATER4	-2026954058
#define CRC_WATER5	1536000314
#define CRC_WATER6	379630929
#define CRC_WATER7	1847065263
#define CRC_SKY1	-107209982 // Hisky
#define CRC_SKY2	-1973195832 // Hisky
#define CRC_SKY3	-643602558 // Hisky
#define CRC_MAPBACK	1524295306 // Map background texture (no postprocessing done for this)
#define CRC_NVG1	229882006	// Nvgoggles noise
#define CRC_NVG2	1921382912
#define CRC_NVG3	-1035560319
#define CRC_NVG4	259542144
#define CRC_NVG5	-1188577540
#define CRC_TRACER	-57859339
#define CRC_RAIN	1466354111

#define VSHADER_STATIC	0x2c4	// Pretransofmed & lighted vertex format, OFP uses this to render sky & UI overlays
#define VSHADER_WORLD	0x112	// "Normal" vertex format for everything else


LPDIRECT3D8 d3dh = NULL;		// D3D Handle
LPDIRECT3DDEVICE8 d3dd = NULL;	// D3D Device handle

LPDIRECT3DTEXTURE8      texWaterBump;
LPDIRECT3DTEXTURE8      texNoiseMap;
LPDIRECT3DVERTEXBUFFER8 ppQuad, ppQuadD4, ppQuadD8;
LPDIRECT3DVERTEXBUFFER8 ppBlurV, ppBlurH; 

struct PPVERTEX
{
    D3DXVECTOR4 p;
    DWORD       color;
    FLOAT       tu, tv;
};

struct PPBLURVERTEX
{
    D3DXVECTOR4 p;
    DWORD       color;
    FLOAT       tu0, tv0;
    FLOAT       tu1, tv1;
    FLOAT       tu2, tv2;
    FLOAT       tu3, tv3;
};

struct STREAMPRIMITIVE {	// Stores stream source & primitive info for later drawing
	LPDIRECT3DVERTEXBUFFER8 vBuffer;
	UINT stride;
	D3DVERTEXBUFFER_DESC desc;

	LPDIRECT3DINDEXBUFFER8 iBuffer;
	UINT vIndex;

	D3DPRIMITIVETYPE PrimitiveType;
	UINT minIndex;
	UINT NumVertices;
	UINT startIndex;
	UINT primCount;

	DWORD texID;
	DWORD vsID;

	D3DMATRIX matrixW, matrixV, matrixP;
	DWORD stateHandle;
	DWORD stateHandleP;
	DWORD stateHandleV;
};
DWORD groundStateBlock = -1;

struct {
	bool	usePostprocessing;
	bool	force32bitPP;
	bool	useSpecialNVGPP;
	bool	disablePPForMap;
	bool	disablePostprocessing;
	bool	useReflections;
	bool	reflectTerrain;
	bool	reflectObjects;
	bool	reflectUseExpFog;
	bool	reflectUseLighting;
	bool	showFPS;
	bool	showStat;
	bool	showUI;
	bool	showHelp;
	bool	useTrilinear;
	bool	sharpenUI;
	bool	forceNoNightShader;	// Remove "black & white night" pixelshader effect
	bool	debugOutput;
	bool	handlePrintScreen;
	bool	enhancedTracers;	// brigher and shorter tracers

	DWORD	filterMin;	// not used
	DWORD	filterMag;	// not used
	DWORD	filterMip;	// not used
	float	mipmapLODstage0;	// Mipmap LODbias for texture stage 0
	float	mipmapLODstage1;	// Mipmap LODbias for texture stage 1 (Multitexturing)
	float	mipmapLODrefl;		// Mipmap LODbias for water reflection

	// Postprocessing
	bool	ppHardLight;
	bool	ppDesaturate;
	bool	ppGlare;

	// Effect strengths
	float	ppHardLightSR, ppHardLightSG, ppHardLightSB;
	float	ppDesaturateSR, ppDesaturateSG, ppDesaturateSB;
	float	ppGlareStrength;
	DWORD	ppGlareSize;
	bool	ppGlareDarkenSky;
	bool	ppGlareForce32bpp;
	bool	ppGlareUseFade;

	int		ppResMultiplier;

	float	reflBumpStrength;
	float	reflTexOffsetX;		// Offset for reflection UV map
	float	reflTexOffsetY;
	float	reflClipTolerance;	// Clipping is adjusted this much from water height to avoid "see through" bumpmapping

	DWORD	reflTexSizeX;		// Reflection render-to-texture size
	DWORD	reflTexSizeY;	

	DWORD	reflObjMaxCount;
	DWORD	reflObjMaxDist;
	DWORD	reflObjMaxHeight;

} config;

int selectedEffect;

typedef struct {
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
} RGBACOLOR;

struct {
	DWORD trisFrame;
	DWORD trisSecond, trisSecondC;
	DWORD FPS, frames;
	DWORD numObjects;
	DWORD numReflObjects;

	int		sBase;
	int		tBase, ltBase;
	float	msBase;
} stat;

char		overlayText[512];	// String displayed on screen
int			overlayTime=-1;		
CD3DFont*	olFont;				// Overlay font

DWORD renderStates[0x171]; // Stores all states set with SetRenderState

STREAMPRIMITIVE *waterPrims;
int	numWaterPrims = 0;
STREAMPRIMITIVE *groundPrims;
int	numGroundPrims = 0;
STREAMPRIMITIVE *objectPrims;
int	numObjectPrims = 0;

// flag whether screenshot key has been pressed 
int screenShotPressed = 0;

extern "C" __declspec(dllexport) IDirect3D8 * __stdcall Direct3DCreate8(UINT SDKVersion);

DWORD orgShaders[16];				// Original shader ID's stored here
int	numOrgShaders=0;				// Number of shaders
DWORD currentShader=0;				// Currently active pixel shader
DWORD currentVShader=0;				// Currently active vertex shader
float waterHeight = -1.5f;				// Water height
float reflPos = 0.0f;
	
int texID[NUM_TEX];	// Holds textureID's for some textures
LPDIRECT3DTEXTURE8 texReplace[NUM_TEX]; // holds pointers to replacement textures
char texReplacePath[NUM_TEX][255]; // paths to replacement textures from config
int mappedTextures[8]; // Currently mapped textures for different texture stages
bool stateDrawWater = FALSE; // Set to TRUE when drawing water
bool frameWaterVisible = FALSE; // Set to TRUE when water visible on frame 

// OFP render flow seems to always go in this order, not reliable if only UI is visible (mission editor for example)
#define RENDER_SKY		0
#define RENDER_GROUND	1
#define RENDER_WATER	2
#define RENDER_HORIZON	3
#define RENDER_OBJECTS	4
#define RENDER_SHADOWS	5
#define RENDER_RAIN		6
#define RENDER_UI		7

bool isNight = FALSE; // Night?
bool mapVisible = FALSE;	// Set to true if rendered scene includes TEX_MAPBACK texture
bool nvgVisible = FALSE;	// Set to true if rendered scene includes TEX_NVGx texture
int	stateRender = 0;	// Current rendering state
int lastStateRender = 0; // Last rendering state

int resX = 1024;	// Overridden at device creation
int resY = 768;
D3DFORMAT DSFormat;
D3DFORMAT BBFormat;
D3DFORMAT PPFormat;	// Postprocessing buffer format
D3DFORMAT GlareFormat; // Glare buffer format

D3DMATRIX matWorldSaved;
D3DMATRIX matViewSaved;
D3DMATRIX matProjSaved;
D3DMATRIX matCamera;

D3DVIEWPORT8 oldViewport, reflViewport; // Current "real" viewport & reflection viewport
D3DXMATRIX matRev, matReflect;
D3DXPLANE clipPlane, reflPlane;

LPDIRECT3DTEXTURE8 pRenderTexture = NULL;	// Render-to-texture (Postprocessing)
LPDIRECT3DSURFACE8 pRenderSurface = NULL, pBackBuffer = NULL, pZBuffer = NULL; // // Render-to-texture surface & backbuffer
LPDIRECT3DSURFACE8 curBuffer = NULL; 

LPDIRECT3DTEXTURE8 pRenderTextureD4 = NULL;	// Render-to-texture (Postprocessing blur)
LPDIRECT3DSURFACE8 pRenderSurfaceD4 = NULL;   // Render-to-texture (Postprocessing blur) surface
LPDIRECT3DTEXTURE8 pRenderTextureD8 = NULL;	// Render-to-texture (Postprocessing blur)
LPDIRECT3DSURFACE8 pRenderSurfaceD8 = NULL;   // Render-to-texture (Postprocessing blur) surface

LPDIRECT3DTEXTURE8 pRenderReflTexture = NULL;	// Render-to-texture (Water reflection)
LPDIRECT3DSURFACE8 pRenderReflSurface = NULL;

LPDIRECT3DTEXTURE8 pRenderReflZBuffer = NULL;	// ZBuffer for water reflection
LPDIRECT3DSURFACE8 pRenderReflZBSurface = NULL;

D3DXMATRIX matRotationY, matTranslation, matProjection,matOldProjection;	// Stores projection matrices during render to texture

long crc32(long crc, char* buf, int len); // in crc.cpp

inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }  // Converts a FLOAT to a DWORD

void DrawWaterPrims(void);
void overwriteD3ddFunctions(void);

const char HelpText[] =
	"   App-I: Toggle reflections\n"
	"   App-O: Toggle object reflections\n"
	"   App-P: Toggle postprocessing\n"
	"   App-J: Toggle glare sky darkening\n"
	"   App-B: Toggle glare fade effect\n"
	"   App-N: Toggle night shader override\n"
	"   App-L: Toggle reflection lighting\n"
	"   App-K: Toggle reflection expfog\n"
	"   App-F: Toggle FPS\n"
	"   App-S: Toggle Stats & Settings\n"
	"   App-U: Toggle UI\n"
	"   App-H: Toggle Help\n"
	"  App-F8: Save screenshot\n"
	"  App-F9: Select Hard Light effect\n"
	" App-F10: Select Desaturation effect\n"
	" App-F11: Select Glare effect\n"
	" App-F12: Toggle selected effect on/off\n"
	" App-Ins: Increase selected effect red channel strength\n"
	"App-Home: Increase selected effect green channel strength\n"
	"App-Pgup: Increase selected effect blue channel strength\n"
	" App-Del: Decrease selected effect red channel strength\n"
	" App-End: Decrease selected effect green channel strength\n"
	"App-Pgdn: Decrease selected effect blue channel strength\n";

//
HRESULT WINAPI NewSetRenderState(LPDIRECT3DDEVICE8 fu, D3DRENDERSTATETYPE State, DWORD Value);

// Replaced functions
typedef HRESULT (WINAPI*TESTCOOPERATIVELEVELF)(LPDIRECT3DDEVICE8 fu);
TESTCOOPERATIVELEVELF OrgTestCooperativeLevel;

typedef HRESULT (WINAPI*RESETF)(LPDIRECT3DDEVICE8 fu, D3DPRESENT_PARAMETERS* pPresentationParameters);
RESETF OrgReset;

typedef HRESULT (WINAPI*CREATEVERTEXBUFFERF)(LPDIRECT3DDEVICE8 fu, UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer8** ppVertexBuffer);
CREATEVERTEXBUFFERF OrgCreateVertexBuffer;

typedef HRESULT (WINAPI*CREATEINDEXBUFFERF)(LPDIRECT3DDEVICE8 fu, UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer8** ppIndexBuffer);
CREATEINDEXBUFFERF OrgCreateIndexBuffer;

typedef HRESULT (WINAPI*SETRENDERSTATEF)(LPDIRECT3DDEVICE8 fu, D3DRENDERSTATETYPE State, DWORD Value);
SETRENDERSTATEF OrgSetRenderState;

typedef HRESULT (WINAPI*LIGHTENABLEF)(LPDIRECT3DDEVICE8 fu, DWORD Index, BOOL Enable);
LIGHTENABLEF OrgLightEnable;

typedef HRESULT (WINAPI*CREATEPIXELSHADERF)(LPDIRECT3DDEVICE8 fu, CONST DWORD* pFunction,DWORD* pHandle);
CREATEPIXELSHADERF OrgCreatePixelShader;

typedef HRESULT (WINAPI*SETPIXELSHADERF)(LPDIRECT3DDEVICE8 fu, DWORD Handle);
SETPIXELSHADERF OrgSetPixelShader;

typedef HRESULT (WINAPI*DRAWINDEXEDPRIMITIVEF)(LPDIRECT3DDEVICE8 fu, D3DPRIMITIVETYPE PrimitiveType,UINT minIndex,UINT NumVertices,UINT startIndex,UINT primCount);
DRAWINDEXEDPRIMITIVEF OrgDrawIndexedPrimitive;

typedef HRESULT (WINAPI*SETTRANSFORMF)(LPDIRECT3DDEVICE8 fu, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
SETTRANSFORMF OrgSetTransform;

typedef HRESULT (WINAPI*MULTIPLYTRANSFORMF)(LPDIRECT3DDEVICE8 fu, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix);
MULTIPLYTRANSFORMF OrgMultiplyTransform;

typedef HRESULT (WINAPI*UPDATETEXTUREF)(LPDIRECT3DDEVICE8 fu, IDirect3DBaseTexture8* pSourceTexture,IDirect3DBaseTexture8* pDestinationTexture);
UPDATETEXTUREF OrgUpdateTexture;

typedef HRESULT (WINAPI*BEGINSCENEF)(LPDIRECT3DDEVICE8 fu);
BEGINSCENEF OrgBeginScene;

typedef HRESULT (WINAPI*ENDSCENEF)(LPDIRECT3DDEVICE8 fu);
ENDSCENEF OrgEndScene;

typedef HRESULT (WINAPI*UNLOCKRECTF)(LPDIRECT3DTEXTURE8 fu, UINT Level);
UNLOCKRECTF OrgUnlockRect;

typedef HRESULT (WINAPI*RELEASETEXTUREF)(LPDIRECT3DTEXTURE8 fu);
RELEASETEXTUREF OrgReleaseTex;

typedef HRESULT (WINAPI*CREATETEXTUREF)(LPDIRECT3DDEVICE8 fu, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture8** ppTexture);
CREATETEXTUREF OrgCreateTexture;

typedef HRESULT (WINAPI*SETTEXTURESTAGEF)(LPDIRECT3DDEVICE8 fu, DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value);
SETTEXTURESTAGEF OrgSetTextureStageState;

typedef HRESULT (WINAPI*SETTEXTUREF)(LPDIRECT3DDEVICE8 fu, DWORD Stage,IDirect3DBaseTexture8* pTexture);
SETTEXTUREF OrgSetTexture;

typedef HRESULT (WINAPI*CREATEDEVICEF)(LPDIRECT3D8 fu, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice8** ppReturnedDeviceInterface);
CREATEDEVICEF OrgCreateDevice;
#endif