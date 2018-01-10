/* Copyright (C) 2004-2018 DXDLL authors, see included COPYING file */

#include "proxydll.h"
#include <windows.h>
#include "d3d8.h"
#include <stdio.h>
#include <tchar.h>
#include <time.h>

typedef IDirect3D8 * (WINAPI*D3DCREATETYPE)(UINT);
D3DCREATETYPE ODirect3DCreate8 = NULL;

HINSTANCE hLib = NULL;

// Show message box
void ShowMessage (char *first, ...)
{
    va_list     argptr;
    char        Message[512];

    va_start (argptr,first);
    vsprintf (Message,first,argptr);
    va_end   (argptr);

    MessageBox(NULL, Message,"ProxyDLL",MB_OK);    
}

// Print message to screen
void PrintMessage (char *first, ...)
{
    va_list     argptr;
    char        Message[512];

    va_start (argptr,first);
    vsprintf (Message,first,argptr);
    va_end   (argptr);

	strcpy(overlayText, Message);
	overlayTime = stat.tBase+2000;
}

// Output debug string
void DebugMessage (char *first, ...)
{
	if(config.debugOutput) {
		va_list     argptr;
		char        Message[512];

		va_start (argptr,first);
		vsprintf (Message,first,argptr);
		va_end   (argptr);

		OutputDebugString(Message);   
	}
}

// Output matrix to debug
void DebugPrintMatrix(D3DMATRIX mat)
{
	DebugMessage("Matrix: %i:", mat);

	DebugMessage("        %.4f, %.4f, %.4f, %.4f", mat._11, mat._12, mat._13, mat._14);
	DebugMessage("        %.4f, %.4f, %.4f, %.4f", mat._21, mat._22, mat._23, mat._24);
	DebugMessage("        %.4f, %.4f, %.4f, %.4f", mat._31, mat._32, mat._33, mat._34);
	DebugMessage("        %.4f, %.4f, %.4f, %.4f", mat._41, mat._42, mat._43, mat._44);
}

int readstr(FILE *f,char *string) // Reads one line from file
{
    char *status;

	do { 
        status = fgets(string, 255, f);
        if(!status)                     // EOF?
            return FALSE;
  
	} while ((string[0] == ';') || (string[0] == '\n'));

	return TRUE;
}

bool loadConfig() {
	FILE *filu;
	char section[255];
	char command[255];
	char value[255];
    char data[255];

	for (int i = 0; i < NUM_TEX; i++) texReplacePath[i][0] = '\0';

	filu = fopen("dxdll/config.cfg", "r");
	if (!filu)
		return false;

	while(readstr(filu,data)) {						// Read one line per time until EOF
		if(sscanf(data, "[%s]", &section) == 0) {
			// Remove '=' character
			char *c = (char*) memchr(data, '=', strlen(data));
			if(c != NULL)
				data[c-data] = 0x20;

	   		sscanf(data, "%s %s", &command, &value);

			if(!lstrcmpi("general", section)) { // General section
				if(!lstrcmpi("showFPS",command))			config.showFPS = atoi(value) != 0;
				if(!lstrcmpi("showStat",command))			config.showStat = atoi(value) != 0;
				if(!lstrcmpi("useTrilinear",command))		config.useTrilinear = atoi(value) != 0;
				if(!lstrcmpi("sharpenUI",command))			config.sharpenUI = atoi(value) != 0;
				if(!lstrcmpi("forceNoNightShader",command))	config.forceNoNightShader = atoi(value) != 0;
				if(!lstrcmpi("EnhancedTracers",command))	config.enhancedTracers = atoi(value) != 0;

				if(!lstrcmpi("debugOutput",command))		config.debugOutput = atoi(value) != 0;
				if(!lstrcmpi("handlePrintScreen",command))	config.handlePrintScreen = atoi(value) != 0;				
				if(!lstrcmpi("LODbias0",command))			config.mipmapLODstage0 = (float) atof(value);
				if(!lstrcmpi("LODbias1",command))			config.mipmapLODstage1 = (float) atof(value);
			}

			if(!lstrcmpi("postprocessing", section)) { // Postprocessing section
				if(!lstrcmpi("UsePostProcessing",command))	config.usePostprocessing = atoi(value) != 0;
				if(!lstrcmpi("Force32bitPostProcessing",command))	config.force32bitPP = atoi(value) != 0;				
				if(!lstrcmpi("DisablePPForMap",command))	config.disablePPForMap = atoi(value) != 0;
				if(!lstrcmpi("UseSpecialNVGPP",command))	config.useSpecialNVGPP = atoi(value) != 0;
				

				if(!lstrcmpi("HardLight",command))	config.ppHardLight = atoi(value) != 0;
				if(!lstrcmpi("Desaturate",command))	config.ppDesaturate = atoi(value) != 0;
				if(!lstrcmpi("Glare",command))		config.ppGlare = atoi(value) != 0;

				if(!lstrcmpi("HardLightSR",command))		config.ppHardLightSR = (float) atof(value);
				if(!lstrcmpi("HardLightSG",command))		config.ppHardLightSG = (float) atof(value);
				if(!lstrcmpi("HardLightSB",command))		config.ppHardLightSB = (float) atof(value);

				if(!lstrcmpi("DesaturateSR",command))		config.ppDesaturateSR = (float) atof(value);
				if(!lstrcmpi("DesaturateSG",command))		config.ppDesaturateSG = (float) atof(value);
				if(!lstrcmpi("DesaturateSB",command))		config.ppDesaturateSB = (float) atof(value);

				if(!lstrcmpi("GlareStrength",command))		config.ppGlareStrength = (float) atof(value);
				if(!lstrcmpi("GlareSize",command))			config.ppGlareSize = atoi(value);
				if(!lstrcmpi("GlareDarkenSky",command))		config.ppGlareDarkenSky = atoi(value) != 0;
				if(!lstrcmpi("GlareUseFade",command))		config.ppGlareUseFade = atoi(value) != 0;				
				if(!lstrcmpi("GlareForce32bitBuffer",command))		config.ppGlareForce32bpp = atoi(value) != 0;
			}

			if(!lstrcmpi("reflections", section)) { // Reflections section
				if(!lstrcmpi("useReflections",command))	config.useReflections = atoi(value) != 0;
				if(!lstrcmpi("reflectTerrain",command))	config.reflectTerrain = atoi(value) != 0;
				if(!lstrcmpi("reflectObjects",command))	config.reflectObjects = atoi(value) != 0;
				if(!lstrcmpi("useExpFog",command))		config.reflectUseExpFog = atoi(value) != 0;
				if(!lstrcmpi("useLighting",command))	config.reflectUseLighting = atoi(value) != 0;

				if(!lstrcmpi("sizeX",command))			config.reflTexSizeX = atoi(value);
				if(!lstrcmpi("sizeY",command))			config.reflTexSizeY = atoi(value);
				if(!lstrcmpi("maxCount",command))		config.reflObjMaxCount = atoi(value);
				if(!lstrcmpi("maxDistance",command))	config.reflObjMaxDist = atoi(value);
				if(!lstrcmpi("maxHeight",command))		config.reflObjMaxHeight = atoi(value);

				if(!lstrcmpi("bumpStrength",command))		config.reflBumpStrength = (float) atof(value)/100;
				if(!lstrcmpi("LODbias",command))			config.mipmapLODrefl = (float) atof(value);
				if(!lstrcmpi("clipTolerance",command))		config.reflClipTolerance = (float) atof(value);
				if(!lstrcmpi("texOffsetX",command))			config.reflTexOffsetX = (float) atof(value);
				if(!lstrcmpi("texOffsetY",command))			config.reflTexOffsetY = (float) atof(value);
			}
			if(!lstrcmpi("Textures", section)) { // textures section
				if(!lstrcmpi("TEX_DET_GROUND",command)) 
					strncpy(texReplacePath[TEX_DET_GROUND], value, 255);
				if(!lstrcmpi("TEX_MAPBACK",command))
					strncpy(texReplacePath[TEX_MAPBACK], value, 255);
				if(!lstrcmpi("TEX_WATERU1",command))
					strncpy(texReplacePath[TEX_WATER1], value, 255);
				if(!lstrcmpi("TEX_WATERU2",command))
					strncpy(texReplacePath[TEX_WATER2], value, 255);
				if(!lstrcmpi("TEX_WATERU3",command))
					strncpy(texReplacePath[TEX_WATER3], value, 255);
				if(!lstrcmpi("TEX_WATERU4",command))
					strncpy(texReplacePath[TEX_WATER4], value, 255);
				if(!lstrcmpi("TEX_WATERU5",command))
					strncpy(texReplacePath[TEX_WATER5], value, 255);
				if(!lstrcmpi("TEX_WATERU6",command))
					strncpy(texReplacePath[TEX_WATER6], value, 255);
				if(!lstrcmpi("TEX_WATERU7",command))
					strncpy(texReplacePath[TEX_WATER7], value, 255);
			}

			//DebugMessage("(%s) %s = %f", section, command, value);
		} else {
			char *c = (char*) memchr(section, ']', strlen(section));
			if(c != NULL)
				section[c-section] = 0x0;
		}
	}

    fclose(filu);
	return true;
}

