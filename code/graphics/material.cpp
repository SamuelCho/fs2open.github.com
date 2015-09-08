#include "graphics/material.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"

material::material()
{
}

int material::get_shader_handle()
{
	return shader_handle;
}

void material::set_color(int r, int g, int b, int a)
{
	gr_init_alphacolor(&clr, r, g, b, a);
}

void material::set_color(color &clr_in)
{
	if ( clr_in.is_alphacolor ) {
		clr = clr_in;
	} else {
		gr_init_alphacolor(&clr, clr_in.red, clr_in.green, clr_in.blue, 255);
	}
}

int material::get_texture_type()
{
	switch ( tex_type ) {
	default:
	case NORMAL:
		return TCACHE_TYPE_NORMAL;
	case INTERFACE:
		return TCACHE_TYPE_INTERFACE;
	case AABITMAP:
		return TCACHE_TYPE_AABITMAP;
	}
}

int model_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	Shader_flags = 0;

	if ( is_clipped() ) {
		Shader_flags |= SDR_FLAG_MODEL_CLIP;
	}

	if ( is_batched() ) {
		Shader_flags |= SDR_FLAG_MODEL_TRANSFORM;
	}

	if ( Shadow_casting ) {
		// if we're building the shadow map, we likely only need the flags here and above so bail
		Shader_flags |= SDR_FLAG_MODEL_SHADOW_MAP;

		return Shader_flags;
	}

	// setup shader flags for the things that we want/need
	if ( lighting ) {
		Shader_flags |= SDR_FLAG_MODEL_LIGHT;
	}

	if ( is_fogged() ) {
		Shader_flags |= SDR_FLAG_MODEL_FOG;
	}

	if ( animated_effect > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_ANIMATED;
	}

	if ( get_texture_map(TM_BASE_TYPE) ) {
		Shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
	}

	if ( get_texture_map(TM_GLOW_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
	}

	if ( lighting ) {
		if ( ( get_texture_map(TM_SPECULAR_TYPE) > 0) && !Specmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_SPEC_MAP;

			if ( (ENVMAP > 0) && !Envmap_override ) {
				Shader_flags |= SDR_FLAG_MODEL_ENV_MAP;
			}
		}

		if ( (get_texture_map(TM_NORMAL_TYPE) > 0) && !Normalmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_NORMAL_MAP;
		}

		if ( (get_texture_map(TM_HEIGHT_TYPE) > 0) && !Heightmap_override ) {
			Shader_flags |= SDR_FLAG_MODEL_HEIGHT_MAP;
		}

		if ( Cmdline_shadow_quality && !Shadow_casting && !Shadow_override ) {
			Shader_flags |= SDR_FLAG_MODEL_SHADOWS;
		}
	}

	if ( get_texture_map(TM_MISC_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_MISC_MAP;

		if ( team_color_set ) {
			Shader_flags |= SDR_FLAG_MODEL_TEAMCOLOR;
		}
	}

	if ( Deferred ) {
		Shader_flags |= SDR_FLAG_MODEL_DEFERRED;
	}

	if ( thrust_scale > 0.0f ) {
		Shader_flags |= SDR_FLAG_MODEL_THRUSTER;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_MODEL, Shader_flags));

	return get_shader_handle();
}

int particle_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	uint flags = 0;

	if ( point_sprite ) {
		flags |= SDR_FLAG_PARTICLE_POINT_GEN;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, flags));

	return flags;
}

int distortion_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0));
}