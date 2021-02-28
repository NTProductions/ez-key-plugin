/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007 Adobe Systems Incorporated                       */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*	Vignette.cpp	

	This is a compiling husk of a project. Fill it in with interesting
	pixel processing code.
	
	Revision History

	Version		Change													Engineer	Date
	=======		======													========	======
	1.0			(seemed like a good idea at the time)					bbb			6/1/2002

	1.0			Okay, I'm leaving the version at 1.0,					bbb			2/15/2006
				for obvious reasons; you're going to 
				copy these files directly! This is the
				first XCode version, though.

	1.0			Let's simplify this barebones sample					zal			11/11/2010

	1.0			Added new entry point									zal			9/18/2017

*/

#include "EZKey.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags =  PF_OutFlag_DEEP_COLOR_AWARE;	// just 16bpc, not 32bpc

	out_data->out_flags2 = PF_OutFlag2_FLOAT_COLOR_AWARE |
						   PF_OutFlag2_SUPPORTS_SMART_RENDER;
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	PF_ADD_COLOR("Key Colour",
		0,
		255,
		0,
		COLOUR_DISK_ID);

	AEFX_CLR_STRUCT(def);


	PF_ADD_FLOAT_SLIDERX("Threshold", 0.0, 100.0, 0.0, 100.0, 100.0, PF_Precision_INTEGER, NULL, NULL, THRESHOLD_DISK_ID);

	
	out_data->num_params = EZKEY_NUM_PARAMS;

	return err;
}

static PF_Err
EZKeyFunc32(
	void* refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel32* inP,
	PF_Pixel32* outP)
{
	PF_Err		err = PF_Err_NONE;

	EZKeyInfo* kiP = reinterpret_cast<EZKeyInfo*>(refcon);
	PF_FpLong	tempF = 0;
	PF_FpLong	threshold = kiP->threshold;
	threshold *= .01;

	bool rImportant = true;
	bool gImportant = false;
	bool bImportant = false;

	if (kiP->colour32.red < kiP->colour32.green) {
		gImportant = true;
	}
	if (kiP->colour32.green < kiP->colour32.blue) {
		bImportant = true;
	}
	if (kiP->colour32.blue > kiP->colour32.red) {
		bImportant = true;
	}

	PF_FpLong	inputLuma;
	PF_FpLong	keyLuma;

	if (rImportant == true) {
		inputLuma = inP->red;
		keyLuma = kiP->colour32.red;
	}

	if (gImportant == true) {
		inputLuma = inP->green;
		keyLuma = kiP->colour32.green;
	}

	if (bImportant == true) {
		inputLuma = inP->blue;
		keyLuma = kiP->colour32.blue;
	}

	PF_FpLong	lowerLumaBound = keyLuma - threshold;
	PF_FpLong	upperLumaBound = keyLuma + threshold;
	if (lowerLumaBound < 0) {
		lowerLumaBound = 0;
	}
	if (upperLumaBound > 3) {
		upperLumaBound = 3;
	}

	if (inputLuma > lowerLumaBound && inputLuma < upperLumaBound) {
		outP->red = 0;
		outP->green = 0;
		outP->blue = 0;
		outP->alpha = 0;
	}
	else {
		outP->red = inP->red;
		outP->green = inP->green;
		outP->blue = inP->blue;
		outP->alpha = inP->alpha;
	}

	return err;
}

static PF_Err
EZKeyFunc16 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;

	EZKeyInfo	*kiP	= reinterpret_cast<EZKeyInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	PF_FpLong	threshold = kiP->threshold;
	threshold *= .01 * 32678;

	bool rImportant = true;
	bool gImportant = false;
	bool bImportant = false;

	if (kiP->colour16.red < kiP->colour16.green) {
		gImportant = true;
	}
	if (kiP->colour16.green < kiP->colour16.blue) {
		bImportant = true;
	}
	if (kiP->colour16.blue > kiP->colour16.red) {
		bImportant = true;
	}

	PF_FpLong	inputLuma;
	PF_FpLong	keyLuma;

	if (rImportant == true) {
		inputLuma = inP->red;
		keyLuma = kiP->colour16.red;
	}

	if (gImportant == true) {
		inputLuma = inP->green;
		keyLuma = kiP->colour16.green;
	}

	if (bImportant == true) {
		inputLuma = inP->blue;
		keyLuma = kiP->colour16.blue;
	}

	PF_FpLong	lowerLumaBound = keyLuma - threshold;
	PF_FpLong	upperLumaBound = keyLuma + threshold;
	if (lowerLumaBound < 0) {
		lowerLumaBound = 0;
	}
	if (upperLumaBound > 98304) {
		upperLumaBound = 98304;
	}

	if (inputLuma > lowerLumaBound && inputLuma < upperLumaBound) {
		outP->red = 0;
		outP->green = 0;
		outP->blue = 0;
		outP->alpha = 0;
	}
	else {
		outP->red = inP->red;
		outP->green = inP->green;
		outP->blue = inP->blue;
		outP->alpha = inP->alpha;
	}

	return err;
}