// Receives keyboard events
LRESULT __declspec(dllexport)__stdcall  CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	bool extended = ((HIWORD (lParam) & KF_EXTENDED) != 0);
	bool state = ((HIWORD (lParam) & KF_UP) != 0);
	int key = wParam;

	if(extended && key == VK_APPLICATION)
		keyModPressed = !state;

	// Print Screen handler
	if(key == VK_SNAPSHOT && config.handlePrintScreen)
		if(keyModPressed && config.showUI) {
			// Remove UI first
			screenShotPressed = 3;
		} else {
			// Normal shot
			screenShotPressed = 1;
			PrintMessage("Saving screenshot");
		}

	if(keyModPressed) {	// Application key is down
		//DebugMessage("KBproc(%i) %i, %i %i", nCode, wParam, extended, state);

		if(!state) // Key was pressed down
			switch(key) {
				case 'P':
					config.usePostprocessing = !config.usePostprocessing;
					PrintMessage("Postprocessing: %i", config.usePostprocessing);
					break;
				case 'O':
					config.reflectObjects = !config.reflectObjects;
					PrintMessage("Object reflections: %i", config.reflectObjects);
					break;
				case 'I':
					config.useReflections = !config.useReflections;
					PrintMessage("Reflections: %i", config.useReflections);
					break;
				case 'N':
					config.forceNoNightShader = !config.forceNoNightShader;
					PrintMessage("Night shader override: %i", config.forceNoNightShader);
					break;
				case 'L':
					config.reflectUseLighting = !config.reflectUseLighting;
					PrintMessage("Reflection lighting: %i", config.reflectUseLighting);
					break;
				case 'K':
					config.reflectUseExpFog = !config.reflectUseExpFog;
					PrintMessage("Reflection expfog: %i", config.reflectUseExpFog);
					break;

				case 'F':
					config.showFPS = !config.showFPS;
					PrintMessage("Show FPS: %i", config.showFPS);
					break;
				case 'S':
					config.showStat = !config.showStat;
					PrintMessage("Show Stat: %i", config.showStat);
					break;

				case 'U':
					config.showUI = !config.showUI;
					PrintMessage("Draw UI: %i", config.showUI);
					break;

				case 'J':
					config.ppGlareDarkenSky = !config.ppGlareDarkenSky;
					NewSetRenderState(d3dd, D3DRS_FOGCOLOR, renderStates[D3DRS_FOGCOLOR]);					
					PrintMessage("Glare darken sky: %i", config.ppGlareDarkenSky);
					break;

				case 'B':
					config.ppGlareUseFade = !config.ppGlareUseFade;				
					PrintMessage("Glare fade: %i", config.ppGlareUseFade);
					break;

				case 'H':
					config.showHelp = !config.showHelp;
					break;
				case 'T':
					config.enhancedTracers = !config.enhancedTracers;
					PrintMessage("Enhanced tracers: %i", config.enhancedTracers);
					break;

				case VK_F8:
					screenShotPressed = 1;
					PrintMessage("Saving screenshot");
					break;

				case VK_F9:
					selectedEffect = 0;
					PrintMessage("Selected effect: Hard Light");
					break;

				case VK_F10:
					selectedEffect = 1;
					PrintMessage("Selected effect: Desaturate");
					break;

				case VK_F11:
					selectedEffect = 2;
					PrintMessage("Selected effect: Glare");
					break;

				case VK_F12:
					if(selectedEffect == 0) {
						config.ppHardLight = !config.ppHardLight;
						PrintMessage("Hard Light effect: %i", config.ppHardLight);
					} else if(selectedEffect == 1) {
						config.ppDesaturate = !config.ppDesaturate;
						PrintMessage("Desaturate effect: %i", config.ppDesaturate);
					} else if(selectedEffect == 2) {
						config.ppGlare = !config.ppGlare;
						PrintMessage("Glare effect: %i", config.ppGlare);
					}

					break;

				// Red strength
				case VK_DELETE:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSR -= 0.1f;
							if(config.ppHardLightSR < 0.0f)
								config.ppHardLightSR = 0.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSR -= 0.1f;
							if(config.ppDesaturateSR < 0.0f)
								config.ppDesaturateSR = 0.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength -= 0.05f;
							if(config.ppGlareStrength < 0.0f)
								config.ppGlareStrength = 0.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;
					
				case VK_INSERT:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSR += 0.1f;
							if(config.ppHardLightSR > 1.0f)
								config.ppHardLightSR = 1.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSR += 0.1f;
							if(config.ppDesaturateSR > 1.0f)
								config.ppDesaturateSR = 1.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength += 0.05f;
							if(config.ppGlareStrength > 1.0f)
								config.ppGlareStrength = 1.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;

				// Green strength
				case VK_END:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSG -= 0.1f;
							if(config.ppHardLightSG < 0.0f)
								config.ppHardLightSG = 0.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSG -= 0.1f;
							if(config.ppDesaturateSG < 0.0f)
								config.ppDesaturateSG = 0.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength -= 0.05f;
							if(config.ppGlareStrength < 0.0f)
								config.ppGlareStrength = 0.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;
					
				case VK_HOME:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSG += 0.1f;
							if(config.ppHardLightSG > 1.0f)
								config.ppHardLightSG = 1.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSG += 0.1f;
							if(config.ppDesaturateSG > 1.0f)
								config.ppDesaturateSG = 1.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength += 0.05f;
							if(config.ppGlareStrength > 1.0f)
								config.ppGlareStrength = 1.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;

				// Blue strength
				case VK_NEXT:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSB -= 0.1f;
							if(config.ppHardLightSB < 0.0f)
								config.ppHardLightSB = 0.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSB -= 0.1f;
							if(config.ppDesaturateSB < 0.0f)
								config.ppDesaturateSB = 0.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength -= 0.05f;
							if(config.ppGlareStrength < 0.0f)
								config.ppGlareStrength = 0.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;
					
				case VK_PRIOR:
					switch(selectedEffect) {
						case 0: // Hard light
							config.ppHardLightSB += 0.1f;
							if(config.ppHardLightSB > 1.0f)
								config.ppHardLightSB = 1.0f;
							PrintMessage("Hard Light strength: %.1f %.1f %.1f", config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB);
							break;
						case 1: // Desaturate
							config.ppDesaturateSB += 0.1f;
							if(config.ppDesaturateSB > 1.0f)
								config.ppDesaturateSB = 1.0f;
							PrintMessage("Desaturate strength: %.1f %.1f %.1f", config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB);
							break;
						case 2: // Glare
							config.ppGlareStrength += 0.05f;
							if(config.ppGlareStrength > 1.0f)
								config.ppGlareStrength = 1.0f;
							PrintMessage("Glare strength: %.2f", config.ppGlareStrength);
							break;
					}
					break;
					
			}

		//return true;
		return CallNextHookEx( hkb, nCode, wParam, lParam );
	} else {
		return CallNextHookEx( hkb, nCode, wParam, lParam );
	}
}

BOOL APIENTRY DllMain( HINSTANCE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                         )
{
  switch (ul_reason_for_call)
    {
         case DLL_PROCESS_ATTACH:
         if (hLib == NULL)
         {
			// Find & load real d3d8.dll
			FILE *filu;
			char path[256];
			char path32[256];
			char dllpath[256];
			char *syspath = getenv("SystemRoot");
			if(!syspath) {
				syspath = getenv("WinDir");
				if(!syspath) {
					ShowMessage("'SystemRoot' or 'WinDir' environment variable not set!");
					exit(0);
				}
			}

			strcpy(path, syspath);
			strcpy(path32, syspath);
			strcat(path, "\\system\\d3d8.dll");
			strcat(path32, "\\system32\\d3d8.dll");

			filu = fopen(path32, "r");
			if(!filu) {
				filu = fopen(path, "r");
				strcpy(dllpath, path);
			} else
				strcpy(dllpath, path32);

			if(!filu) {
				ShowMessage("d3d8.dll not found!\n\n%s\n%s", path, path32);
				exit(0);
			} else {
				DebugMessage("Load DLL %s", dllpath);
				fclose(filu);
				hLib=LoadLibrary(dllpath);
				ODirect3DCreate8 = (D3DCREATETYPE) GetProcAddress((HMODULE)hLib, "Direct3DCreate8");
			}

			// Set keyboard hook
			hkb = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, NULL, GetCurrentThreadId());
			
         }
         case DLL_THREAD_ATTACH:
         case DLL_THREAD_DETACH:
              break;
         case DLL_PROCESS_DETACH:
				DebugMessage("ProxyDLL Exit");

				// Release keyboard hook
				UnhookWindowsHookEx(hkb);

				// TODO: propably should free shaders, buffers, etc. here
              break;
   }
   return TRUE;
}

/*
// Draws water primitives
void DrawGroundPrims(void) {
	if(numGroundPrims==0)
		return;

	for(int x =0;x < numGroundPrims;x++) {
		HRESULT ret = (*OrgDrawIndexedPrimitive)(d3dd, groundPrims[x].PrimitiveType, groundPrims[x].minIndex, groundPrims[x].NumVertices, groundPrims[x].startIndex, groundPrims[x].primCount);
	}
}*/

void drawObjectPrims(void) {
	DWORD stateBlock;
	d3dd->CreateStateBlock(D3DSBT_ALL, &stateBlock);
	//d3dd->SetVertexShader( 0x112 );	// TODO: Convert this to proper format

	(*OrgSetRenderState)(d3dd, D3DRS_ZFUNC, 0x4);
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHAREF, 0);
	(*OrgSetRenderState)(d3dd, D3DRS_ZWRITEENABLE, TRUE);
	(*OrgSetRenderState)(d3dd, D3DRS_FOGENABLE, TRUE);
	(*OrgSetRenderState)(d3dd, D3DRS_CLIPPING, TRUE);
	
	/*(*OrgSetRenderState)(d3dd, D3DRS_ALPHAREF, );
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHATESTENABLE, );
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHABLENDENABLE, );*/
	(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

//IDirect3DDevice8_SetTextureStageState
	//DebugMessage("Drawing %i primitives", numObjectPrims);
	for(int x =0;x < numObjectPrims;x++) {
		/*d3dd->SetVertexShader( objectPrims[x].vsID );	// TODO: Convert this to proper format
		d3dd->SetTexture(0, (LPDIRECT3DTEXTURE8) objectPrims[x].texID);

		d3dd->SetIndices(objectPrims[x].iBuffer, objectPrims[x].vIndex);
		d3dd->SetStreamSource(0, objectPrims[x].vBuffer, objectPrims[x].stride);

		d3dd->SetTransform(D3DTS_VIEW, &objectPrims[x].matrixV);
		d3dd->SetTransform(D3DTS_WORLD, &objectPrims[x].matrixW);
		d3dd->SetTransform(D3DTS_PROJECTION, &objectPrims[x].matrixP);*/

		d3dd->ApplyStateBlock(objectPrims[x].stateHandle);
		d3dd->DeleteStateBlock(objectPrims[x].stateHandle);
		HRESULT ret = (*OrgDrawIndexedPrimitive)(d3dd, objectPrims[x].PrimitiveType, objectPrims[x].minIndex, objectPrims[x].NumVertices, objectPrims[x].startIndex, objectPrims[x].primCount);
		/*objectPrims[x].vBuffer->Release();
		objectPrims[x].iBuffer->Release();*/
	}
	d3dd->ApplyStateBlock(stateBlock);
	d3dd->DeleteStateBlock(stateBlock);
}

void drawWaterPrims(void) {
	DWORD stateBlock;
	d3dd->CreateStateBlock(D3DSBT_ALL, &stateBlock);
	//d3dd->SetVertexShader( 0x112 );	// TODO: Convert this to proper format

/*	(*OrgSetRenderState)(d3dd, D3DRS_ZFUNC, 0x4);
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHAREF, 0);
	(*OrgSetRenderState)(d3dd, D3DRS_ZWRITEENABLE, TRUE);
	(*OrgSetRenderState)(d3dd, D3DRS_FOGENABLE, TRUE);
	(*OrgSetRenderState)(d3dd, D3DRS_CLIPPING, TRUE);*/
	
	/*(*OrgSetRenderState)(d3dd, D3DRS_ALPHAREF, );
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHATESTENABLE, );
	(*OrgSetRenderState)(d3dd, D3DRS_ALPHABLENDENABLE, );*/
	(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);


	DebugMessage("Drawing %i primitives", numWaterPrims);
	for(int x =0;x < numWaterPrims;x++) {
		/*d3dd->SetVertexShader( objectPrims[x].vsID );	// TODO: Convert this to proper format
		d3dd->SetTexture(0, (LPDIRECT3DTEXTURE8) objectPrims[x].texID);

		d3dd->SetIndices(objectPrims[x].iBuffer, objectPrims[x].vIndex);
		d3dd->SetStreamSource(0, objectPrims[x].vBuffer, objectPrims[x].stride);

		d3dd->SetTransform(D3DTS_VIEW, &objectPrims[x].matrixV);
		d3dd->SetTransform(D3DTS_WORLD, &objectPrims[x].matrixW);
		d3dd->SetTransform(D3DTS_PROJECTION, &objectPrims[x].matrixP);*/

		d3dd->ApplyStateBlock(waterPrims[x].stateHandle);
		d3dd->DeleteStateBlock(waterPrims[x].stateHandle);
		HRESULT ret = (*OrgDrawIndexedPrimitive)(d3dd, waterPrims[x].PrimitiveType, waterPrims[x].minIndex, waterPrims[x].NumVertices, waterPrims[x].startIndex, waterPrims[x].primCount);
		/*objectPrims[x].vBuffer->Release();
		objectPrims[x].iBuffer->Release();*/
	}
	d3dd->ApplyStateBlock(stateBlock);
	d3dd->DeleteStateBlock(stateBlock);
}

void drawGroundPrims(void) {
/*WORD stateBlock;
	d3dd->CreateStateBlock(D3DSBT_ALL, &stateBlock);*/

	DebugMessage("Drawing %i primitives", numGroundPrims);
	for(int x =0;x < numGroundPrims;x++) {
		//d3dd->ApplyStateBlock(groundPrims[x].stateHandle);

		/*d3dd->ApplyStateBlock(groundPrims[x].stateHandleP);
		d3dd->ApplyStateBlock(groundPrims[x].stateHandleV);*/
		d3dd->SetStreamSource(0, groundPrims[x].vBuffer, groundPrims[x].stride);
		d3dd->SetIndices(groundPrims[x].iBuffer, groundPrims[x].vIndex);
		d3dd->SetTransform(D3DTS_WORLD, &groundPrims[x].matrixW);
		/*d3dd->SetTransform(D3DTS_VIEW, &groundPrims[x].matrixV);
		d3dd->SetTransform(D3DTS_PROJECTION, &groundPrims[x].matrixP);*/

		HRESULT ret = (*OrgDrawIndexedPrimitive)(d3dd, groundPrims[x].PrimitiveType, groundPrims[x].minIndex, groundPrims[x].NumVertices, groundPrims[x].startIndex, groundPrims[x].primCount);
		/*d3dd->DeleteStateBlock(groundPrims[x].stateHandleP);
		d3dd->DeleteStateBlock(groundPrims[x].stateHandleV);*/

		//d3dd->DeleteStateBlock(groundPrims[x].stateHandle);
	}
	/*d3dd->ApplyStateBlock(stateBlock);
	d3dd->DeleteStateBlock(stateBlock);*/
}

