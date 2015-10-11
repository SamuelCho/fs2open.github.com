#include "globalincs/pstypes.h"
#include "graphics/grinternal.h"
#include "graphics/2d.h"
#include "graphics/material.h"
#include "globalincs/systemvars.h"
#include "cmdline/cmdline.h"

material::material(): 
Sdr_handle(-1), 
Sdr_type(SDR_TYPE_NONE),
Tex_source(TEXTURE_SOURCE_NO_FILTERING), 
Tex_type(TEX_TYPE_NORMAL),
Texture_addressing(TMAP_ADDRESS_WRAP),
Depth_bias(0),
Depth_mode(ZBUFFER_TYPE_NONE),
Blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA),
Cull_mode(true),
Fill_mode(GR_FILL_MODE_SOLID)
{
	gr_init_alphacolor(&Clr, 255, 255, 255, 255);
	
	Texture_maps[TM_BASE_TYPE]		= -1;
	Texture_maps[TM_GLOW_TYPE]		= -1;
	Texture_maps[TM_SPECULAR_TYPE]	= -1;
	Texture_maps[TM_NORMAL_TYPE]	= -1;
	Texture_maps[TM_HEIGHT_TYPE]	= -1;
	Texture_maps[TM_MISC_TYPE]		= -1;

	Fog_params.dist_near = -1.0f;
	Fog_params.dist_far = -1.0f;
	Fog_params.r = 0;
	Fog_params.g = 0;
	Fog_params.b = 0;
	Fog_params.enabled = false;

	Clip_params.enabled = false;
};

void material::set_shader_type(shader_type init_sdr_type)
{
	Sdr_type = init_sdr_type;
}

void material::set_shader_handle(int handle)
{
	Sdr_handle = handle;
}

int material::get_shader_handle()
{
	return Sdr_handle;
}

void material::set_texture_map(int texture_type, int texture_num)
{
	Assert(texture_type > -1 && texture_type < TM_NUM_TYPES);

	Texture_maps[texture_type] = texture_num;
}

int material::get_texture_map(int texture_type)
{
	Assert(texture_type > -1 && texture_type < TM_NUM_TYPES);

	return Texture_maps[texture_type];
}

bool material::is_textured()
{
	return 
	Texture_maps[TM_BASE_TYPE]		> -1 ||
	Texture_maps[TM_GLOW_TYPE]		> -1 ||
	Texture_maps[TM_SPECULAR_TYPE]	> -1 ||
	Texture_maps[TM_NORMAL_TYPE]	> -1 ||
	Texture_maps[TM_HEIGHT_TYPE]	> -1 ||
	Texture_maps[TM_MISC_TYPE]		> -1;
}

void material::set_texture_type(texture_type t_type)
{
	Tex_type = t_type;
}

int material::get_texture_type()
{
	switch ( Tex_type ) {
	default:
	case TEX_TYPE_NORMAL:
		return TCACHE_TYPE_NORMAL;
	case TEX_TYPE_INTERFACE:
		return TCACHE_TYPE_INTERFACE;
	case TEX_TYPE_AABITMAP:
		return TCACHE_TYPE_AABITMAP;
	}
}

void material::set_texture_source(gr_texture_source source)
{
	Tex_source = source;
}

gr_texture_source material::get_texture_source()
{
	return Tex_source;
}

bool material::is_clipped()
{
	return Clip_params.enabled;
}

void material::set_clip_plane(const vec3d &normal, const vec3d &position)
{
	Clip_params.enabled = true;
	Clip_params.normal = normal;
	Clip_params.position = position;
}

void material::set_clip_plane()
{
	Clip_params.enabled = false;
}

material::clip_plane& material::get_clip_plane()
{
	return Clip_params;
}

void material::set_texture_addressing(int addressing)
{
	Texture_addressing = addressing;
}

int material::get_texture_addressing()
{
	return Texture_addressing;
}

void material::set_fog(int r, int g, int b, float near, float far)
{
	Fog_params.enabled = true;
	Fog_params.r = r;
	Fog_params.g = g;
	Fog_params.b = b;
	Fog_params.dist_near = near;
	Fog_params.dist_far = far;
}

void material::set_fog()
{
	Fog_params.enabled = false;
}

bool material::is_fogged()
{
	return Fog_params.enabled;
}

material::fog& material::get_fog()
{
	return Fog_params; 
}

void material::set_depth_mode(gr_zbuffer_type mode)
{
	Depth_mode = mode;
}

gr_zbuffer_type material::get_depth_mode()
{
	return Depth_mode;
}

void material::set_cull_mode(bool mode)
{
	Cull_mode = mode;
}

bool material::get_cull_mode()
{
	return Cull_mode;
}

void material::set_fill_mode(int mode)
{
	Fill_mode = mode;
}

int material::get_fill_mode()
{
	return Fill_mode;
}

void material::set_blend_mode(gr_alpha_blend mode)
{
	Blend_mode = mode;
}

gr_alpha_blend material::get_blend_mode()
{
	return Blend_mode;
}

void material::set_depth_bias(int bias)
{
	Depth_bias = bias;
}