static PF_Err
EZKeyFunc8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	EZKeyInfo	*kiP	= reinterpret_cast<EZKeyInfo*>(refcon);
	PF_FpLong	tempF	= 0;

	PF_FpLong	threshold = kiP->threshold;
	threshold *= .01 * 255;

	bool rImportant = true;
	bool gImportant = false;
	bool bImportant = false;

	if (kiP->colour8.red < kiP->colour8.green) {
		gImportant = true;
	}
	if (kiP->colour8.green < kiP->colour8.blue) {
		bImportant = true;
	}
	if (kiP->colour8.blue > kiP->colour8.red) {
		bImportant = true;
	}

	PF_FpLong	inputLuma;
	PF_FpLong	keyLuma;

	if (rImportant == true) {
		inputLuma = inP->red;
		keyLuma = kiP->colour8.red;
	}

	if (gImportant == true) {
		inputLuma = inP->green;
		keyLuma = kiP->colour8.green;
	}

	if (bImportant == true) {
		inputLuma = inP->blue;
		keyLuma = kiP->colour8.blue;
	}

	PF_FpLong	lowerLumaBound = keyLuma - threshold;
	PF_FpLong	upperLumaBound = keyLuma + threshold;
	if (lowerLumaBound < 0) {
		lowerLumaBound = 0;
	}
	if (upperLumaBound > 765) {
		upperLumaBound = 765;
	}

	if (inputLuma > lowerLumaBound && inputLuma < upperLumaBound) {
		outP->red = 0;
		outP->green = 0;
		outP->blue = 0;
		outP->alpha = 0;
	}
	else {
		outP->red = inP->red;
		outP->green = inP->green;
		outP->blue = inP->blue;
		outP->alpha = inP->alpha;
	}

		

	return err;
}

static PF_Pixel16
convertRGBA8To16(PF_Pixel8 c8) {
	PF_Pixel16 c16;
	c16.red = c8.red * 128.5019607843137;
	c16.green = c8.green * 128.5019607843137;
	c16.blue = c8.blue * 128.5019607843137;
	c16.alpha = c8.alpha * 128.5019607843137;

	return c16;
}

static PF_Pixel32
convertRGBA8To32(PF_Pixel8 c8) {
	PF_Pixel32 c32;
	c32.red = c8.red * .003921568627451;
	c32.green = c8.green * .003921568627451;
	c32.blue = c8.blue * .003921568627451;
	c32.alpha = c8.alpha * .003921568627451;

	return c32;
}

static PF_Err
ActuallyRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_LayerDef* output,
	PF_EffectWorld* input,
	PF_ParamDef* params[])
{
	PF_Err				err = PF_Err_NONE;
	PF_Err				err2 = PF_Err_NONE;
	PF_PixelFormat format = PF_PixelFormat_INVALID;
	PF_WorldSuite2* wsP = NULL;
	AEGP_CompositeSuite2* poop;

	A_Rect* src_rect;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	EZKeyInfo			biP;
	AEFX_CLR_STRUCT(biP);
	A_long				linesL = 0;


	linesL = output->extent_hint.bottom - output->extent_hint.top;
	PF_Pixel8 colour8 = params[EZKEY_COLOUR]->u.cd.value;
	PF_Pixel16 colour16 = convertRGBA8To16(colour8);
	PF_Pixel32 colour32 = convertRGBA8To32(colour8);
	PF_FpLong threshold = params[EZKEY_THRESHOLD]->u.fs_d.value;

	biP.colour8 = colour8;
	biP.colour16 = colour16;
	biP.colour32 = colour32;
	biP.threshold = threshold;
	

	PF_NewWorldFlags	flags = PF_NewWorldFlag_CLEAR_PIXELS;
	PF_Boolean			deepB = PF_WORLD_IS_DEEP(output);

	if (deepB) {
		flags |= PF_NewWorldFlag_DEEP_PIXELS;
	}


	ERR(AEFX_AcquireSuite(in_data,
		out_data,
		kPFWorldSuite,
		kPFWorldSuiteVersion2,
		"Couldn't load suite.",
		(void**)&wsP));

	if (!err) {

		ERR(wsP->PF_GetPixelFormat(input, &format));

		// pixel depth switch
		switch (format) {
		case PF_PixelFormat_ARGB128:
			ERR(suites.IterateFloatSuite1()->iterate(in_data,
				0,								// progress base
				output->height,							// progress final
				input,	// src 
				NULL,							// area - null for all pixels
				(void*)&biP,					// refcon - your custom data pointer
				EZKeyFunc32,				// pixel function pointer
				output));
		break;
		case PF_PixelFormat_ARGB64:
			ERR(suites.Iterate16Suite1()->iterate(in_data,
				0,								// progress base
				output->height,							// progress final
				input,	// src 
				NULL,							// area - null for all pixels
				(void*)&biP,					// refcon - your custom data pointer
				EZKeyFunc16,				// pixel function pointer
				output));
		break;
		case PF_PixelFormat_ARGB32:
			ERR(suites.Iterate8Suite1()->iterate(in_data,
				0,								// progress base
				output->height,							// progress final
				input,	// src 
				NULL,							// area - null for all pixels
				(void*)&biP,					// refcon - your custom data pointer
				EZKeyFunc8,				// pixel function pointer
				output));
		break;
		}


	}


	ERR2(AEFX_ReleaseSuite(in_data,
		out_data,
		kPFWorldSuite,
		kPFWorldSuiteVersion2,
		"Coludn't release suite."));

	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	//GainInfo			giP;
	//AEFX_CLR_STRUCT(giP);
	//A_long				linesL	= 0;

	//linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	////giP.gainF 	= params[EZKEY_COLOUR]->u.fs_d.value;
	//
	//if (PF_WORLD_IS_DEEP(output)){
	//	ERR(suites.Iterate16Suite1()->iterate(	in_data,
	//											0,								// progress base
	//											linesL,							// progress final
	//											&params[EZKEY_INPUT]->u.ld,	// src 
	//											NULL,							// area - null for all pixels
	//											(void*)&giP,					// refcon - your custom data pointer
	//											MySimpleGainFunc16,				// pixel function pointer
	//											output));
	//} else {
	//	ERR(suites.Iterate8Suite1()->iterate(	in_data,
	//											0,								// progress base
	//											linesL,							// progress final
	//											&params[EZKEY_INPUT]->u.ld,	// src 
	//											NULL,							// area - null for all pixels
	//											(void*)&giP,					// refcon - your custom data pointer
	//											MySimpleGainFunc8,				// pixel function pointer
	//											output));	
	//}

	return err;
}