// Compiles a pixel shader
DWORD compileShader(const char *source) {
	DWORD shader;
	D3DXAssembleShader( source, strlen(source)-1, 0, NULL, &shaderCode[numShaders], NULL );    // assemble shader code
	(*OrgCreatePixelShader)(d3dd, (DWORD*)shaderCode[numShaders]->GetBufferPointer(), &shader );
	numShaders++;
	return shader;
}

void createQuad(LPDIRECT3DVERTEXBUFFER8 *quad, int x, int y) {
	PPVERTEX* vBackground;

    // Create a quad vertexbuffer
    if(D3D_OK != d3dd->CreateVertexBuffer(4*sizeof(PPVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1, D3DPOOL_MANAGED, quad)) {
		ShowMessage("CreateVertexBuffer failed!");
	}

    (*quad)->Lock(0, 0, (BYTE**)&vBackground, 0);
    for(int i=0; i<4; i++)
    {
        vBackground[i].p = D3DXVECTOR4(0.0f, 0.0f, 0.9f, 1.0f);
        vBackground[i].color = 0xffffffff;
    }
    vBackground[0].p.y = (FLOAT) y;
    vBackground[2].p.y = (FLOAT) y;
    vBackground[2].p.x = (FLOAT) x;
    vBackground[3].p.x = (FLOAT) x;

    vBackground[0].tu = (float) 0.5/x; vBackground[0].tv = (float) (y + 0.5)/y;
    vBackground[1].tu = (float) 0.5/x; vBackground[1].tv = (float) 0.5/y;
    vBackground[2].tu = (float) (x + 0.5)/x; vBackground[2].tv = (float) (y + 0.5)/y;
    vBackground[3].tu = (float) (x + 0.5)/x; vBackground[3].tv = (float) 0.5/y;
    (*quad)->Unlock();
}

// Creates RTT surfaces, reflection planes, etc.
void initStuff(bool reset) {

	overwriteD3ddFunctions();

	// Set texID's to -1
	if(reset) {
		for(int x=0;x < NUM_TEX;x++)
			texID[x] = -1;
		// Compile pixel shaders
		DebugMessage("Compiling hardlight shader...");
		hardLightShader = compileShader(hardLightSRC);

		DebugMessage("Compiling desaturate shader...");
		desaturateShader = compileShader(desaturateSRC);

		DebugMessage("Compiling blur shaders...");
		blurPass1Shader = compileShader(blurPass1SRC);
		blurCombineShader = compileShader(blurCombineSRC);
		blurPass1fadeShader = compileShader(blurPass1fadeSRC);

		DebugMessage("Compiling sky shader...");
		skyDarkenShader = compileShader(skyDarkenSRC);
		
	}

	//BBFormat = D3DFMT_R5G6B5;

	// Create render to texture target texture (Water reflection)
	if(D3D_OK != d3dd->CreateTexture(config.reflTexSizeX, config.reflTexSizeY, 1, D3DUSAGE_RENDERTARGET, BBFormat, D3DPOOL_DEFAULT, &pRenderReflTexture))
		ShowMessage("Reflection CreateTexture failed!");

	pRenderReflTexture->GetSurfaceLevel(0,&pRenderReflSurface);

	// Create render to texture Z-Buffer (Water reflection)
	d3dd->CreateDepthStencilSurface(config.reflTexSizeX, config.reflTexSizeY, D3DFMT_D16, D3DMULTISAMPLE_NONE, &pRenderReflZBSurface);

	// Create render to texture target texture (Postprocessing)
	if(D3D_OK != d3dd->CreateTexture(resX, resY, 1, D3DUSAGE_RENDERTARGET, PPFormat, D3DPOOL_DEFAULT, &pRenderTexture))
		ShowMessage("Postprocessing CreateTexture failed!");
	pRenderTexture->GetSurfaceLevel(0,&pRenderSurface);
	
	// For blur effect
	if(D3D_OK != d3dd->CreateTexture(resX/config.ppGlareSize, resY/config.ppGlareSize, 1, D3DUSAGE_RENDERTARGET, GlareFormat, D3DPOOL_DEFAULT, &pRenderTextureD4))
		ShowMessage("Postprocessing CreateTexture failed!");	
	pRenderTextureD4->GetSurfaceLevel(0,&pRenderSurfaceD4);
	if(D3D_OK != d3dd->CreateTexture(resX/config.ppGlareSize, resY/config.ppGlareSize, 1, D3DUSAGE_RENDERTARGET, GlareFormat, D3DPOOL_DEFAULT, &pRenderTextureD8))
		ShowMessage("Postprocessing CreateTexture failed!");	
	pRenderTextureD8->GetSurfaceLevel(0,&pRenderSurfaceD8);

	// Create a quads for postprocessing renderer
	createQuad(&ppQuad, resX, resY);

	// Create a quad vertexbuffer, for vertical blur
	if(D3D_OK != d3dd->CreateVertexBuffer(4*sizeof(PPBLURVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX4, D3DPOOL_MANAGED, &ppBlurV)) {
		ShowMessage("CreateVertexBuffer failed!");
	}

	PPBLURVERTEX* vBackground;
	int xW = resX/config.ppGlareSize;
	int yW = resY/config.ppGlareSize;

	ppBlurV->Lock(0, 0, (BYTE**)&vBackground, 0);
	for(int i=0; i<4; i++)
	{
		vBackground[i].p = D3DXVECTOR4(0.0f, 0.0f, 0.9f, 1.0f);
		vBackground[i].color = 0xffffffff;
	}
	vBackground[0].p.y = (FLOAT) yW;
	vBackground[2].p.y = (FLOAT) yW;
	vBackground[2].p.x = (FLOAT) xW;
	vBackground[3].p.x = (FLOAT) xW;

	vBackground[0].tu0 = (float) -1.0/xW; vBackground[0].tv0 = (float) (yW + 0.5)/yW;
	vBackground[1].tu0 = (float) -1.0/xW; vBackground[1].tv0 = (float) 0.5/yW;
	vBackground[2].tu0 = (float) (xW + -1.0)/xW; vBackground[2].tv0 = (float) (yW + 0.5)/yW;
	vBackground[3].tu0 = (float) (xW + -1.0)/xW; vBackground[3].tv0 = (float) 0.5/yW;

	vBackground[0].tu1 = (float) 0.0/xW; vBackground[0].tv1 = (float) (yW + 0.5)/yW;
	vBackground[1].tu1 = (float) 0.0/xW; vBackground[1].tv1 = (float) 0.5/yW;
	vBackground[2].tu1 = (float) (xW + 0.0)/xW; vBackground[2].tv1 = (float) (yW + 0.5)/yW;
	vBackground[3].tu1 = (float) (xW + 0.0)/xW; vBackground[3].tv1 = (float) 0.5/yW;

	vBackground[0].tu2 = (float) 1.0/xW; vBackground[0].tv2 = (float) (yW + 0.5)/yW;
	vBackground[1].tu2 = (float) 1.0/xW; vBackground[1].tv2 = (float) 0.5/yW;
	vBackground[2].tu2 = (float) (xW + 1.0)/xW; vBackground[2].tv2 = (float) (yW + 0.5)/yW;
	vBackground[3].tu2 = (float) (xW + 1.0)/xW; vBackground[3].tv2 = (float) 0.5/yW;

	vBackground[0].tu3 = (float) 2.0/xW; vBackground[0].tv3 = (float) (yW + 0.5)/yW;
	vBackground[1].tu3 = (float) 2.0/xW; vBackground[1].tv3 = (float) 0.5/yW;
	vBackground[2].tu3 = (float) (xW + 2.0)/xW; vBackground[2].tv3 = (float) (yW + 0.5)/yW;
	vBackground[3].tu3 = (float) (xW + 2.0)/xW; vBackground[3].tv3 = (float) 0.5/yW;

	ppBlurV->Unlock();


	// Create a quad vertexbuffer, for horizontal blur
	if(D3D_OK != d3dd->CreateVertexBuffer(4*sizeof(PPBLURVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX4, D3DPOOL_MANAGED, &ppBlurH)) {
		ShowMessage("CreateVertexBuffer failed!");
	}

	ppBlurH->Lock(0, 0, (BYTE**)&vBackground, 0);
	for(i=0; i<4; i++)
	{
		vBackground[i].p = D3DXVECTOR4(0.0f, 0.0f, 0.9f, 1.0f);
		vBackground[i].color = 0xffffffff;
	}
	vBackground[0].p.y = (FLOAT) yW;
	vBackground[2].p.y = (FLOAT) yW;
	vBackground[2].p.x = (FLOAT) xW;
	vBackground[3].p.x = (FLOAT) xW;

	vBackground[0].tu0 = (float) 0.5/xW; vBackground[0].tv0 = (float) (yW + -1.0)/yW;
	vBackground[1].tu0 = (float) 0.5/xW; vBackground[1].tv0 = (float) -1.0/yW;
	vBackground[2].tu0 = (float) (xW + 0.5)/xW; vBackground[2].tv0 = (float) (yW + -1.0)/yW;
	vBackground[3].tu0 = (float) (xW + 0.5)/xW; vBackground[3].tv0 = (float) -1.0/yW;

	vBackground[0].tu1 = (float) 0.5/xW; vBackground[0].tv1 = (float) (yW + 0.0)/yW;
	vBackground[1].tu1 = (float) 0.5/xW; vBackground[1].tv1 = (float) 0.0/yW;
	vBackground[2].tu1 = (float) (xW + 0.5)/xW; vBackground[2].tv1 = (float) (yW + 0.0)/yW;
	vBackground[3].tu1 = (float) (xW + 0.5)/xW; vBackground[3].tv1 = (float) 0.0/yW;

	vBackground[0].tu2 = (float) 0.5/xW; vBackground[0].tv2 = (float) (yW + 1.0)/yW;
	vBackground[1].tu2 = (float) 0.5/xW; vBackground[1].tv2 = (float) 1.0/yW;
	vBackground[2].tu2 = (float) (xW + 0.5)/xW; vBackground[2].tv2 = (float) (yW + 1.0)/yW;
	vBackground[3].tu2 = (float) (xW + 0.5)/xW; vBackground[3].tv2 = (float) 1.0/yW;

	vBackground[0].tu3 = (float) 0.5/xW; vBackground[0].tv3 = (float) (yW + 2.0)/yW;
	vBackground[1].tu3 = (float) 0.5/xW; vBackground[1].tv3 = (float) 2.0/yW;
	vBackground[2].tu3 = (float) (xW + 0.5)/xW; vBackground[2].tv3 = (float) (yW + 2.0)/yW;
	vBackground[3].tu3 = (float) (xW + 0.5)/xW; vBackground[3].tv3 = (float) 2.0/yW;
	ppBlurH->Unlock();


	// Load water bumpmap texture	
	DebugMessage("Loading water bumpmap");
	if(D3DXCreateTextureFromFileA(d3dd,"dxdll/textures/waterbump.png",&texWaterBump) != D3D_OK)
		DebugMessage("Failed!");

	// load replacement textures
	for (int i = 0; i < NUM_TEX; i++) {
		if (strlen(texReplacePath[i]) > 0) {
			DebugMessage("Found replacement for %d: %s", i, texReplacePath[i]);
			if (D3DXCreateTextureFromFileA(d3dd, texReplacePath[i], &texReplace[i]) != D3D_OK)
				DebugMessage("Failed!");
		} else texReplace[i] = NULL;
	}

	// Setup viewport for reflection
	reflViewport.X = 0;
	reflViewport.Y = 0;
	reflViewport.Width = config.reflTexSizeX;
	reflViewport.Height = config.reflTexSizeY;
	reflViewport.MinZ = 0.0;
	reflViewport.MaxZ = 1.0;

	// Initialize reflection & clip planes
	D3DXPlaneFromPointNormal(&reflPlane, &D3DXVECTOR3(0,waterHeight,0), &D3DXVECTOR3(0,10,0));
	D3DXPlaneFromPointNormal(&clipPlane, &D3DXVECTOR3(0,waterHeight+config.reflClipTolerance,0), &D3DXVECTOR3(0,10,0));

	config.showUI = true;

	// Set texture stage LODbias
	if(reset) {
		(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&config.mipmapLODstage0)));
		(*OrgSetTextureStageState)(d3dd, 1, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&config.mipmapLODstage1)));
	}
}

