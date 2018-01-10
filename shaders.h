/* Copyright (C) 2004-2018 DXDLL authors, see included COPYING file */

// The game creates 16 pixel shaders but seems to only use these??
// orgShaders array is filled in newCreatePixelShader function
#define PSHADER_SPECULAR		orgShaders[0]	// Specular light effect
#define PSHADER_SPECULAR_NIGHT	orgShaders[4]	// Specular light effect at night
#define PSHADER_GROUND			orgShaders[1]	// Water & ground multitexture effect (inc. specular light)
#define PSHADER_GROUND_NIGHT	orgShaders[5]	// Water & ground multitexture effect at night (inc. specular light)
#define PSHADER_DEFAULT			orgShaders[8]	// Default shader for sky, objects & UI
#define PSHADER_DEFAULT_NIGHT	orgShaders[12]	// Default shader for objects at night (sky & UI still drawn with PSHADER_DEFAULT)
#define PSHADER_DECAL			orgShaders[9]	// Decals?
#define PSHADER_DECAL_NIGHT		orgShaders[13]	// Decals at night?

LPD3DXBUFFER shaderCode[16];	// Holds assembled shader code
int	numShaders=0;				// Number of compiled shaders

#define SHADERTYPE_NORMAL				0
#define SHADERTYPE_MULTITEX				1
#define SHADERTYPE_SPECULAR				2
#define SHADERTYPE_MULTITEXSPECULAR		3

// Shader ID's for compiled shaders
DWORD hardLightShader;
DWORD desaturateShader;
DWORD blurPass1Shader;
DWORD blurPass1fadeShader;
DWORD blurCombineShader;
DWORD skyDarkenShader;
DWORD waterShader;

// Pixel shaders are defined here

/*
	t0 = water texture
	t1 = bumpamp
	t2 = reflection texture
	c0 = light vector

	ps 1.1
	tex t0
	tex t1
	mov r1,t1
	texbem t2,t1
	add	

*/
const char waterShaderSRC[] = 
	"ps.1.3\n"
	"def c1, 0.0, 1.0, 0.0, 1.0\n"
	"tex t0\n"
	"tex t1\n"
	"add r1,c1,t1_bias\n"
	"dp3_sat r1,c0,r1\n"
	"mul r1,r1,v0\n"
	"mad r0,r1,t0,r1\n";

// Used to make sky darker
const char skyDarkenSRC[] =
		"ps.1.1\n"
		"def c0, 0.85, 0.85, 0.85, 1.0\n"	// Strength
		"tex t0\n"
		"mul r0 , v0, t0\n"	// Multiply texture with color
		"mul r0,  c0, r0\n"; // Darken by c0

// "hard lighting" shader
const char hardLightSRC[] =
		"ps.1.1	\n"
		//"def c0, 0.2, 0.2, 0.9, 0.0 \n"	// Effect strength (RGBA)
		"tex	t0\n"					// Primary texture
		"mov	r0, t0\n"					// Move texture t0 to r0
		"mul	r0, 1-r0, 1-t0\n"			// Inverse multiply of t0 to r0
		"mul	r0, 1-r0, 1-r0\n"			// Inverse multiply of r0 to self
		"lrp	r0, c0, r0, t0\n";			// Linearily interpolate original texture based on c0

// desaturate shader
const char desaturateSRC[] =
		"ps.1.1	\n"
		//"def c0, 0.2, 0.2, 0.9, 0.0 \n"	// Effect strength (RGBA)
		"def	c1, 0.333, 0.333, 0.333, 0.0\n"	// DP3 amount
		"tex	t0\n"	
		"dp3	r0, t0, c1\n"	// "Desaturate" to r0
		"lrp	r0, c0, r0, t0\n";	// Linearily interpolate original texture based on c0

const char blurPass1SRC[] =
		"ps.1.1\n"
		//"def c0, 0,0,0,0.5\n"
		"def c0, 0.5,0.5,0.5,0.33\n"
		"def c1, 1,1,1,1.0\n"
		"tex t0\n"
		"tex t1\n"
		"tex t2\n"
		"tex t3\n"
		"lrp r1, c0.a, t0, t1\n"	// Interpolate textures to each other
		"lrp r0, c0.a, t3, t2\n"
		"lrp r0.rgb, c0, r0, r1\n"
		"+mov r0.a, c1.a\n";	// Fill alpha with 1.0

const char blurPass1fadeSRC[] =
		"ps.1.1\n"
		//"def c0, 0,0,0,0.5\n"
		"def c0, 0.5,0.5,0.5,0.33\n"
		"def c1, 1,1,1,1.0\n"
		"tex t0\n"
		"tex t1\n"
		"tex t2\n"
		"tex t3\n"
		"lrp r1, c0.a, t0, t1\n"	// Interpolate textures to each other
		"lrp r0, c0.a, t3, t2\n"
		"lrp r0.rgb, c0, r0, r1\n"

		"dp3 r1, r0, c2\n"		// Dot product to r1
		"mov r0.a, r1.a\n"
		"add r0.a, r0.a, c2\n"; // Avoid "burning" of images to the buffer
		//"+mov r0.a, c1.a\n";	// Fill alpha with 1.0

const char blurCombineSRC[] =
		"ps.1.1\n"
		//"def	c0, 0.0, 0.0, 0.0, 0.3\n" // Effect strength (RGBA)
		"def c1, 1, 1, 1, 1\n"
		"tex	t0\n"	// Original texture
		"tex	t1\n"	// Blurred texure
		"mad r0.rgb, c0.a, t1, t0\n"	// Combine blur textures
		"+mov r0.a, c1.a\n";		// Fill alpha with 1.0
		//"add_d2	r0, t0, t1\n"	// Combine blur textures
		//"mad	r0, c0.a, r0, t0\n";	// Combine original with blur, based on c0