static PF_Err
PreRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_PreRenderExtra* extra)
{
	PF_Err err = PF_Err_NONE;
	PF_ParamDef channel_param;
	PF_RenderRequest req = extra->input->output_request;
	PF_CheckoutResult in_result;

	//AEFX_CLR_STRUCT(channel_param);

	// Mix in the background color of the comp, as a demonstration of GuidMixInPtr()
	// When the background color changes, the effect will need to be rerendered.
	// Note: This doesn't handle the collapsed comp case
	// Your effect can use a similar approach to trigger a rerender based on changes beyond just its effect parameters.

	req.channel_mask = PF_ChannelMask_RED | PF_ChannelMask_GREEN | PF_ChannelMask_BLUE;

	req.preserve_rgb_of_zero_alpha = FALSE;	//	Hey, we dont care.N


	ERR(extra->cb->checkout_layer(in_data->effect_ref,
		EZKEY_INPUT,
		EZKEY_INPUT,
		&req,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&in_result));

	UnionLRect(&in_result.result_rect, &extra->output->result_rect);
	UnionLRect(&in_result.max_result_rect, &extra->output->max_result_rect);

	//	Notice something missing, namely the PF_CHECKIN_PARAM to balance
	//	the old-fashioned PF_CHECKOUT_PARAM, above? 

	//	For SmartFX, AE automagically checks in any params checked out 
	//	during PF_Cmd_SMART_PRE_RENDER, new or old-fashioned.

	return err;
}

static PF_Err
SmartRender(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_SmartRenderExtra* extra)

{

	PF_Err			err = PF_Err_NONE,
		err2 = PF_Err_NONE;

	PF_EffectWorld* input_worldP = NULL;
	PF_EffectWorld* output_worldP = NULL;

	PF_ParamDef params[EZKEY_NUM_PARAMS];
	PF_ParamDef* paramsP[EZKEY_NUM_PARAMS];

	AEFX_CLR_STRUCT(params);

	for (int i = 0; i < EZKEY_NUM_PARAMS; i++) {
		paramsP[i] = &params[i];
	}

	ERR((extra->cb->checkout_layer_pixels(in_data->effect_ref, EZKEY_INPUT, &input_worldP)));
	ERR(extra->cb->checkout_output(in_data->effect_ref, &output_worldP));

	ERR(PF_CHECKOUT_PARAM(in_data,
		COLOUR_DISK_ID,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&params[EZKEY_COLOUR]));

	ERR(PF_CHECKOUT_PARAM(in_data,
		THRESHOLD_DISK_ID,
		in_data->current_time,
		in_data->time_step,
		in_data->time_scale,
		&params[EZKEY_THRESHOLD]));


	ERR(ActuallyRender(in_data,
		out_data,
		output_worldP,
		input_worldP,
		paramsP));

	ERR2(PF_CHECKIN_PARAM(in_data, &params[EZKEY_COLOUR]));
	ERR2(PF_CHECKIN_PARAM(in_data, &params[EZKEY_THRESHOLD]));

	return err;

}

extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"Vignette", // Name
		"NT EZKey", // Match Name
		"NTProductions", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;

			case PF_Cmd_SMART_PRE_RENDER:
				err = PreRender(in_data, out_data, (PF_PreRenderExtra*)extra);
				break;

			case PF_Cmd_SMART_RENDER:
				err = SmartRender(in_data, out_data, (PF_SmartRenderExtra*)extra);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