// LightEnable replacement
HRESULT WINAPI NewLightEnable(LPDIRECT3DDEVICE8 fu, DWORD Index, BOOL Enable) {
	//DebugMessage("Light enable %i %i", Index, Enable);
	HRESULT ret = (*OrgLightEnable)(fu, Index, Enable);
	return ret;	
}

// CreateVertexBuffer replacement
HRESULT WINAPI NewCreateVertexBuffer(LPDIRECT3DDEVICE8 fu, UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer8** ppVertexBuffer) {
	//DebugMessage("CreateVertexBuffer %i", ppVertexBuffer);
//	if (FVF & D3DFVF_XYZ) Usage |= D3DUSAGE_NPATCHES;
	HRESULT ret = (*OrgCreateVertexBuffer)(fu, Length, Usage, FVF, Pool, ppVertexBuffer);
	return ret;	
}

// CreateIndexBuffer replacement
HRESULT WINAPI NewCreateIndexBuffer(LPDIRECT3DDEVICE8 fu, UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer8** ppIndexBuffer) {
	//DebugMessage("CreateIndexBuffer %i", ppIndexBuffer);	
	HRESULT ret = (*OrgCreateIndexBuffer)(fu, Length, Usage, Format, Pool, ppIndexBuffer);
	return ret;	
}

int drawPrimitiveCalled = 0;

// DrawIndexedPrimitive replacement
HRESULT WINAPI NewDrawIndexedPrimitive(LPDIRECT3DDEVICE8 fu, D3DPRIMITIVETYPE PrimitiveType, UINT minIndex,UINT NumVertices,UINT startIndex,UINT primCount) {
	//DebugMessage("Draw primitive %i (%i vertices)", PrimitiveType, NumVertices);

	d3dd->GetVertexShader(&currentVShader);

	lastStateRender = stateRender;

	// If using pretransformed shader, sky is already rendered, and ZFunc is 0x8, the UI is being drawn
	if(currentVShader == VSHADER_STATIC && stateRender > RENDER_SKY && renderStates[D3DRS_ZFUNC] == 0x8)
		stateRender = RENDER_UI;

	// Ground being drawn?
	if(mappedTextures[1] == texID[TEX_DET_GROUND] && (currentShader == PSHADER_GROUND || currentShader == PSHADER_GROUND_NIGHT))
		stateRender = RENDER_GROUND;
	else if(stateRender == RENDER_GROUND)  // In case no water is visible
		stateRender = RENDER_OBJECTS;
	

	// Water being drawn?
	if(mappedTextures[1] == texID[TEX_DET_WATER] && (mappedTextures[0] == (int) texID[TEX_WATER1] || mappedTextures[0] == (int) texID[TEX_WATER2] || mappedTextures[0] == (int) texID[TEX_WATER3] || mappedTextures[0] == (int) texID[TEX_WATER4] || mappedTextures[0] == (int) texID[TEX_WATER5] || mappedTextures[0] == (int) texID[TEX_WATER6] || mappedTextures[0] == (int) texID[TEX_WATER7])) {
		stateRender = RENDER_WATER;
		// Grab water height from world matrix
		waterHeight = matWorldSaved._42;

		// Update reflection & clip planes
		D3DXPlaneFromPointNormal(&reflPlane, &D3DXVECTOR3(0,waterHeight,0), &D3DXVECTOR3(0,10,0));
		D3DXPlaneFromPointNormal(&clipPlane, &D3DXVECTOR3(0,waterHeight+config.reflClipTolerance,0), &D3DXVECTOR3(0,10,0));
	} else if(stateRender == RENDER_WATER) { // Horizon is drawn right after water
		stateRender = RENDER_HORIZON;
	}

	if((stateRender == RENDER_HORIZON || stateRender == RENDER_SHADOWS) && currentVShader != VSHADER_STATIC) 	// Objects
		stateRender = RENDER_OBJECTS;	

	if(stateRender == RENDER_OBJECTS && currentVShader == VSHADER_STATIC && renderStates[D3DRS_ZFUNC] != 0x8) 	// Shadows
		stateRender = RENDER_SHADOWS;
	
	/*if(mappedTextures[0] == texID[TEX_TRACER] && stateRender == RENDER_SHADOWS)
		DebugMessage("TRACER! %i", stateRender);*/

	if(mappedTextures[0] == texID[TEX_MAPBACK] && currentVShader == VSHADER_STATIC)
		mapVisible = TRUE;

	if((mappedTextures[0] == texID[TEX_NVG1] || mappedTextures[0] == texID[TEX_NVG2] || mappedTextures[0] == texID[TEX_NVG3] || mappedTextures[0] == texID[TEX_NVG4] || mappedTextures[0] == texID[TEX_NVG5]) && currentVShader == VSHADER_STATIC)
		nvgVisible = TRUE;

	if(stateRender == RENDER_SKY && config.ppGlareDarkenSky && config.ppGlare && config.usePostprocessing && !config.disablePostprocessing)
		(*OrgSetPixelShader)(fu, skyDarkenShader);	// Used to darken sky by 15%

	/*
	// Store ground primitivies
	if(stateRender == RENDER_GROUND) {
		
		if(groundStateBlock == -1) { // TODO: get rid of this
			d3dd->CreateStateBlock(D3DSBT_ALL, &groundStateBlock);	
			DebugMessage("Storing ground primitive state...");
		}

		// Store stream & indices
		d3dd->GetStreamSource(0, &groundPrims[numGroundPrims].vBuffer, &groundPrims[numGroundPrims].stride);
		d3dd->GetIndices(&groundPrims[numGroundPrims].iBuffer, &groundPrims[numGroundPrims].vIndex);

		// Store function parameters
		groundPrims[numGroundPrims].PrimitiveType = PrimitiveType;
		groundPrims[numGroundPrims].minIndex = minIndex;
		groundPrims[numGroundPrims].NumVertices = NumVertices;
		groundPrims[numGroundPrims].startIndex = startIndex;
		groundPrims[numGroundPrims].primCount = primCount;

		// Save matrix
		groundPrims[numGroundPrims].matrixW = matWorldSaved;
		groundPrims[numGroundPrims].matrixV = matViewSaved;
		numGroundPrims++;
		//return D3D_OK;
	}

	// If water is being drawn & a ground state block exists, draw reflection to reflection buffer
	if(stateRender == RENDER_WATER && groundStateBlock != -1) {
		DebugMessage("Drawing ground primitives...");

		// Store current state
		DWORD stateBlock;
		//d3dd->CreateStateBlock(D3DSBT_ALL, &stateBlock);

		// Set ground state
		d3dd->ApplyStateBlock(groundStateBlock);

		// Store current backbuffer etc., set render-to-texture surface & zbuffer
		d3dd->GetViewport(&oldViewport);
		d3dd->GetRenderTarget(&curBuffer);
		d3dd->GetDepthStencilSurface(&pZBuffer);		
		d3dd->SetRenderTarget(pRenderReflSurface, pRenderReflZBSurface);

		if(!config.reflectUseLighting)
			(*OrgSetRenderState)(d3dd, D3DRS_LIGHTING, FALSE);		// No lighting for reflected things
		
		d3dd->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);	// Faces are backwards so invert culling
		d3dd->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x01);	// Enable first clipping plane

		// Setup new viewport for reflection
		d3dd->SetViewport(&reflViewport);

		(*OrgSetPixelShader)(fu, 0);	// Cant use pixel shaders with clipplanes(??)
		d3dd->SetClipPlane(0, clipPlane);


		// Draw primitives to texture
		for(int x =0;x < numGroundPrims;x++) {
			// Set stream, indices & transform
			d3dd->SetStreamSource(0, groundPrims[x].vBuffer, groundPrims[x].stride);
			d3dd->SetIndices(groundPrims[x].iBuffer, groundPrims[x].vIndex);
			d3dd->SetTransform(D3DTS_WORLD, &groundPrims[x].matrixW);

			// Create reflection matrix, reflect view matrix and set clipping plane
			D3DXMatrixReflect(&matReflect, &reflPlane);
			D3DXMatrixMultiply(&matRev, &matReflect, (D3DXMATRIX*) &groundPrims[x].matrixV);
			d3dd->SetTransform(D3DTS_VIEW, &matRev);

			HRESULT ret = (*OrgDrawIndexedPrimitive)(d3dd, groundPrims[x].PrimitiveType, groundPrims[x].minIndex, groundPrims[x].NumVertices, groundPrims[x].startIndex, groundPrims[x].primCount);
		}

		numGroundPrims = 0;

		// Update stats
		stat.trisFrame += primCount;

		// Reset things back to render normally

		d3dd->SetRenderTarget(curBuffer, pZBuffer);
		d3dd->SetViewport(&oldViewport);

		curBuffer->Release();
		pZBuffer->Release();

		// Restore state*/
		/*d3dd->ApplyStateBlock(stateBlock);
		d3dd->DeleteStateBlock(stateBlock);*/
/*
		// Free groundStateBlock
		d3dd->DeleteStateBlock(groundStateBlock);	
		groundStateBlock = -1;
	}*/

	// Render reflection
	// Objects wont be rendered if water isn't visible
	if((config.useReflections) && ((config.reflectTerrain && stateRender == RENDER_GROUND) || (config.reflectObjects && stateRender == RENDER_OBJECTS && waterHeight != 255))) {
		
		float dist = 0;
		float height = 0;

		if(stateRender == RENDER_OBJECTS) {
			float dx = (float) matWorldSaved._41 - (float) matCamera._41;
			float dy = (float) matWorldSaved._43 - (float) matCamera._43;
			height = matWorldSaved._42;
			dist = (float) sqrt(dx*dx + dy*dy);
		}
		
		if((height < config.reflObjMaxHeight || config.reflObjMaxHeight == 0) && (dist < config.reflObjMaxDist || config.reflObjMaxDist == 0) && (stat.numReflObjects < config.reflObjMaxCount || config.reflObjMaxCount == 0)) {

			// Store current backbuffer etc., set render-to-texture surface & zbuffer
			d3dd->GetViewport(&oldViewport);
			d3dd->GetRenderTarget(&curBuffer);
			d3dd->GetDepthStencilSurface(&pZBuffer);		
			d3dd->SetRenderTarget(pRenderReflSurface, pRenderReflZBSurface);

			if(!config.reflectUseLighting)
				(*OrgSetRenderState)(d3dd, D3DRS_LIGHTING, FALSE);		// No lighting for reflected things
			
			d3dd->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);	// Faces are backwards so invert culling
			d3dd->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x01);	// Enable first clipping plane

			// Setup new viewport for reflection
			d3dd->SetViewport(&reflViewport);

			// Create reflection matrix, reflect view matrix and set clipping plane
			D3DXMatrixReflect(&matReflect, &reflPlane);
			D3DXMatrixMultiply(&matRev, &matReflect, (D3DXMATRIX*) &matViewSaved);
			d3dd->SetTransform(D3DTS_VIEW, &matRev);
			d3dd->SetClipPlane(0, clipPlane);

			(*OrgSetPixelShader)(fu, 0);	// Cant use pixel shaders with clipplanes(??)

			// Draw primitive to texture
			(*OrgDrawIndexedPrimitive)(fu, PrimitiveType, minIndex, NumVertices, startIndex, primCount);

			// Update stats
			stat.trisFrame += primCount;
			if(stateRender == RENDER_OBJECTS)
				stat.numReflObjects++;

			// Reset things back to render normally
			(*OrgSetPixelShader)(fu, currentShader);
			d3dd->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x00);
			d3dd->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			if(!config.reflectUseLighting)
				(*OrgSetRenderState)(d3dd, D3DRS_LIGHTING, renderStates[D3DRS_LIGHTING]);
			d3dd->SetRenderTarget(curBuffer, pZBuffer);
			d3dd->SetViewport(&oldViewport);
			D3DXMatrixReflect(&matReflect, &reflPlane);
			D3DXMatrixMultiply(&matRev, &matReflect, (D3DXMATRIX*) &matViewSaved);
			d3dd->SetTransform(D3DTS_VIEW, &matRev);
			curBuffer->Release();
			pZBuffer->Release();
		}
	}


