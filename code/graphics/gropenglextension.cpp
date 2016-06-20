/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/




#ifdef _WIN32
#include <windows.h>
#endif

#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "osapi/outwnd.h"
#include "popup/popup.h"

char *OGL_extension_string = NULL;

ogl_extension GL_extension_info[NUM_GL_EXTENSIONS] =
{
	{ GL_EXTENSION_EXT_FOG_COORD,					false,	"GL_EXT_fog_coord" },
	{ GL_EXTENSION_ARB_MULTITEXTURE,				true,	"GL_ARB_multitexture" },
	{ GL_EXTENSION_ARB_TEXTURE_ENV_ADD,				true,	"GL_ARB_texture_env_add" },
	{ GL_EXTENSION_ARB_TEXTURE_COMPRESSION,			false,	"GL_ARB_texture_compression" },
	{ GL_EXTENSION_EXT_TEXTURE_COMPRESSION_S3TC,	false,	"GL_EXT_texture_compression_s3tc" },
	{ GL_EXTENSION_EXT_TEXTURE_FILTER_ANISOTROPIC,	false,	"GL_EXT_texture_filter_anisotropic" },
	{ GL_EXTENSION_ARB_TEXTURE_ENV_COMBINE,			false,	"GL_ARB_texture_env_combine" },
	{ GL_EXTENSION_EXT_COMPILED_VERTEX_ARRAY,		false,	"GL_EXT_compiled_vertex_array" },
	{ GL_EXTENSION_EXT_DRAW_RANGE_ELEMENTS,			true,	"GL_EXT_draw_range_elements" },
	{ GL_EXTENSION_ARB_TEXTURE_MIRRORED_REPEAT,		false,	"GL_ARB_texture_mirrored_repeat" },
	{ GL_EXTENSION_ARB_TEXTURE_NON_POWER_OF_TWO,	false,	"GL_ARB_texture_non_power_of_two" },
	{ GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT,		false,	"GL_ARB_vertex_buffer_object" },
	{ GL_EXTENSION_ARB_PIXEL_BUFFER_OBJECT,			false,	"GL_ARB_pixel_buffer_object" },
	{ GL_EXTENSION_SGIS_GENERATE_MIPMAP,			false,	"GL_SGIS_generate_mipmap" },
	{ GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT,			false,	"GL_EXT_framebuffer_object" },
	{ GL_EXTENSION_ARB_TEXTURE_RECTANGLE,			false,	"GL_ARB_texture_rectangle" }, // "GL_EXT_texture_rectangle"
	{ GL_EXTENSION_EXT_BGRA,						true,	"GL_EXT_bgra" },
	{ GL_EXTENSION_ARB_TEXTURE_CUBE_MAP,			false,	"GL_ARB_texture_cube_map" },
	{ GL_EXTENSION_EXT_TEXTURE_LOD_BIAS,			false,	"GL_EXT_texture_lod_bias" },
	{ GL_EXTENSION_ARB_POINT_SPRITE,				false,	"GL_ARB_point_sprite" },
	{ GL_EXTENSION_ARB_SHADING_LANGUAGE_100,		false,	"GL_ARB_shading_language_100" },
	{ GL_EXTENSION_ARB_SHADER_OBJECTS,				false,	"GL_ARB_shader_objects" },
	{ GL_EXTENSION_ARB_VERTEX_PROGRAM,				false,	"GL_ARB_vertex_program" },
	{ GL_EXTENSION_ARB_VERTEX_SHADER,				false,	"GL_ARB_vertex_shader" },
	{ GL_EXTENSION_ARB_FRAGMENT_SHADER,				false,	"GL_ARB_fragment_shader" },
	{ GL_EXTENSION_SM30,							false,	"GL_ARB_shader_texture_lod" },
	{ GL_EXTENSION_ARB_FLOATING_POINT_TEXTURES,		false,	"GL_ARB_texture_float" },
	{ GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX,	false,	"GL_ARB_draw_elements_base_vertex" },
	{ GL_EXTENSION_EXT_FRAMEBUFFER_BLIT,			false,	"GL_EXT_framebuffer_blit" },
	{ GL_EXTENSION_ARB_GEOMETRY_SHADER4,			false,	"GL_ARB_geometry_shader4" },
	{ GL_EXTENSION_EXT_TEXTURE_ARRAY,				false,	"GL_EXT_texture_array" },
	{ GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT,		false,	"GL_ARB_uniform_buffer_object" },
	{ GL_EXTENSION_EXT_TRANSFORM_FEEDBACK,			false,	"GL_EXT_transform_feedback" },
	{ GL_EXTENSION_ARB_DRAW_INSTANCED,				false,	"GL_ARB_draw_instanced" },
	{ GL_EXTENSION_ARB_TEXTURE_BUFFER,				false,	"GL_ARB_texture_buffer_object" },
	{ GL_EXTENSION_EXT_GPU_SHADER4,					false,	"GL_EXT_gpu_shader4" },
	{ GL_EXTENSION_ARB_VERTEX_ARRAY_OBJECT,			false,	"GL_ARB_vertex_array_object" }
};

