#ifndef _VIS_I_
	#define _VIS_I_

	#ifndef PLUGIN_TYPE
		#define PLUGIN_TYPE PLUGIN_VIS
	#endif

	#ifdef __cplusplus
		extern "C" {
	#endif

	#include <psptypes.h>
	#include <pspgu.h>	

	#define PLUGIN_VIS_VERSION	2

	/* Config is populated by PSPRadio */
	typedef struct _VisPluginConfig
	{
		/* Screen properties */
		int sc_width, sc_height, sc_pitch, sc_pixel_format;

		/* Upper left / Lower right corners (rectangle) for visualization */
		int x1,y1, x2,y2;

		char reserved[32]; 				/* For future use */
	} VisPluginConfig;

	/* GU Functions */
	typedef struct _guFunctions
	{
		void (*sceGuDrawBufferList)(int psm, void* fbp, int fbw);
		void (*sceGuDepthFunc)(int function);
		void (*sceGuDepthMask)(int mask);
		void (*sceGuDepthOffset)(unsigned int offset);
		void (*sceGuDepthRange)(int near, int far);
		void (*sceGuFog)(float near, float far, unsigned int color);
		void (*sceGuBreak)(int a0);
		void (*sceGuContinue)(void);
		void* (*sceGuSetCallback)(int signal, void (*callback)(int));
		void (*sceGuSignal)(int signal, int behavior);
		void (*sceGuSendCommandf)(int cmd, float argument);
		void (*sceGuSendCommandi)(int cmd, int argument);
		void* (*sceGuGetMemory)(int size);
		void (*sceGuStart)(int cid, void* list);
		int (*sceGuFinish)(void);
		int (*sceGuFinishId)(unsigned int id);
		void (*sceGuCallList)(const void* list);
		void (*sceGuCallMode)(int mode);
		int (*sceGuCheckList)(void);
		void (*sceGuSendList)(int mode, const void* list, PspGeContext* context);
		int (*sceGuSync)(int mode, int a1);
		void (*sceGuDrawArray)(int prim, int vtype, int count, const void* indices, const void* vertices);
		void (*sceGuBeginObject)(int vtype, int count, const void* indices, const void* vertices);
		void (*sceGuEndObject)(void);
		void (*sceGuSetStatus)(int state, int status);
		int (*sceGuGetStatus)(int state);
		void (*sceGuSetAllStatus)(int status);
		int (*sceGuGetAllStatus)(void);
		void (*sceGuEnable)(int state);
		void (*sceGuDisable)(int state);
		void (*sceGuLight)(int light, int type, int components, const ScePspFVector3* position);
		void (*sceGuLightAtt)(int light, float atten0, float atten1, float atten2);
		void (*sceGuLightColor)(int light, int component, unsigned int color);
		void (*sceGuLightMode)(int mode);
		void (*sceGuLightSpot)(int light, const ScePspFVector3* direction, float exponent, float cutoff);
		void (*sceGuClear)(int flags);
		void (*sceGuClearColor)(unsigned int color);
		void (*sceGuClearDepth)(unsigned int depth);
		void (*sceGuClearStencil)(unsigned int stencil);
		void (*sceGuPixelMask)(unsigned int mask);
		void (*sceGuColor)(unsigned int color);
		void (*sceGuColorFunc)(int func, unsigned int color, unsigned int mask);
		void (*sceGuColorMaterial)(int components);
		void (*sceGuAlphaFunc)(int func, int value, int mask);
		void (*sceGuAmbient)(unsigned int color);
		void (*sceGuAmbientColor)(unsigned int color);
		void (*sceGuBlendFunc)(int op, int src, int dest, unsigned int srcfix, unsigned int destfix);
		void (*sceGuMaterial)(int mode, int color);
		void (*sceGuModelColor)(unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular);
		void (*sceGuStencilFunc)(int func, int ref, int mask);
		void (*sceGuStencilOp)(int fail, int zfail, int zpass);
		void (*sceGuSpecular)(float power);
		void (*sceGuFrontFace)(int order);
		void (*sceGuLogicalOp)(int op);
		void (*sceGuSetDither)(const ScePspIMatrix4* matrix);
		void (*sceGuShadeModel)(int mode);
		void (*sceGuCopyImage)(int psm, int sx, int sy, int width, int height, int srcw, void* src, int dx, int dy, int destw, void* dest);
		void (*sceGuTexEnvColor)(unsigned int color);
		void (*sceGuTexFilter)(int min, int mag);
		void (*sceGuTexFlush)(void);
		void (*sceGuTexFunc)(int tfx, int tcc);
		void (*sceGuTexImage)(int mipmap, int width, int height, int tbw, const void* tbp);
		void (*sceGuTexLevelMode)(unsigned int a0, float f12);
		void (*sceGuTexMapMode)(int mode, unsigned int a1, unsigned int a2);
		void (*sceGuTexMode)(int tpsm, int maxmips, int a2, int swizzle);
		void (*sceGuTexOffset)(float u, float v);
		void (*sceGuTexProjMapMode)(int mode);
		void (*sceGuTexScale)(float u, float v);
		void (*sceGuTexSlope)(float slope);
		void (*sceGuTexSync)();
		void (*sceGuTexWrap)(int u, int v);
		void (*sceGuClutLoad)(int num_blocks, const void* cbp);
		void (*sceGuClutMode)(unsigned int cpsm, unsigned int a1, unsigned int a2, unsigned int a3);
		void (*sceGuOffset)(unsigned int x, unsigned int y);
		void (*sceGuScissor)(int x, int y, int w, int h);
		void (*sceGuViewport)(int cx, int cy, int width, int height);
		void (*sceGuDrawBezier)(int vtype, int ucount, int vcount, const void* indices, const void* vertices);
		void (*sceGuPatchDivide)(unsigned int ulevel, unsigned int vlevel);
		void (*sceGuPatchFrontFace)(unsigned int a0);
		void (*sceGuPatchPrim)(int prim);
		void (*sceGuDrawSpline)(int vtype, int ucount, int vcount, int uedge, int vedge, const void* indices, const void* vertices);
		void (*sceGuSetMatrix)(int type, const ScePspFMatrix4* matrix);
		void (*sceGuBoneMatrix)(unsigned int index, const ScePspFMatrix4* matrix);
		void (*sceGuMorphWeight)(int index, float weight);
		void (*sceGuDrawArrayN)(int primitive_type, int vertex_type, int count, int a3, const void* indices, const void* vertices);
	} VisPluginGuFunctions;
	
	/* Loosely based on xmms visual plugin to ease porting plugins written for xmms to PSPRadio */
	typedef struct _VisPlugin 
	{
		/* Set by Plugin */
		int  interface_version; 		/* Populate with PLUGIN_VIS_VERSION */
		char *description; 				/* Plugin description */
		char *about; 					/* Plugin about info */
		char need_gu;					/* Set to 1 if plugin needs GU; 0 if not. */
		void (*init)(void); 			/* Called when the plugin is enabled */
		void (*cleanup)(void); 			/* Called when the plugin is disabled */
		void (*playback_start)(void); 	/* not used atm *//* Called when playback starts */
		void (*playback_stop)(void); 	/* not used atm *//* Called when playback stops */
		/* Render the PCM (2ch/44KHz) data, pcm_data has 2 channels interleaved */
		void (*render_pcm)(u32* vram_frame, int16 *pcm_data); 
		/* Render the freq data, don't do anything time consuming in here */
		void (*render_freq)(u32* vram_frame, float freq_data[2][257]); 
		void (*config_update)(); 		/* Called by PSPRadio when config changes */

		/* Set by PSPRadio */
		VisPluginConfig 		*config;	/* Filled in by PSPRadio */
		VisPluginGuFunctions	*gu;		/* GU functions to use in plugin */
	} VisPlugin; 

	/* The plugin exports this function */
	VisPlugin *get_vplugin_info();


	#ifdef __cplusplus
		};
	#endif

#endif