/*
	if(stateRender == RENDER_WATER && config.useReflections) {
		d3dd->CreateStateBlock(D3DSBT_ALL, &waterPrims[numWaterPrims].stateHandle);
		waterPrims[numWaterPrims].PrimitiveType = PrimitiveType;
		waterPrims[numWaterPrims].minIndex = minIndex;
		waterPrims[numWaterPrims].NumVertices = NumVertices;
		waterPrims[numWaterPrims].startIndex = startIndex;
		waterPrims[numWaterPrims].primCount = primCount;
		numWaterPrims++;
		return D3D_OK;
	}

	if(numWaterPrims != 0 && lastStateRender == RENDER_OBJECTS && (stateRender == RENDER_HORIZON || stateRender == RENDER_UI)) {
		drawWaterPrims();
		numWaterPrims = 0;
	}*/

	// Water being drawn?
	if(stateRender == RENDER_WATER && config.useReflections) {
		// EMBM effect for water, stage 0 hold original water, stage 1 holds bumpmap, stage 2 holds reflection texture

		d3dd->SetPixelShader(0);	// Can't use pixel shaders with EMBM

		//d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		//d3dd->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_DESTCOLOR );
		//d3dd->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR );

		(*OrgSetTextureStageState)(fu, 1, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&config.mipmapLODrefl)));
		//(*OrgSetTextureStageState)(fu, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_TEXCOORDINDEX, 0);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_ALPHAOP, D3DTOP_ADD);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_COLORARG2, D3DTA_CURRENT);

		SYSTEMTIME systime;
		GetSystemTime(&systime);
		float phase = (float)systime.wMilliseconds / 1000;
		float cosv = (float)cos(2 * 3.141592 * phase) * config.reflBumpStrength;
		float sinv = (float)sin(2 * 3.141592 * phase) * config.reflBumpStrength;

		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVMAT00, F2DW(cosv));
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVMAT01, F2DW(sinv));
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVMAT10, F2DW(0));
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVMAT11, F2DW(0));
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVLSCALE, F2DW(1.0f)); // not used
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f)); // not used

		(*OrgSetTextureStageState)(fu, 2, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );	// Clamp mapping from sides to avoid wrapping
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );	// Linear filtering
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
        (*OrgSetTextureStageState)(fu, 2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3|D3DTTFF_PROJECTED );
        (*OrgSetTextureStageState)(fu, 2, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION | 2 );

		// Scale bumpmap texture coordinates
		D3DMATRIX matTrans;
		matTrans._11 = 3.9f;
		matTrans._22 = 3.9f;
		matTrans._33 = 3.9f;
		matTrans._44 = 1;

		//matTrans._31 = waterHeight; matTrans._32 = waterHeight;
		//matTrans._31 = reflPos; matTrans._32 = reflPos;

		d3dd->SetTransform( D3DTS_TEXTURE1, &matTrans );

		// Transform reflection texture coordinates
        D3DXMATRIX mat;
		mat._11 = matProjSaved._11/2; mat._12 = 0.0f; mat._13 = 0.0f;
        mat._21 = 0.0f; mat._22 = -matProjSaved._22/2; mat._23 = 0.0f;
        mat._31 = 0.495f; mat._32 = 0.495f; mat._33 = 1.0f;
        mat._41 = 0.0f; mat._42 = 0.0f; mat._43 = 0.0f;
        d3dd->SetTransform( D3DTS_TEXTURE2, &mat );

		if(config.reflectUseExpFog) {
			// Use exponential fog for water, density relative to the real fog distance
			float fogEnd = *((FLOAT*) &renderStates[D3DRS_FOGEND]);
			float fogDensity = (5000/fogEnd)/1500;
			(*OrgSetRenderState)(d3dd, D3DRS_FOGDENSITY, *((DWORD*) (&fogDensity)));
			(*OrgSetRenderState)(d3dd, D3DRS_FOGTABLEMODE, D3DFOG_EXP);
		}

		(*OrgSetTexture)(fu, 1, texWaterBump);
		(*OrgSetTexture)(fu, 2, pRenderReflTexture);

		//(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLORARG2, D3DTA_CURRENT);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		stateDrawWater = TRUE;

	} else if (stateDrawWater == TRUE){
		// Last time we were drawing EMBM water, now reset settings
		stateDrawWater = FALSE;

		if(config.reflectUseExpFog) {
			(*OrgSetRenderState)(d3dd, D3DRS_FOGTABLEMODE, renderStates[D3DRS_FOGTABLEMODE]);		
			(*OrgSetRenderState)(d3dd, D3DRS_FOGDENSITY, renderStates[D3DRS_FOGDENSITY]);		
		}
		//d3dd->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		//d3dd->SetRenderState(D3DRS_SRCBLEND, 5 );
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&config.mipmapLODstage1)));
		//(*OrgSetTextureStageState)(fu, 1, D3DTSS_MIPMAPLODBIAS, (DWORD) config.mipmapLODstage1);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_COLOROP, D3DTOP_DISABLE);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

		LPDIRECT3DSURFACE8 curBuffer = NULL; 
		LPDIRECT3DSURFACE8 curBufferZ = NULL; 
		// Clear reflection render-to-texture surface and z-buffer
		d3dd->GetRenderTarget(&curBuffer);
		d3dd->GetDepthStencilSurface(&curBufferZ);		
		d3dd->SetRenderTarget(pRenderReflSurface, pRenderReflZBSurface);
		d3dd->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(183,194,226),1.0f,0);
		d3dd->SetRenderTarget(curBuffer, curBufferZ);
		curBuffer->Release();
		curBufferZ->Release();
	}

	if(!config.showUI && stateRender == RENDER_UI) // Disable UI
		return D3D_OK;

	if(stateRender == RENDER_UI && config.sharpenUI)	// Makes GUI overlay elemets sharper
		(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_MIPMAPLODBIAS, -2);

	DWORD oldFog;
	DWORD oldAlpha;
	if(config.enhancedTracers && mappedTextures[0] == texID[TEX_TRACER] && 
		stateRender == RENDER_SHADOWS) {
		d3dd->GetRenderState(D3DRS_FOGENABLE, &oldFog);
		d3dd->GetRenderState(D3DRS_ALPHABLENDENABLE, &oldAlpha);
		d3dd->SetRenderState(D3DRS_FOGENABLE, FALSE);
		d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		primCount = 1;
	}

	float fPatch4 = 4.0;
	float fPatch1 = 1.0;

	if (stateRender == RENDER_GROUND) {
//		d3dd->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
//		d3dd->SetRenderState(D3DRS_PATCHSEGMENTS, *((DWORD*)&fPatch4));
	} else {
//		d3dd->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
//		d3dd->SetRenderState(D3DRS_PATCHSEGMENTS, *((DWORD*)&fPatch1));
	}

	HRESULT ret = (*OrgDrawIndexedPrimitive)(fu, PrimitiveType, minIndex, NumVertices, startIndex, primCount);

	if(config.enhancedTracers && mappedTextures[0] == texID[TEX_TRACER] && 
		stateRender == RENDER_SHADOWS) {
		d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE, oldAlpha);
		d3dd->SetRenderState(D3DRS_FOGENABLE, oldFog);
	}

	// Update stats
	stat.trisFrame += primCount;
	if(stateRender == RENDER_OBJECTS)
		stat.numObjects++;

	drawPrimitiveCalled++;

	return ret;
}



// SetTransform replacement
HRESULT WINAPI NewSetTransform(LPDIRECT3DDEVICE8 fu, D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
	//DebugMessage("SetTransform");

	// Store matrices
	switch(State) {
		case D3DTS_WORLD:
			memcpy(&matWorldSaved, pMatrix, sizeof(D3DMATRIX));
			break;
		case D3DTS_VIEW:
			memcpy(&matViewSaved, pMatrix, sizeof(D3DMATRIX));
			break;
		case D3DTS_PROJECTION:
			memcpy(&matProjSaved, pMatrix, sizeof(D3DMATRIX));
			break;
	}
	
	HRESULT ret = (*OrgSetTransform)(fu, State, pMatrix);
	return ret;
}

// MultiplyTransform replacement - Never called
HRESULT WINAPI NewMultiplyTransform(LPDIRECT3DDEVICE8 fu, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix) {

	DebugMessage("Fu");
	switch(State) {
		case D3DTS_WORLD:
			d3dd->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		case D3DTS_VIEW:
			break;
		case D3DTS_PROJECTION:
			break;
	}

	HRESULT ret = (*OrgMultiplyTransform)(fu, State, pMatrix);
	return ret;
}

// CopyRects replacement
HRESULT WINAPI NewUpdateTexture(LPDIRECT3DDEVICE8 fu, IDirect3DBaseTexture8* pSourceTexture,IDirect3DBaseTexture8* pDestinationTexture) {
	DebugMessage("UpdateTexture %i -> %i", pSourceTexture, pDestinationTexture);
	int id = (int) pSourceTexture;

	// Check if this texture is going to overwrite any stored texID's
	for(int x = 0;x < NUM_TEX; x++)
		if((int) pDestinationTexture == texID[x])
			texID[x] = -1;

	if(id == texID[TEX_WATER1]) {
		texID[TEX_WATER1] = (int) pDestinationTexture;
		DebugMessage("Water 1 now %i", texID[TEX_WATER1]);
	} else
	if(id == texID[TEX_WATER2]) {
		texID[TEX_WATER2] = (int) pDestinationTexture;
		DebugMessage("Water 2 now %i", texID[TEX_WATER2]);
	} else
	if(id == texID[TEX_WATER3]) {
		texID[TEX_WATER3] = (int) pDestinationTexture;
		DebugMessage("Water 3 now %i", texID[TEX_WATER3]);
	} else
	if(id == texID[TEX_WATER4]) {
		texID[TEX_WATER4] = (int) pDestinationTexture;
		DebugMessage("Water 4 now %i", texID[TEX_WATER4]);
	} else
	if(id == texID[TEX_WATER5]) {
		texID[TEX_WATER5] = (int) pDestinationTexture;
		DebugMessage("Water 5 now %i", texID[TEX_WATER5]);
	} else
	if(id == texID[TEX_WATER6]) {
		texID[TEX_WATER6] = (int) pDestinationTexture;
		DebugMessage("Water 6 now %i", texID[TEX_WATER6]);
	} else
	if(id == texID[TEX_WATER7])   {
		texID[TEX_WATER7] = (int) pDestinationTexture;
		DebugMessage("Water 7 now %i", texID[TEX_WATER7]);
	} else
	if(id == texID[TEX_DET_GROUND])  {
		texID[TEX_DET_GROUND] = (int) pDestinationTexture;
		DebugMessage("Ground detail now %i", texID[TEX_DET_GROUND]);
	} else
	if(id == texID[TEX_DET_WATER]) {
		texID[TEX_DET_WATER] = (int) pDestinationTexture;
		DebugMessage("Water detail now %i", texID[TEX_DET_WATER]);
	} else
	if(id == texID[TEX_SKY1]) {
		texID[TEX_SKY1] = (int) pDestinationTexture;
		DebugMessage("Sky1 now %i", texID[TEX_SKY1]);
	} else
	if(id == texID[TEX_SKY2]) {
		texID[TEX_SKY2] = (int) pDestinationTexture;
		DebugMessage("Sky2 now %i", texID[TEX_SKY2]);
	} else
	if(id == texID[TEX_SKY3]) {
		texID[TEX_SKY3] = (int) pDestinationTexture;
		DebugMessage("Sky3 now %i", texID[TEX_SKY3]);
	} else
	if(id == texID[TEX_MAPBACK]) {
		texID[TEX_MAPBACK] = (int) pDestinationTexture;
		DebugMessage("Mapback now %i", texID[TEX_MAPBACK]);
	} else
	if(id == texID[TEX_NVG1]) {
		texID[TEX_NVG1] = (int) pDestinationTexture;
		DebugMessage("nvg1 now %i", texID[TEX_NVG1]);
	} else
	if(id == texID[TEX_NVG2]) {
		texID[TEX_NVG2] = (int) pDestinationTexture;
		DebugMessage("nvg1 now %i", texID[TEX_NVG2]);
	} else
	if(id == texID[TEX_NVG3]) {
		texID[TEX_NVG3] = (int) pDestinationTexture;
		DebugMessage("nvg1 now %i", texID[TEX_NVG3]);
	} else
	if(id == texID[TEX_NVG4]) {
		texID[TEX_NVG4] = (int) pDestinationTexture;
		DebugMessage("nvg1 now %i", texID[TEX_NVG4]);
	} else
	if(id == texID[TEX_NVG5]) {
		texID[TEX_NVG5] = (int) pDestinationTexture;
		DebugMessage("nvg1 now %i", texID[TEX_NVG5]);
	} else
	if(id == texID[TEX_TRACER]) {
		texID[TEX_TRACER] = (int) pDestinationTexture;
		DebugMessage("tracer now %i", texID[TEX_TRACER]);
	} else
	if(id == texID[TEX_RAIN]) {
		texID[TEX_RAIN] = (int) pDestinationTexture;
		DebugMessage("rain now %i", texID[TEX_RAIN]);
	}

	HRESULT ret = (*OrgUpdateTexture)(fu, pSourceTexture, pDestinationTexture);
	return ret;
}