ogl_function GL_function_info[NUM_GL_FUNCTIONS] =
{
	{ GL_FUNC_MULTI_TEX_COORD_2F,						GL_EXTENSION_ARB_MULTITEXTURE, 0, "glMultiTexCoord2fARB", 0 },
	{ GL_FUNC_ACTIVE_TEXTURE,							GL_EXTENSION_ARB_MULTITEXTURE, 0, "glActiveTextureARB", 0 },
	{ GL_FUNC_CLIENT_ACTIVE_TEXTURE,					GL_EXTENSION_ARB_MULTITEXTURE, 0, "glClientActiveTextureARB", 0 },
	{ GL_FUNC_COMPRESSED_TEX_IMAGE_2D,					GL_EXTENSION_ARB_TEXTURE_COMPRESSION, 0, "glCompressedTexImage2D", 0 },
	{ GL_FUNC_COMPRESSED_TEX_SUB_IMAGE_2D,				GL_EXTENSION_ARB_TEXTURE_COMPRESSION, 0, "glCompressedTexSubImage2D", 0 },
	{ GL_FUNC_GET_COMPRESSED_TEX_IMAGE,					GL_EXTENSION_ARB_TEXTURE_COMPRESSION, 0, "glGetCompressedTexImageARB", 0 },
	{ GL_FUNC_LOCK_ARRAYS,								GL_EXTENSION_EXT_COMPILED_VERTEX_ARRAY, 0, "glLockArraysEXT", 0 },
	{ GL_FUNC_UNLOCK_ARRAYS,							GL_EXTENSION_EXT_COMPILED_VERTEX_ARRAY, 0, "glUnlockArraysEXT", 0 },
	{ GL_FUNC_DRAW_RANGE_ELEMENTS,						GL_EXTENSION_EXT_DRAW_RANGE_ELEMENTS, 0, "glDrawRangeElements", 0 },
	{ GL_FUNC_BIND_BUFFER,								GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glBindBufferARB", 0 },
	{ GL_FUNC_DELETE_BUFFERS,							GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glDeleteBuffersARB", 0 },
	{ GL_FUNC_GEN_BUFFERS,								GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glGenBuffersARB", 0 },
	{ GL_FUNC_BUFFER_DATA,								GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glBufferDataARB", 0 },
	{ GL_FUNC_BUFFER_SUB_DATA,							GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glBufferSubDataARB", 0 },
	{ GL_FUNC_MAP_BUFFER,								GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glMapBufferARB", 0 },
	{ GL_FUNC_UNMAP_BUFFER,								GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT, 0, "glUnmapBufferARB", 0 },
	{ GL_FUNC_IS_RENDERBUFFER,							GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glIsRenderbufferEXT", 0 },
	{ GL_FUNC_BIND_RENDERBUFFER,						GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glBindRenderbufferEXT", 0 },
	{ GL_FUNC_DELETE_RENDERBUFFERS,						GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glDeleteRenderbuffersEXT", 0 },
	{ GL_FUNC_GEN_RENDERBUFFERS,						GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glGenRenderbuffersEXT", 0 },
	{ GL_FUNC_RENDERBUFFER_STORAGE,						GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glRenderbufferStorageEXT", 0 },
	{ GL_FUNC_GET_RENDERBUFFER_PARAMETER_IV,			GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glGetRenderbufferParameterivEXT", 0 },
	{ GL_FUNC_IS_FRAMEBUFFER,							GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glIsFramebufferEXT", 0 },
	{ GL_FUNC_BIND_FRAMEBUFFER,							GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glBindFramebufferEXT", 0 },
	{ GL_FUNC_DELETE_FRAMEBUFFERS,						GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glDeleteFramebuffersEXT", 0 },
	{ GL_FUNC_GEN_FRAMEBUFFERS,							GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glGenFramebuffersEXT", 0 },
	{ GL_FUNC_CHECK_FRAMEBUFFER_STATUS,					GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glCheckFramebufferStatusEXT", 0 },
	{ GL_FUNC_FRAMEBUFFER_TEXTURE_2D,					GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glFramebufferTexture2DEXT", 0 },
	{ GL_FUNC_FRAMEBUFFER_RENDERBUFFER,					GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glFramebufferRenderbufferEXT", 0 },
	{ GL_FUNC_GET_FRAMEBUFFER_ATTACHMENT_PARAMETER_IV,	GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glGetFramebufferAttachmentParameterivEXT", 0 },
	{ GL_FUNC_GENERATE_MIPMAP,							GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT, 0, "glGenerateMipmapEXT", 0 },
	{ GL_FUNC_DRAWBUFFERS,								GL_EXTENSION_NONE, 20, "glDrawBuffers", 0 },
	{ GL_FUNC_DELETE_OBJECT,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glDeleteObjectARB", 0 },
	{ GL_FUNC_CREATE_SHADER_OBJECT,						GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glCreateShaderObjectARB", 0 },
	{ GL_FUNC_SHADER_SOURCE,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glShaderSourceARB", 0 },
	{ GL_FUNC_COMPILE_SHADER,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glCompileShaderARB", 0 },
	{ GL_FUNC_GET_OBJECT_PARAMETERIV,					GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glGetObjectParameterivARB", 0 },
	{ GL_FUNC_GET_INFO_LOG,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glGetInfoLogARB", 0 },
	{ GL_FUNC_CREATE_PROGRAM_OBJECT,					GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glCreateProgramObjectARB", 0 },
	{ GL_FUNC_ATTACH_OBJECT,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glAttachObjectARB", 0 },
	{ GL_FUNC_LINK_PROGRAM,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glLinkProgramARB", 0 },
	{ GL_FUNC_USE_PROGRAM_OBJECT,						GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUseProgramObjectARB", 0 },
	{ GL_FUNC_VALIDATE_PROGRAM,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glValidateProgramARB", 0 },
	{ GL_FUNC_GET_UNIFORM_LOCATION,						GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glGetUniformLocationARB", 0 },
	{ GL_FUNC_GET_UNIFORMIV,							GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glGetUniformivARB", 0 },
	{ GL_FUNC_UNIFORM1F,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform1fARB", 0 },
	{ GL_FUNC_UNIFORM2F,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform2fARB", 0 },
	{ GL_FUNC_UNIFORM3F,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform3fARB", 0 },
	{ GL_FUNC_UNIFORM4F,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform4fARB", 0 },
	{ GL_FUNC_UNIFORM1FV,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform1fvARB", 0 },
	{ GL_FUNC_UNIFORM3FV,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform3fvARB", 0 },
	{ GL_FUNC_UNIFORM4FV,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform4fvARB", 0 },
	{ GL_FUNC_UNIFORM1I,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform1iARB", 0 },
	{ GL_FUNC_UNIFORM1IV,								GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniform1ivARB", 0 },
	{ GL_FUNC_UNIFORM_MATRIX4FV,						GL_EXTENSION_ARB_SHADER_OBJECTS, 0, "glUniformMatrix4fvARB", 0 },
	{ GL_FUNC_ENABLE_VERTEX_ATTRIB_ARRAY,				GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glEnableVertexAttribArrayARB", 0 },
	{ GL_FUNC_DISABLE_VERTEX_ATTRIB_ARRAY,				GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glDisableVertexAttribArrayARB", 0 },
	{ GL_FUNC_VERTEX_ATTRIB_POINTER,					GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glVertexAttribPointerARB", 0 },
	{ GL_FUNC_VERTEX_ATTRIB1F,							GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glVertexAttrib1fARB", 0 },
	{ GL_FUNC_VERTEX_ATTRIB2F,							GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glVertexAttrib2fARB", 0 },
	{ GL_FUNC_VERTEX_ATTRIB3F,							GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glVertexAttrib3fARB", 0 },
	{ GL_FUNC_VERTEX_ATTRIB4F,							GL_EXTENSION_ARB_VERTEX_PROGRAM, 0, "glVertexAttrib4fARB", 0 },
	{ GL_FUNC_GET_ATTRIB_LOCATION,						GL_EXTENSION_ARB_VERTEX_SHADER, 0, "glGetAttribLocationARB", 0 },
	{ GL_FUNC_BIND_ATTRIB_LOCATION,						GL_EXTENSION_ARB_VERTEX_SHADER, 0, "glBindAttribLocationARB", 0 },
	{ GL_FUNC_DRAW_ELEMENTS_BASE_VERTEX,				GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX, 0, "glDrawElementsBaseVertex", 0	},
	{ GL_FUNC_DRAW_RANGE_ELEMENTS_BASE_VERTEX,			GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX, 0, "glDrawRangeElementsBaseVertex", 0 },
	{ GL_FUNC_DRAW_ELEMENTS_INSTANCED_BASE_VERTEX,		GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX, 0, "glDrawElementsInstancedBaseVertex", 0 },
	{ GL_FUNC_MULTI_DRAW_ELEMENTS_BASE_VERTEX,			GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX, 0, "glMultiDrawElementsBaseVertex", 0 },
	{ GL_FUNC_BLITFRAMEBUFFER,							GL_EXTENSION_EXT_FRAMEBUFFER_BLIT, 0, "glBlitFramebufferEXT", 0},
	{ GL_FUNC_PROGRAM_PARAMETERI,						GL_EXTENSION_ARB_GEOMETRY_SHADER4, 0, "glProgramParameteriARB", 0 },
	{ GL_FUNC_FRAMEBUFFER_TEXTURE,						GL_EXTENSION_ARB_GEOMETRY_SHADER4, 0, "glFramebufferTextureARB", 0 },
	{ GL_FUNC_TEXIMAGE3D,								GL_EXTENSION_NONE, 12, "glTexImage3D", 0 },
	{ GL_FUNC_GET_UNIFORM_INDICES,						GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glGetUniformIndices", 0 },
	{ GL_FUNC_GET_ACTIVE_UNIFORMS_IV,					GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glGetActiveUniformsiv", 0 },
	{ GL_FUNC_GET_ACTIVE_UNIFORM_NAME,					GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glGetActiveUniformName", 0 },
	{ GL_FUNC_GET_UNIFORM_BLOCK_INDEX,					GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glGetUniformBlockIndex", 0 },
	{ GL_FUNC_GET_ACTIVE_UNIFORM_BLOCK_IV,				GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glGetActiveUniformBlockName", 0 },
	{ GL_FUNC_UNIFORM_BLOCK_BINDING,					GL_EXTENSION_ARB_UNIFORM_BUFFER_OBJECT, 0, "glUniformBlockBinding", 0 },
	{ GL_FUNC_BEGIN_TRANSFORM_FEEDBACK,					GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glBeginTransformFeedbackEXT", 0 },
	{ GL_FUNC_END_TRANSFORM_FEEDBACK,					GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glEndTransformFeedbackEXT", 0 },
	{ GL_FUNC_BIND_BUFFER_RANGE,						GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glBindBufferRangeEXT", 0 },
	{ GL_FUNC_BIND_BUFFER_OFFSET,						GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glBindBufferOffsetEXT", 0 },
	{ GL_FUNC_BIND_BUFFER_BASE,							GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glBindBufferBaseEXT", 0 },
	{ GL_FUNC_TRANSFORM_FEEDBACK_VARYINGS,				GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glTransformFeedbackVaryingsEXT", 0 },
	{ GL_FUNC_GET_TRANSFORM_FEEDBACK_VARYING,			GL_EXTENSION_EXT_TRANSFORM_FEEDBACK, 0, "glGetTransformFeedbackVaryingEXT", 0 },
	{ GL_FUNC_ARB_DRAW_ARRAYS_INSTANCED,				GL_EXTENSION_ARB_DRAW_INSTANCED, 0, "glDrawArraysInstancedARB", 0	},
	{ GL_FUNC_ARB_DRAW_ELEMENTS_INSTANCED,				GL_EXTENSION_ARB_DRAW_INSTANCED, 0, "glDrawElementsInstancedARB", 0 },
	{ GL_FUNC_ARB_TEX_BUFFER,							GL_EXTENSION_ARB_TEXTURE_BUFFER, 0, "glTexBufferARB", 0 },
	{ GL_FUNC_BIND_FRAG_DATA_LOCATION,					GL_EXTENSION_NONE, 30, "glBindFragDataLocation", 0 },
	{ GL_FUNC_GET_FRAG_DATA_LOCATION,					GL_EXTENSION_NONE, 30, "glGetFragDataLocation", 0 },
	{ GL_FUNC_BIND_VERTEX_ARRAY,						GL_EXTENSION_ARB_VERTEX_ARRAY_OBJECT, 0, "glBindVertexArray", 0 },
	{ GL_FUNC_DELETE_VERTEX_ARRAYS,						GL_EXTENSION_ARB_VERTEX_ARRAY_OBJECT, 0, "glDeleteVertexArrays", 0 },
	{ GL_FUNC_GEN_VERTEX_ARRAYS,						GL_EXTENSION_ARB_VERTEX_ARRAY_OBJECT, 0, "glGenVertexArrays", 0 },
	{ GL_FUNC_IS_VERTEX_ARRAY,							GL_EXTENSION_ARB_VERTEX_ARRAY_OBJECT, 0, "glIsVertexArray", 0 }
};

// special extensions (only special functions are supported at the moment)
ogl_spc_function GL_special_function_info[NUM_GL_SPC_FUNCTIONS] = {
	{ GL_SPC_FUNC_WGL_SWAP_INTERVAL, "wglSwapIntervalEXT" },
	{ GL_SPC_FUNC_GLX_SWAP_INTERVAL, "glXSwapIntervalSGI" }
};

bool GL_extensions_availability[NUM_GL_EXTENSIONS] = { false };
ptr_u GL_function_ptrs[NUM_GL_FUNCTIONS] = { 0 };
ptr_u GL_spc_function_ptrs[NUM_GL_SPC_FUNCTIONS] = { 0 };

#ifdef _WIN32
#define GET_PROC_ADDRESS(x)		wglGetProcAddress((x))
#else
#define GET_PROC_ADDRESS(x)		SDL_GL_GetProcAddress((x))
#endif

//tries to find a certain extension
static inline int opengl_find_extension(const char *ext_to_find)
{
	if (OGL_extension_string == NULL)
		return 0;

	return ( strstr(OGL_extension_string, ext_to_find) != NULL );
}

// these extensions may not be listed the normal way so we don't check the extension string for them
static int opengl_get_extensions_special()
{
	int i, num_found = 0;
	ogl_spc_function *func = NULL;

	for (i = 0; i < NUM_OGL_EXT_SPECIAL; i++) {
		func = &GL_special_function_info[i];

		Assert( func->function_name != NULL );

		GL_spc_function_ptrs[func->func_handle] = (ptr_u)GET_PROC_ADDRESS(func->function_name);

		if ( GL_spc_function_ptrs[func->func_handle] ) {
			mprintf(("  Found special extension function \"%s\".\n", func->function_name));
			num_found++;
		}
	}

	return num_found;
}

//finds OGL extension functions
//returns number found
int opengl_get_extensions()
{
	int i, j, num_found = 0;
	ogl_extension *ext = NULL;
	ogl_function *func = NULL;
	
	OGL_extension_string = (char*)glGetString(GL_EXTENSIONS);

	for (i = 0; i < NUM_GL_EXTENSIONS; i++) {
		ext = &GL_extension_info[i];
		
		GL_extension_handle current_ext = ext->ext_handle;
		int num_functions = 0;

		if ( !opengl_find_extension(ext->extension_name) ) {
			GL_extensions_availability[current_ext] = false;
			mprintf(("  Unable to find extension \"%s\".\n", ext->extension_name));

			if ( ext->required_to_run )
				Error(LOCATION, "The required OpenGL extension '%s' is not supported by your current driver version or graphics card.\n", ext->extension_name);

			continue;
		}

		// count the number of functions required by this extension
		for ( j = 0; j < NUM_GL_FUNCTIONS; ++j ) {
			if ( GL_function_info[j].ext_handle == current_ext ) {
				++num_functions;
			}
		}

		// some extensions do not have functions
		if ( !num_functions ) {
			mprintf(("  Using extension \"%s\".\n", ext->extension_name));
			GL_extensions_availability[current_ext] = true;
			num_found++;
			continue;
		}

		// we do have functions so check any/all of them
		for ( j = 0; j < NUM_GL_FUNCTIONS; j++ ) {
			if ( GL_function_info[j].ext_handle != current_ext ) {
				continue;
			}

			func = &GL_function_info[j];

			if (func == NULL)
				break;

			ptr_u &func_ptr = GL_function_ptrs[func->func_handle];
			if ( !func_ptr )
				func_ptr = (ptr_u)GET_PROC_ADDRESS(func->function_name);

			if ( !func_ptr )
				break;
		}

		if ( j != num_functions ) {
			GL_extensions_availability[current_ext] = false;
			mprintf(("  Found extension \"%s\", but can't find the required function \"%s()\".  Extension will be disabled!\n", ext->extension_name, func->function_name));

			if (ext->required_to_run)
				Error( LOCATION, "The required OpenGL extension '%s' is not fully supported by your current driver version or graphics card.\n", ext->extension_name);
		} else {
			mprintf(("  Using extension \"%s\".\n", ext->extension_name));
			GL_extensions_availability[current_ext] = true;
			num_found++;
		}
	}

	num_found += opengl_get_extensions_special();

	mprintf(( "\n" ));

	return num_found;
}


extern int OGL_fogmode;

void opengl_extensions_init()
{
	opengl_get_extensions();

	// if S3TC compression is found, then "GL_ARB_texture_compression" must be an extension
	Use_compressed_textures = Is_Extension_Enabled(GL_EXTENSION_EXT_TEXTURE_COMPRESSION_S3TC);
	Texture_compression_available = Is_Extension_Enabled(GL_EXTENSION_ARB_TEXTURE_COMPRESSION);
	
	//allow VBOs to be used
	if ( !Cmdline_nohtl && !Cmdline_novbo && Is_Extension_Enabled(GL_EXTENSION_ARB_VERTEX_BUFFER_OBJECT) ) {
		Use_VBOs = 1;
	}

	if ( !Cmdline_no_pbo && Is_Extension_Enabled(GL_EXTENSION_ARB_PIXEL_BUFFER_OBJECT) ) {
		Use_PBOs = 1;
	}

	// setup the best fog function found
	if ( !Fred_running ) {
		if ( Is_Extension_Enabled(GL_EXTENSION_EXT_FOG_COORD) ) {
			OGL_fogmode = 2;
		} else {
			OGL_fogmode = 1;
		}
	}

	// if we can't do cubemaps then turn off Cmdline_env
	if ( !(Is_Extension_Enabled(GL_EXTENSION_ARB_TEXTURE_CUBE_MAP) && Is_Extension_Enabled(GL_EXTENSION_ARB_TEXTURE_ENV_COMBINE)) ) {
		Cmdline_env = 0;
	}
	
	if ( !(Is_Extension_Enabled(GL_EXTENSION_ARB_GEOMETRY_SHADER4) && Is_Extension_Enabled(GL_EXTENSION_EXT_TEXTURE_ARRAY) && Is_Extension_Enabled(GL_EXTENSION_ARB_DRAW_ELEMENTS_BASE_VERTEX)) ) {
		Cmdline_shadow_quality = 0;
		mprintf(("  No hardware support for shadow mapping. Shadows will be disabled. \n"));
	}

	if ( !Cmdline_noglsl && Is_Extension_Enabled(GL_EXTENSION_ARB_SHADER_OBJECTS) && Is_Extension_Enabled(GL_EXTENSION_ARB_FRAGMENT_SHADER)
			&& Is_Extension_Enabled(GL_EXTENSION_ARB_VERTEX_SHADER) ) {
		int ver = 0, major = 0, minor = 0;
		const char *glsl_ver = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

		sscanf(glsl_ver, "%d.%d", &major, &minor);
		ver = (major * 100) + minor;

		GLSL_version = ver;

		// we require a minimum GLSL version
		if (!is_minimum_GLSL_version()) {
			mprintf(("  OpenGL Shading Language version %s is not sufficient to use GLSL mode in FSO. Defaulting to fixed-function renderer.\n", glGetString(GL_SHADING_LANGUAGE_VERSION) ));
		}
	}

	// can't have this stuff without GLSL support
	if ( !is_minimum_GLSL_version() ) {
		Cmdline_normal = 0;
		Cmdline_height = 0;
		Cmdline_postprocess = 0;
		Cmdline_shadow_quality = 0;
		Cmdline_no_deferred_lighting = 1;
	}

	if ( GLSL_version < 120 || !Is_Extension_Enabled(GL_EXTENSION_EXT_FRAMEBUFFER_OBJECT) || !Is_Extension_Enabled(GL_EXTENSION_ARB_FLOATING_POINT_TEXTURES) ) {
        mprintf(("  No hardware support for deferred lighting. Deferred lighting will be disabled. \n"));
		Cmdline_no_deferred_lighting = 1;
		Cmdline_no_batching = true;
	}

	if (is_minimum_GLSL_version()) {
		GLint max_texture_units;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_texture_units);

		// we need enough texture slots for this stuff to work
		
		if (max_texture_units < 6) {
			mprintf(( "Not enough texture units for height map support. We need at least 6, we found %d.\n", max_texture_units ));
			Cmdline_height = 0;
		} else if (max_texture_units < 5) {
			mprintf(( "Not enough texture units for height and normal map support. We need at least 5, we found %d.\n", max_texture_units ));
			Cmdline_normal = 0;
			Cmdline_height = 0;
		} else if (max_texture_units < 4) {
			mprintf(( "Not enough texture units found for GLSL support. We need at least 4, we found %d.\n", max_texture_units ));
			GLSL_version = 0;
		}
	}
}
