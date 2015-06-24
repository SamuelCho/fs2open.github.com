#include "graphics/material.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"

material::material()
{
}

uint model_material::determine_shader()
{
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

	if ( textured ) {
		if (!Basemap_override) {
			Shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
		}

		if (  get_texture_map(TM_GLOW_TYPE) > 0) {
			Shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
		}

		if ( lighting ) {
			if (( get_texture_map(TM_SPECULAR_TYPE) > 0) && !Specmap_override) {
				Shader_flags |= SDR_FLAG_MODEL_SPEC_MAP;

				if ((ENVMAP > 0) && !Envmap_override) {
					Shader_flags |= SDR_FLAG_MODEL_ENV_MAP;
				}
			}

			if ((get_texture_map(TM_NORMAL_TYPE) > 0) && !Normalmap_override) {
				Shader_flags |= SDR_FLAG_MODEL_NORMAL_MAP;
			}

			if ((get_texture_map(TM_HEIGHT_TYPE) > 0) && !Heightmap_override) {
				Shader_flags |= SDR_FLAG_MODEL_HEIGHT_MAP;
			}

			if (Cmdline_shadow_quality && !Shadow_casting && !Shadow_override) {
				Shader_flags |= SDR_FLAG_MODEL_SHADOWS;
			}
		}

		if (get_texture_map(TM_MISC_TYPE) > 0) {
			Shader_flags |= SDR_FLAG_MODEL_MISC_MAP;
		}

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

	return Shader_flags;
}