// BeginScene replacement
HRESULT WINAPI NewBeginScene(LPDIRECT3DDEVICE8 fu) {
	//DebugMessage("Begin Scene");

	memcpy(&matCamera, &matWorldSaved, sizeof(D3DMATRIX)); // TODO: This is really the player's position!

	if(mapVisible && config.disablePPForMap)
		config.disablePostprocessing = TRUE;
	else
		config.disablePostprocessing = FALSE;

	waterHeight = 255;	// will be overwritten with real value if water if visible
	stateRender = 0;
	numWaterPrims = 0;
	numGroundPrims = 0;
	numObjectPrims = 0;
	mapVisible = FALSE;
	nvgVisible = FALSE;

	drawPrimitiveCalled = 0;

	if(config.usePostprocessing && !config.disablePostprocessing) {
	// Whole scene will be rendered to pRenderSurface
		d3dd->GetRenderTarget(&pBackBuffer);
		d3dd->GetDepthStencilSurface(&pZBuffer);		

		/*// Clear reflection render-to-texture surface and z-buffer
		d3dd->SetRenderTarget(pRenderReflSurface, pRenderReflZBSurface);
		d3dd->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(183,194,226),1.0f,0);
		//d3dd->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(122,137,180),1.0f,0);
		//d3dd->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(255,255,255),1.0f,0);
	*/
		// Clear postprocessing RTT surface
		d3dd->SetRenderTarget(pRenderSurface, pZBuffer);
		pBackBuffer->Release();
		pZBuffer->Release();
		d3dd->Clear(0,NULL,D3DCLEAR_TARGET,D3DCOLOR_XRGB(0,0,0),1.0f,0);
	}

	HRESULT ret = (*OrgBeginScene)(fu);

	(*OrgSetTextureStageState)(d3dd, 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&config.mipmapLODstage0)));
	return ret;
}


// Draws overlay text
void drawOverlayText(void) {
	char string[2048];
	string[0] = 0;

	if(config.showFPS)
		sprintf(string, "FPS: %i\n", stat.FPS);
	if(config.showStat)
		sprintf(string, "%sTris: %i (%i), Objs: %i (%i)\n", string, stat.trisFrame, stat.trisSecond, stat.numObjects, stat.numReflObjects);

	if(config.showHelp)
		sprintf(string, "%s\nKeys:\n%s\n", string, HelpText);

	if(config.showStat)
		sprintf(string, "%s\n Current settings\n      Trilinear: %i\n  NightshaderOR: %i\n      SharpenUI: %i\n        LODbias: %.2f, %.2f\n\n Postprocessing: %i\n     Hard Light: %i (%.1f, %.1f, %.1f)\n     Desaturate: %i (%.1f, %.1f, %.1f)\n          Glare: %i (%.1f, size: %i)\n\n    Reflections: %i (%ix%i, strength: %0.3f)\n        Objects: %i (max: %i, d: %i, h: %i)\n        Terrain: %i\n         Expfog: %i\n       Lighting: %i\n       Cliptolr: %.3f\n", 
			string, config.useTrilinear, config.forceNoNightShader, config.sharpenUI, config.mipmapLODstage0, config.mipmapLODstage1,
			config.usePostprocessing, config.ppHardLight, config.ppHardLightSR, config.ppHardLightSG, config.ppHardLightSB,
			config.ppDesaturate, config.ppDesaturateSR, config.ppDesaturateSG, config.ppDesaturateSB,
			config.ppGlare, config.ppGlareStrength, config.ppGlareSize,
			config.useReflections, config.reflTexSizeX, config.reflTexSizeY, config.reflBumpStrength,
			config.reflectObjects, config.reflObjMaxCount, config.reflObjMaxDist, config.reflObjMaxHeight,
			config.reflectTerrain, config.reflectUseExpFog, config.reflectUseLighting, config.reflClipTolerance);



	if(overlayTime >= stat.tBase)
		strcat(string, overlayText);

	if(strlen(string) > 0) {
		LPDIRECT3DVERTEXBUFFER8 oldStream;
		UINT oldStreamStride;
		d3dd->GetStreamSource(0, &oldStream, &oldStreamStride);

		// Crappy outline
		olFont->DrawText(6, 6, D3DCOLOR_ARGB(255,0,0,0), string);
		olFont->DrawText(4, 4, D3DCOLOR_ARGB(255,0,0,0), string);
		olFont->DrawText(6, 4, D3DCOLOR_ARGB(255,0,0,0), string);
		olFont->DrawText(4, 6, D3DCOLOR_ARGB(255,0,0,0), string);
		// Text
		olFont->DrawText(5, 5, D3DCOLOR_ARGB(255,255,255,0), string);

		d3dd->SetPixelShader(PSHADER_DEFAULT);
		d3dd->SetVertexShader( 0x2c4 );	// TODO: Convert this to proper format
		d3dd->SetStreamSource( 0, NULL, 0);
		d3dd->SetStreamSource( 0, oldStream, oldStreamStride );
		oldStream->Release();
	}
}