int material::get_depth_bias()
{
	return Depth_bias;
}

void material::set_color(float red, float green, float blue, float alpha)
{
	set_color(fl2i(red * 255.0f), fl2i(green * 255.0f), fl2i(blue * 255.0f), fl2i(alpha * 255.0f));
}

void material::set_color(int r, int g, int b, int a)
{
	gr_init_alphacolor(&Clr, r, g, b, a);
}

void material::set_color(color &clr_in)
{
	if ( clr_in.is_alphacolor ) {
		Clr = clr_in;
	} else {
		gr_init_alphacolor(&Clr, clr_in.red, clr_in.green, clr_in.blue, 255);
	}
}

color& material::get_color()
{
	return Clr;
}

model_material::model_material(): 
material(), 
Animated_effect(0), 
Animated_timer(0.0f), 
Thrust_scale(-1.0f), 
lighting(false), 
Light_factor(1.0f), 
Batched(false), 
Team_color_set(false), 
Center_alpha(0),
Desaturate(false)
{ 
	set_shader_type(SDR_TYPE_MODEL); 
}

void model_material::set_desaturation(bool enabled)
{
	Desaturate = enabled;
}

bool model_material::is_desaturated()
{
	return Desaturate;
}

void model_material::set_shadow_casting(bool enabled)
{
	Shadow_casting = enabled;
}

void model_material::set_light_factor(float factor)
{
	Light_factor = factor;
}

float model_material::get_light_factor()
{
	return Light_factor;
}

void model_material::set_lighting(bool mode)
{
	lighting = mode;
} 

bool model_material::is_lit()
{
	return lighting;
}

void model_material::set_deferred_lighting(bool enabled)
{
	Deferred = enabled;
}

void model_material::set_center_alpha(int c_alpha)
{
	Center_alpha = c_alpha;
}

int model_material::get_center_alpha()
{
	return Center_alpha;
}

void model_material::set_thrust_scale(float scale)
{
	Thrust_scale = scale;
}

float model_material::get_thrust_scale()
{
	return Thrust_scale;
}

void model_material::set_team_color(const team_color &color)
{
	Team_color_set = true;
	Tm_color = color;
}

void model_material::set_team_color()
{
	Team_color_set = false;
}

team_color& model_material::get_team_color()
{
	return Tm_color;
}

void model_material::set_animated_effect(int effect, float time)
{
	Animated_effect = effect;
	Animated_timer = time;
}

void model_material::set_animated_effect()
{
	Animated_effect = 0;
	Animated_timer = 0.0f;
}

int model_material::get_animated_effect()
{
	return Animated_effect;
}

float model_material::get_animated_effect_time()
{
	return Animated_timer;
}

void model_material::set_batching(bool enabled)
{
	Batched = enabled;
}

bool model_material::is_batched()
{
	return Batched;
}

uint model_material::get_shader_flags()
{
	uint Shader_flags = 0;

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
	
	if ( is_fogged() ) {
		Shader_flags |= SDR_FLAG_MODEL_FOG;
	}

	if ( Animated_effect > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_ANIMATED;
	}

	if ( get_texture_map(TM_BASE_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_DIFFUSE_MAP;
	}

	if ( get_texture_map(TM_GLOW_TYPE) > 0 ) {
		Shader_flags |= SDR_FLAG_MODEL_GLOW_MAP;
	}

	if ( lighting ) {
		Shader_flags |= SDR_FLAG_MODEL_LIGHT;

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

		if ( Team_color_set ) {
			Shader_flags |= SDR_FLAG_MODEL_TEAMCOLOR;
		}
	}

	if ( Deferred ) {
		Shader_flags |= SDR_FLAG_MODEL_DEFERRED;
	}

	if ( Thrust_scale > 0.0f ) {
		Shader_flags |= SDR_FLAG_MODEL_THRUSTER;
	}

	return Shader_flags;
}

int model_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_MODEL, get_shader_flags()));

	return get_shader_handle();
}

particle_material::particle_material(): 
material()  
{
	set_shader_type(SDR_TYPE_EFFECT_PARTICLE);
}

void particle_material::set_point_sprite_mode(bool enabled)
{
	Point_sprite = enabled;
}

bool particle_material::get_point_sprite_mode()
{
	return Point_sprite;
}

int particle_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	uint flags = 0;

	if ( Point_sprite ) {
		flags |= SDR_FLAG_PARTICLE_POINT_GEN;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_PARTICLE, flags));

	return flags;
}

distortion_material::distortion_material(): 
material()
{
	set_shader_type(SDR_TYPE_EFFECT_DISTORTION);
}

void distortion_material::set_thruster_rendering(bool enabled)
{
	Thruster_render = enabled;
}

bool distortion_material::get_thruster_rendering()
{
	return Thruster_render;
}

int distortion_material::get_shader_handle()
{
	int handle = material::get_shader_handle();

	if ( handle >= 0 ) {
		return handle;
	}

	set_shader_handle(gr_maybe_create_shader(SDR_TYPE_EFFECT_DISTORTION, 0));
}