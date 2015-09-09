/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
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

void render_colored_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, depth_testing);
	material_info.set_color(255, 255, 255, 255);

	//gr_render_primitives(&material_info, PRIM_TYPE_TRISTRIP, &layout, 0, n_verts, -1);
	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, color *clr, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, depth_testing);

	material_info.set_color(*clr);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, int alpha, bool blending, bool depth_testing)
{
	color clr;

	gr_init_alphacolor(&clr, 255, 255, 255, alpha);

	render_primitives(verts, n_verts, prim_type, texture, &clr, blending, depth_testing);
}

void render_colored_primitives_2d(vertex* verts, int n_verts, primitive_type prim_type, int texture, bool blending)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, false);

	material_info.set_color(255, 255, 255, 255);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives_2d(vertex* verts, int n_verts, primitive_type prim_type, int texture, color *clr, bool blending)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, false);

	material_info.set_color(*clr);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives_2d(vertex* verts, int n_verts, primitive_type prim_type, int texture, int alpha, bool blending)
{
	color clr;

	gr_init_alphacolor(&clr, 255, 255, 255, alpha);

	render_primitives_2d(verts, n_verts, prim_type, texture, &clr, blending);
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

	render_primitives(v, 4, PRIM_TYPE_TRISTRIP, texture, 255, true, true);
}

void render_oriented_quad(vec3d *pos, vec3d *norm, float width, float height, int texture)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	render_oriented_quad(pos, &m, width, height, texture);
}

void render_bitmap_list(bitmap_rect_list* list, int n_bm, int texture, float alpha, bool blending, int resize_mode)
{
	// adapted from g3_draw_2d_poly_bitmap_list

	for ( int i = 0; i < n_bm; i++ ) {
		bitmap_2d_list *l = &list[i].screen_rect;

		// if no valid hight or width values were given get some from the bitmap
		if ( (l->w <= 0) || (l->h <= 0) ) {
			bm_get_info(texture, &l->w, &l->h, NULL, NULL, NULL);
		}

		if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
			gr_resize_screen_pos(&l->x, &l->y, &l->w, &l->h, resize_mode);
		}
	}

	vertex* vert_list = new vertex[6 * n_bm];

	for ( int i = 0; i < n_bm; i++ ) {
		// stuff coords	

		bitmap_2d_list* b = &list[i].screen_rect;
		texture_rect_list* t = &list[i].texture_rect;
		//tri one
		vertex *V = &vert_list[i*6];
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;
	
		//tri two
		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = 0.0f;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;	
	}

	render_primitives_2d(vert_list, 6 * n_bm, PRIM_TYPE_TRIS, texture, 255, true);

	delete[] vert_list;
}