// EndScene replacement
HRESULT WINAPI NewEndScene(LPDIRECT3DDEVICE8 fu) {
	//DebugMessage("End scene");
	
	//DebugPrintMatrix(matProjSaved);

	//drawWaterPrims();

	// If anything is stored in groundStateBlock, remove it
	/*if(groundStateBlock != -1) {
		d3dd->DeleteStateBlock(groundStateBlock);	
		groundStateBlock = -1;
	}*/

	if(config.usePostprocessing && !config.disablePostprocessing) {
		// Save current stream source
		LPDIRECT3DVERTEXBUFFER8 oldStream;
		UINT oldStreamStride;
		d3dd->GetStreamSource(0, &oldStream, &oldStreamStride);

		// Set render-to-texture texture (contains the whole scene)
		(*OrgSetTexture)(fu, 0, pRenderTexture);	

		(*OrgSetTextureStageState)(fu, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
		(*OrgSetRenderState)(d3dd, D3DRS_ZWRITEENABLE, FALSE);
		(*OrgSetRenderState)(d3dd, D3DRS_ZENABLE, D3DZB_FALSE);

		// Glare effect
		if(config.ppGlare || (config.useSpecialNVGPP && nvgVisible)) {
			// Set RTT texture to all 4 stages
			(*OrgSetTexture)(fu, 0, pRenderTexture);	
			(*OrgSetTexture)(fu, 1, pRenderTexture);	
			(*OrgSetTexture)(fu, 2, pRenderTexture);	
			(*OrgSetTexture)(fu, 3, pRenderTexture);	
			(*OrgSetTextureStageState)(fu, 0, D3DTSS_TEXCOORDINDEX, 0);
			(*OrgSetTextureStageState)(fu, 1, D3DTSS_TEXCOORDINDEX, 1);
			(*OrgSetTextureStageState)(fu, 2, D3DTSS_TEXCOORDINDEX, 2);
			(*OrgSetTextureStageState)(fu, 3, D3DTSS_TEXCOORDINDEX, 3);
			
			// Fadeoff strength
			if(nvgVisible || config.useSpecialNVGPP) {// Predefined strength for NVG
				D3DXVECTOR4 ppstr(1.0f, 1.0f, 1.0f, (0.6f*stat.msBase)+0.1f);
				d3dd->SetPixelShaderConstant(2, &ppstr, 1);		// Set postprocessing strength
			} else {
				D3DXVECTOR4 ppstr(1.0f, 1.0f, 1.0f, (1.0f*stat.msBase)+0.1f);
				d3dd->SetPixelShaderConstant(2, &ppstr, 1);		// Set postprocessing strength
			}

			// Apply vertical blur
			d3dd->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX4);

			if((isNight && config.ppGlareUseFade) || (nvgVisible && config.useSpecialNVGPP))
				(*OrgSetPixelShader)(fu, blurPass1fadeShader);		
			else
				(*OrgSetPixelShader)(fu, blurPass1Shader);		

			d3dd->SetStreamSource(0, ppBlurV, sizeof(PPBLURVERTEX));
			d3dd->SetRenderTarget(pRenderSurfaceD4, pZBuffer);
			d3dd->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			d3dd->SetStreamSource(0, NULL, 0);

			// Apply horizontal blur
			(*OrgSetTexture)(fu, 0, pRenderTextureD4);	
			(*OrgSetTexture)(fu, 1, pRenderTextureD4);	
			(*OrgSetTexture)(fu, 2, pRenderTextureD4);	
			(*OrgSetTexture)(fu, 3, pRenderTextureD4);

			d3dd->SetStreamSource(0, ppBlurH, sizeof(PPBLURVERTEX));
			d3dd->SetRenderTarget(pRenderSurfaceD8, pZBuffer);
			d3dd->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
			d3dd->SetStreamSource(0, NULL, 0);
		}

		// Set quad stream source
		d3dd->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
		d3dd->SetStreamSource(0, ppQuad, sizeof(PPVERTEX));

		// Hard light
		if(config.ppHardLight) {
			(*OrgSetTexture)(fu, 0, pRenderTexture);	
			d3dd->SetRenderTarget(pRenderSurface, pZBuffer);
			D3DXVECTOR4 ppstr(config.ppHardLightSR*0.99f, config.ppHardLightSG*0.99f, config.ppHardLightSB*0.99f,0);
			d3dd->SetPixelShaderConstant(0, &ppstr, 1);		// Set postprocessing strength
			(*OrgSetPixelShader)(fu, hardLightShader);			// Set postprocessing shader
			d3dd->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		}

		// Desaturate
		if(config.ppDesaturate || (config.useSpecialNVGPP && nvgVisible)) {
			(*OrgSetTexture)(fu, 0, pRenderTexture);	
			d3dd->SetRenderTarget(pRenderSurface, pZBuffer);

			if(nvgVisible && config.useSpecialNVGPP) {// Predefined strength for NVG
				D3DXVECTOR4 ppstr(1.0f, 0.0f, 0.95f, 0);
				d3dd->SetPixelShaderConstant(0, &ppstr, 1);		// Set postprocessing strength
			} else {
				D3DXVECTOR4 ppstr(config.ppDesaturateSR*0.99f, config.ppDesaturateSG*0.99f, config.ppDesaturateSB*0.99f,0);
				d3dd->SetPixelShaderConstant(0, &ppstr, 1);		// Set postprocessing strength
			}

			(*OrgSetPixelShader)(fu, desaturateShader);			// Set postprocessing shader
			d3dd->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		}


		// Draw to backbuffer
		if(config.ppGlare || (config.useSpecialNVGPP && nvgVisible)) {
			// If glare enabled, apply it to current RTT texture

			if(nvgVisible && config.useSpecialNVGPP) {// Predefined strength for NVG
				D3DXVECTOR4 ppstr(0, 0, 0, 1.0f);
				d3dd->SetPixelShaderConstant(0, &ppstr, 1);		// Set postprocessing strength
			} else {
				D3DXVECTOR4 ppstr(0, 0, 0, config.ppGlareStrength*0.99f);
				d3dd->SetPixelShaderConstant(0, &ppstr, 1);		// Set postprocessing strength
			}

			(*OrgSetTexture)(fu, 0, pRenderTexture);
			(*OrgSetTexture)(fu, 1, pRenderTextureD8);
			(*OrgSetTextureStageState)(fu, 1, D3DTSS_TEXCOORDINDEX, 0);
			(*OrgSetPixelShader)(fu, blurCombineShader);
		} else {
			// No glare, just copy frame
			// TODO: Could also do this in the last effect(desatureate or hard light)
			(*OrgSetTexture)(fu, 0, pRenderTexture);
			(*OrgSetPixelShader)(fu, 0);
		}

		d3dd->SetRenderTarget(pBackBuffer, pZBuffer);
		d3dd->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);	
		

		// Reset settings for next frame
		(*OrgSetTexture)(fu, 0, (LPDIRECT3DTEXTURE8) mappedTextures[0]);
		/*(*OrgSetTextureStageState)(fu, 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		(*OrgSetTextureStageState)(fu, 0, D3DTSS_MIPFILTER, D3DTEXF_POINT);*/
		d3dd->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		d3dd->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		d3dd->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_POINT);
		(*OrgSetRenderState)(d3dd, D3DRS_ZWRITEENABLE, renderStates[D3DRS_ZWRITEENABLE]);
		(*OrgSetRenderState)(d3dd, D3DRS_ZENABLE, renderStates[D3DRS_ZENABLE]);

		d3dd->SetPixelShader(PSHADER_DEFAULT);
		d3dd->SetVertexShader( 0x2c4 );	// TODO: Convert this to proper format
		d3dd->SetStreamSource(0, NULL, 0);
		d3dd->SetStreamSource( 0, oldStream, oldStreamStride );
		oldStream->Release();
	}

	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	if(stat.sBase != sysTime.wSecond) {
		//DebugMessage("Tris: %i (%i), Obj: %i, ReflObj: %i", stat.trisFrame, stat.trisSecond, stat.numObjects, stat.numReflObjects);
		
		stat.FPS = stat.frames;

		stat.frames = 0;
		stat.trisSecond = stat.trisSecondC;
		stat.trisSecondC = 0;
		stat.sBase = sysTime.wSecond;
	}

	if (screenShotPressed == 2) {
		// Restore UI, grab shot from this frame
		config.showUI = true;
		screenShotPressed = 1;
	} else if(screenShotPressed == 3) {
		// Hide UI from next frame
		config.showUI = false;
		screenShotPressed = 2;
	} else if (screenShotPressed == 1) {
		screenShotPressed = 0;

		IDirect3DSurface8 *frontBuf;
		D3DLOCKED_RECT lockRect;

		HRESULT res;

		if (d3dd->CreateImageSurface(resX, resY, D3DFMT_A8R8G8B8, &frontBuf) == D3D_OK)
		{
			DebugMessage("screenshot: created ImageSurface");

			if (d3dd->GetFrontBuffer(frontBuf) == D3D_OK)
			{

				DebugMessage("screenshot: GetFrontBuffer == D3D_OK");

				if ((res = frontBuf->LockRect(&lockRect, NULL, D3DLOCK_READONLY)) == D3D_OK) {
					DebugMessage("screenshot: LockRect == D3D_OK");

					HANDLE screenShotFile;
					DWORD bytesWritten;

					DWORD foo;

					char sshotfilename[256];

					sprintf(sshotfilename, "dxdll/screenshots/dxdshot%d.tga", time(NULL));

					screenShotFile = CreateFile(sshotfilename, GENERIC_WRITE, FILE_SHARE_READ, 
						NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (screenShotFile != INVALID_HANDLE_VALUE) {
						DebugMessage("screenshot: CreateFile ok");

						// tga header
						foo = 0;
						// ident header size
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);
						// colour map type
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);
						// image format
						foo = 2;
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);
						// color map start
						foo = 0;
						WriteFile(screenShotFile, &foo, 2, &bytesWritten, NULL);
						// color map length
						WriteFile(screenShotFile, &foo, 2, &bytesWritten, NULL);
						// color map bits
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);
						// origin x
						WriteFile(screenShotFile, &foo, 2, &bytesWritten, NULL);
						// origin y
						WriteFile(screenShotFile, &foo, 2, &bytesWritten, NULL);
						// width
						WriteFile(screenShotFile, &resX, 2, &bytesWritten, NULL);
						// height
						WriteFile(screenShotFile, &resY, 2, &bytesWritten, NULL);
						// bpp
						foo = 32;
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);
						// descriptor
						foo = 0x20;
						WriteFile(screenShotFile, &foo, 1, &bytesWritten, NULL);

						// file data
						WriteFile(screenShotFile, lockRect.pBits, lockRect.Pitch * resY, 
								&bytesWritten, NULL);
						CloseHandle(screenShotFile);
					}
					res = frontBuf->UnlockRect();
					if (res == D3D_OK) {
						DebugMessage("screenshot: front buffer unlock OK");
					} else DebugMessage("screenshot: unlock failed %d", res);

				} else DebugMessage("screenshot: lock failed %d", res);

			}
			int foo = frontBuf->Release();
			DebugMessage("screenshot: buffer release count = %d", foo);
		}
	};

	drawOverlayText();

	// Update tricks & zero framestats
	stat.ltBase = stat.tBase;
	stat.tBase = GetTickCount();
	stat.msBase = (float) (stat.tBase - stat.ltBase)/1000;
	stat.frames++;
	stat.trisSecondC += stat.trisFrame;
	stat.trisFrame = 0;
	stat.numObjects = 0;
	stat.numReflObjects = 0;

	isNight = false;

	// Transform reflection texture
	reflPos = reflPos + (0.03f*(stat.msBase));
	if(reflPos > 1)
		reflPos -= 1;

	HRESULT ret = (*OrgEndScene)(fu);
	return ret;
}

// CreatePixelShader replacement
HRESULT WINAPI NewCreatePixelShader(LPDIRECT3DDEVICE8 fu, CONST DWORD* pFunction,DWORD* pHandle) {
	HRESULT ret = (*OrgCreatePixelShader)(fu, pFunction, pHandle);
	DebugMessage("Create shader %i", *pHandle);
	orgShaders[numOrgShaders++] = *pHandle;
	return ret;
}

// SetPixelShader replacement
HRESULT WINAPI NewSetPixelShader(LPDIRECT3DDEVICE8 fu, DWORD Handle) {
	//DebugMessage("Set shader %i", Handle);
	/*if(Handle != 0 && Handle != PSHADER_SPECULAR && Handle != PSHADER_SPECULAR_NIGHT && Handle != PSHADER_GROUND && Handle != PSHADER_GROUND_NIGHT && Handle != PSHADER_DEFAULT && Handle != PSHADER_DEFAULT_NIGHT && Handle != PSHADER_DECAL && Handle != PSHADER_DECAL_NIGHT)
		DebugMessage("Set unknown pixel shader %i", Handle);*/

	if(Handle == PSHADER_SPECULAR_NIGHT || Handle == PSHADER_GROUND_NIGHT || Handle == PSHADER_DECAL_NIGHT || Handle == PSHADER_DEFAULT_NIGHT)
		isNight = true;

	if(config.forceNoNightShader) {
		// Overwrite night pixelshaders with default ones
		if(Handle == PSHADER_SPECULAR_NIGHT)		Handle = PSHADER_SPECULAR;
		else if(Handle == PSHADER_GROUND_NIGHT)		Handle = PSHADER_GROUND;
		else if(Handle == PSHADER_DECAL_NIGHT)		Handle = PSHADER_DECAL;
		else if(Handle == PSHADER_DEFAULT_NIGHT)	Handle = PSHADER_DEFAULT;
	}

	currentShader = Handle;

	//Handle = testiShader;
	HRESULT ret = (*OrgSetPixelShader)(fu, Handle);
	return ret;
}


