/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/pstypes.h"
#include "render/3d.h"
#include "graphics/material.h"


gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool is_transparent)
{
	if (is_transparent) {
		if (base_bitmap >= 0 && bm_has_alpha_channel(base_bitmap)) {
			return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		}

		return ALPHA_BLEND_ADDITIVE;
	}

	return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
}

gr_zbuffer_type render_determine_depth_mode(bool using_depth_test, bool is_transparent)
{
	if (using_depth_test) {
		if (is_transparent) {
			return ZBUFFER_TYPE_READ;
		}

		return ZBUFFER_TYPE_FULL;
	}

	return ZBUFFER_TYPE_NONE;
}

void render_set_unlit_material(material* mat_info, int texture, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
}

void render_primitives(vertex* verts, int n_verts, int texture, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);

	material material_info;

	material_info.set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	material_info.set_blend_mode(blend_mode);
	material_info.set_depth_mode(depth_mode);
	material_info.set_cull_mode(false);
	material_info.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	material_info.set_color(255, 255, 255, 255);
}

void render_primitives(vertex* verts, int n_verts, int texture, int red, int green, int blue, int alpha, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, depth_testing);

	material_info.set_color(red, green, blue, alpha);

	gr_render_primitives(&material_info, PRIM_TYPE_TRISTRIP, &layout, 0, n_verts, -1);
}

void render_primitives(vertex* verts, int n_verts, int texture, int alpha, bool blending, bool depth_testing)
{
	render_primitives(verts, n_verts, texture, 255, 255, 255, alpha, blending, depth_testing);
}

void render_primitives_2d(vertex* verts, int n_verts, int texture, int red, int green, int blue, int alpha, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	material_info.set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	material_info.set_blend_mode(blend_mode);
	material_info.set_depth_mode(depth_mode);
	material_info.set_cull_mode(false);
	material_info.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	material_info.set_color(red, green, blue, alpha);
}

void render_primitives_2d(vertex* verts, int texture, int alpha, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	material_info.set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	material_info.set_blend_mode(blend_mode);
	material_info.set_depth_mode(depth_mode);
	material_info.set_cull_mode(false);
	material_info.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	material_info.set_color(255, 255, 255, alpha);
}

void render_primitives_2d(vertex* verts, int texture, int red, int green, int blue, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	material_info.set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	material_info.set_blend_mode(blend_mode);
	material_info.set_depth_mode(depth_mode);
	material_info.set_cull_mode(false);
	material_info.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	material_info.set_color(red, green, blue, 255);
}

void render_primitives_2d(vertex* verts, int texture, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);

	material material_info;

	material_info.set_texture_map(texture, TM_BASE_TYPE);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	material_info.set_blend_mode(blend_mode);
	material_info.set_depth_mode(depth_mode);
	material_info.set_cull_mode(false);
	material_info.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	material_info.set_color(255, 255, 255, 255);
}

void render_oriented_quad(vec3d *pos, matrix *ori, float width, float height, int texture)
{
	//idiot-proof
	if(width == 0 || height == 0)
			return;

	Assert(pos != NULL);
	Assert(ori != NULL);
	
	//Let's begin.
	
	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	vertex v[NUM_VERTICES];
    
    memset(v, 0, sizeof(v));

	p[0].xyz.x = width;
	p[0].xyz.y = height;

	p[1].xyz.x = -width;
	p[1].xyz.y = height;

	p[2].xyz.x = -width;
	p[2].xyz.y = -height;

	p[3].xyz.x = width;
	p[3].xyz.y = -height;

	for(int i = 0; i < NUM_VERTICES; i++)
	{
		vec3d tmp = vmd_zero_vector;

		//Rotate correctly
		vm_vec_unrotate(&tmp, &p[i], ori);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		//Convert to vertex
		g3_transfer_vertex(&v[i], &tmp);
	}

	v[0].texture_position.u = 1.0f;
	v[0].texture_position.v = 0.0f;

	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;
	
	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 1.0f;

	v[3].texture_position.u = 1.0f;
	v[3].texture_position.v = 1.0f;

	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &v[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &v[0].texture_position.u);

	render_primitives(v, 4, texture, 255, true, true);
}

void render_oriented_quad(vec3d *pos, vec3d *norm, float width, float height, int texture)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	render_oriented_quad(pos, &m, width, height, texture);
}