// UnlockRect replacement
HRESULT WINAPI NewUnlockRect(LPDIRECT3DTEXTURE8 fu, UINT Level) {
	//DebugMessage("Unlockrect %i %i", fu, Level);
	HRESULT ret = (*OrgUnlockRect)(fu, Level);

	// Get surface description
	D3DSURFACE_DESC surf;
	fu->GetLevelDesc(Level, &surf);

	// Lock the surface again for our own use	
	D3DLOCKED_RECT d3dlr;
	fu->LockRect( Level, &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;
    //DebugMessage("size: %i, %i, x:%i y:%i", d3dlr.Pitch, surf.Size, surf.Width, surf.Height);

	if(Level == 0) { // Calculate CRC if this is the first mipmap
		long crc = crc32(0, (char*) pDst, surf.Size/4);
		DebugMessage("Texture size %ix%i (%i bytes), CRC: %i", surf.Width, surf.Height, surf.Size/4, crc);
		switch (crc) {
			case CRC_WATER1:
				texID[TEX_WATER1] = (int) fu;
				DebugMessage("This is water 1");
			break;
			case CRC_WATER2:
				texID[TEX_WATER2] = (int) fu;
				DebugMessage("This is water 2");
			break;
			case CRC_WATER3:
				texID[TEX_WATER3] = (int) fu;
				DebugMessage("This is water 3");
			break;
			case CRC_WATER4:
				texID[TEX_WATER4] = (int) fu;
				DebugMessage("This is water 4");
			break;
			case CRC_WATER5:
				texID[TEX_WATER5] = (int) fu;
				DebugMessage("This is water 5");
			break;
			case CRC_WATER6:
				texID[TEX_WATER6] = (int) fu;
				DebugMessage("This is water 6");
			break;
			case CRC_WATER7:
				texID[TEX_WATER7] = (int) fu;
				DebugMessage("This is water 7");
			break;
			case CRC_DET_GROUND:
				texID[TEX_DET_GROUND] = (int) fu;
				DebugMessage("This is ground detail");
			break;
			case CRC_DET_WATER:
				texID[TEX_DET_WATER] = (int) fu;
				DebugMessage("This is water detail");
			break;

			case CRC_SKY1:
				texID[TEX_SKY1] = (int) fu;
				DebugMessage("This is sky1");
			break;
			case CRC_SKY2:
				texID[TEX_SKY2] = (int) fu;
				DebugMessage("This is sky2");
			break;
			case CRC_SKY3:
				texID[TEX_SKY3] = (int) fu;
				DebugMessage("This is sky3");
			break;

			case CRC_MAPBACK:
				texID[TEX_MAPBACK] = (int) fu;
				DebugMessage("This is mapbackground");
			break;

			case CRC_TRACER:
				texID[TEX_TRACER] = (int) fu;
				DebugMessage("This is tracer");
			break;

			case CRC_NVG1:
				texID[TEX_NVG1] = (int) fu;
				DebugMessage("This is nvg1");
			break;
			case CRC_NVG2:
				texID[TEX_NVG2] = (int) fu;
				DebugMessage("This is nvg1");
			break;
			case CRC_NVG3:
				texID[TEX_NVG3] = (int) fu;
				DebugMessage("This is nvg1");
			break;
			case CRC_NVG4:
				texID[TEX_NVG4] = (int) fu;
				DebugMessage("This is nvg1");
			break;
			case CRC_NVG5:
				texID[TEX_NVG5] = (int) fu;
				DebugMessage("This is nvg1");
			break;
			case CRC_RAIN:
				texID[TEX_RAIN] = (int) fu;
				DebugMessage("This is rain");
			break;

		}
	}

	(*OrgUnlockRect)(fu, Level);
	return ret;
}


bool hasReplacedNewUnlock = false;
// CreateTexture replacement
HRESULT WINAPI NewCreateTexture(LPDIRECT3DDEVICE8 fu, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture8** ppTexture) {
	DebugMessage("Create texture %i %i %i %i %i %i %i", ppTexture, Width, Height, Levels, Usage, Format, Pool);
	HRESULT ret = (*OrgCreateTexture)(fu, Width, Height, Levels, Usage, Format, Pool, ppTexture);

	if(!hasReplacedNewUnlock) {
		// Grab the UnlockRect function from ppTexture
		DWORD *iface  = (DWORD *) *ppTexture;
		DWORD *vtable = (DWORD *) *iface;

		// Store original functions to global
		OrgUnlockRect = (UNLOCKRECTF) vtable[17];		

		// Protection crap
		DWORD dwOldProtect1;
		HANDLE hSelf = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
		VirtualProtectEx(hSelf, vtable, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect1);

		// Replace function pointer with own functions
		vtable[17]  = (unsigned long) NewUnlockRect; 

		hasReplacedNewUnlock = true;
	}

	return ret;
}

// SetRenderState replacement
HRESULT WINAPI NewSetRenderState(LPDIRECT3DDEVICE8 fu, D3DRENDERSTATETYPE State, DWORD Value) {
	//DebugMessage("Set Render state");
	renderStates[State] = Value;

	if(State == D3DRS_FOGCOLOR && config.ppGlareDarkenSky) {
		// Darken fog
		RGBACOLOR *color = (RGBACOLOR*) &Value;
		color->r = (unsigned char) (color->r * 0.9f);
		color->g = (unsigned char) (color->g * 0.9f);
		color->b = (unsigned char) (color->b * 0.9f);
	}

	HRESULT ret = (*OrgSetRenderState)(fu, State, Value);
	return ret;
}

// SetTextureStage replacement
HRESULT WINAPI NewSetTextureStageState(LPDIRECT3DDEVICE8 fu, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type,DWORD Value) {

	if(config.useTrilinear)
		switch(Type) { 
			// Change texture filtering
			case D3DTSS_MINFILTER:
				Value = D3DTEXF_LINEAR;
				break;
			case D3DTSS_MAGFILTER:
				Value = D3DTEXF_LINEAR;
				break;
			case D3DTSS_MIPFILTER:
				Value = D3DTEXF_LINEAR;
				break;
		}

	//(*OrgSetTextureStageState)(fu, 2, D3DTSS_MIPMAPLODBIAS, -1);

	HRESULT ret = (*OrgSetTextureStageState)(fu, Stage, Type, Value);
	return ret;
}

// SetTexture replacement
HRESULT WINAPI NewSetTexture(LPDIRECT3DDEVICE8 fu, DWORD Stage,IDirect3DBaseTexture8* pTexture) {
	//DebugMessage("Bind texture %i", pTexture);

	// Update mappedTextures array
	mappedTextures[Stage] = (int) pTexture;

	for (int i = 0; i < NUM_TEX; i++) {
		if (texReplace[i] != NULL && texID[i] == (int)pTexture) {
			pTexture = texReplace[i];
			break;
		}
	}

	HRESULT ret = (*OrgSetTexture)(fu, Stage, pTexture);
	return ret;
}
HRESULT WINAPI NewTestCooperativeLevel(LPDIRECT3DDEVICE8 fu) ;

// Reset replacement
HRESULT WINAPI NewReset(LPDIRECT3DDEVICE8 fu, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	//DebugMessage("Reset %i", fu);
	HRESULT ret = (*OrgReset)(fu, pPresentationParameters);
	DebugMessage("Reset result %i", ret);
	olFont->InitDeviceObjects(d3dd);
	olFont->RestoreDeviceObjects();
	initStuff(FALSE);
	return ret;
}

// TestCooperativeLevel replacement
HRESULT WINAPI NewTestCooperativeLevel(LPDIRECT3DDEVICE8 fu) {
	//DebugMessage("Testcooperativelevel %i", fu);
	HRESULT ret = (*OrgTestCooperativeLevel)(fu);
	//DebugMessage("Result %i", ret);

	switch(ret) {
		case D3DERR_DEVICENOTRESET:
			// Release everything
			olFont->InvalidateDeviceObjects();
			pRenderReflTexture->Release();
			pRenderReflSurface->Release();
			pRenderReflZBSurface->Release();
			pRenderTexture->Release();
			pRenderSurface->Release();
			pRenderTextureD4->Release();
			pRenderSurfaceD4->Release();
			pRenderTextureD8->Release();
			pRenderSurfaceD8->Release();

			ppQuad->Release();
			ppBlurV->Release();
			ppBlurH->Release();
			texWaterBump->Release();
			// Shaders will be recreated on reset
			numOrgShaders=0;

			for (int i = 0; i < NUM_TEX; i++)
				if (texReplace[i] != NULL) texReplace[i]->Release();

			DebugMessage("Released objects");
			//olFont->RestoreDeviceObjects();
			break;
	}

	return ret;
}

// Overwrites function pointers from d3dd device
void overwriteD3ddFunctions(void) {
	// Overwrite original functions
	DWORD *iface  = (DWORD *) d3dd;
	DWORD *vtable = (DWORD *) *iface;

	// Protection crap
	DWORD dwOldProtect1;
	HANDLE hSelf = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hSelf, vtable, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect1);

	// Replace function pointers with own functions
	vtable[63]  = (unsigned long) NewSetTextureStageState;
	vtable[61]  = (unsigned long) NewSetTexture;
	vtable[20]  = (unsigned long) NewCreateTexture;
	vtable[88]  = (unsigned long) NewSetPixelShader;
	vtable[37]  = (unsigned long) NewSetTransform;
	vtable[39]  = (unsigned long) NewMultiplyTransform;
	vtable[34]  = (unsigned long) NewBeginScene;
	vtable[35]  = (unsigned long) NewEndScene;
	vtable[29]  = (unsigned long) NewUpdateTexture;
	vtable[71]  = (unsigned long) NewDrawIndexedPrimitive;
	vtable[87]  = (unsigned long) NewCreatePixelShader;
	vtable[46]  = (unsigned long) NewLightEnable;
	vtable[50]  = (unsigned long) NewSetRenderState;
	vtable[23]  = (unsigned long) NewCreateVertexBuffer;
	vtable[24]  = (unsigned long) NewCreateIndexBuffer;
	vtable[3]   = (unsigned long) NewTestCooperativeLevel;
	vtable[14]  = (unsigned long) NewReset;
	vtable[61]  = (unsigned long) NewSetTexture;
}

// CreateDevice replacement
HRESULT WINAPI NewCreateDevice(LPDIRECT3D8 fu, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice8** ppReturnedDeviceInterface) {
	DebugMessage("Createdevice %i", fu);

	if((BehaviorFlags & D3DCREATE_PUREDEVICE))
		BehaviorFlags -= D3DCREATE_PUREDEVICE; // Remove puredevice flag

	// Load config file
	if(!loadConfig()) {
		ShowMessage("dxdll/config.cfg not found!");
		exit(-1);
	}

	pPresentationParameters->AutoDepthStencilFormat = (D3DFORMAT)71;

	DebugMessage("CreateDevice: EnableAutoDepthStencil = %i", 
		pPresentationParameters->EnableAutoDepthStencil);
	DebugMessage("CreateDevice: AutoDepthStencilFormat = %i", 
		pPresentationParameters->AutoDepthStencilFormat);

	// Call the real CreateDevice function
	HRESULT ret = (*OrgCreateDevice)(fu, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	DebugMessage("CreateDevice returned %x", ret);

	d3dd = *ppReturnedDeviceInterface; // Grab D3D Device handle
	resX = pPresentationParameters->BackBufferWidth;	// Grab resolution
	resY = pPresentationParameters->BackBufferHeight;
	DSFormat = pPresentationParameters->AutoDepthStencilFormat;
	BBFormat = pPresentationParameters->BackBufferFormat;

	// Force 32bit postprocessing buffer
	if(config.force32bitPP)
		PPFormat = D3DFMT_X8R8G8B8; 
	else
		PPFormat = BBFormat;

	// Force 32bit glare buffer
	if(config.ppGlareForce32bpp)
		GlareFormat = D3DFMT_X8R8G8B8; 
	else
		GlareFormat = BBFormat;

	DebugMessage("Reset Create %i", d3dd);

	// Create font
	olFont = new CD3DFont(_T("FIXEDSYS"), 12, 0);
	olFont->InitDeviceObjects(d3dd);
	olFont->RestoreDeviceObjects();
	//strcpy(overlayText,"");
 
	stat.tBase = GetTickCount();

	// Allocate space for water (TODO: use dynamic size)
	//waterPrims = new STREAMPRIMITIVE[1024];
	//groundPrims = new STREAMPRIMITIVE[1024];
	//objectPrims = new STREAMPRIMITIVE[10240];

	// "splash" text
	strcpy(overlayText, "DXDLL Active\nPress App-H for help");
	overlayTime = stat.tBase+7000;

	DWORD *iface  = (DWORD *) d3dd;
	DWORD *vtable = (DWORD *) *iface;

	// Store original functions to globals
	OrgTestCooperativeLevel = (TESTCOOPERATIVELEVELF) vtable[3];
	OrgReset = (RESETF) vtable[14];
	OrgSetTextureStageState = (SETTEXTURESTAGEF) vtable[63];
	OrgSetTexture = (SETTEXTUREF) vtable[61];
	OrgCreateTexture = (CREATETEXTUREF) vtable[20]; 
	OrgSetPixelShader = (SETPIXELSHADERF) vtable[88];
	OrgSetTransform = (SETTRANSFORMF) vtable[37];
	OrgMultiplyTransform = (MULTIPLYTRANSFORMF) vtable[39];
	OrgBeginScene = (BEGINSCENEF) vtable[34];
	OrgEndScene = (ENDSCENEF) vtable[35];
	OrgUpdateTexture = (UPDATETEXTUREF) vtable[29];
	OrgDrawIndexedPrimitive = (DRAWINDEXEDPRIMITIVEF) vtable[71];
	OrgCreatePixelShader = (CREATEPIXELSHADERF) vtable[87];
	OrgLightEnable = (LIGHTENABLEF) vtable[46];
	OrgSetRenderState = (SETRENDERSTATEF) vtable[50];	
	OrgCreateVertexBuffer = (CREATEVERTEXBUFFERF) vtable[23];	
	OrgCreateIndexBuffer = (CREATEINDEXBUFFERF) vtable[24];	

	initStuff(TRUE);	// Initialize everything
	return ret;
}



// New d3d create function
extern "C" _declspec(dllexport) IDirect3D8 * __stdcall Direct3DCreate8(UINT SDKVersion) {
	DebugMessage("ProxyDLL Direct3DCreate8 (ver %i)", SDKVersion);

	// Call original from d3d8.dll
	d3dh = (*ODirect3DCreate8)(SDKVersion);

	DWORD *iface = (DWORD *) d3dh;
	DWORD *vtable = (DWORD *) *iface;

	OrgCreateDevice = (CREATEDEVICEF) vtable[15]; // Original function
	DWORD dwOldProtect1;
	HANDLE hSelf = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	VirtualProtectEx(hSelf, vtable, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect1);
	vtable[15] = (unsigned long) NewCreateDevice; // Replace CreateDevice function pointer with pointer to new function

	return d3dh;
}