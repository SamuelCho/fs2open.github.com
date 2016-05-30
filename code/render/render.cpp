/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "freespace2/freespace.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/line.h"
#include "math/floating.h"
#include "osapi/osapi.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "render/3dinternal.h"
#include "graphics/material.h"
#include "render/render.h"

gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool blending)
{
	if ( blending ) {
		if ( base_bitmap >= 0 && bm_has_alpha_channel(base_bitmap) ) {
			return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		}

		return ALPHA_BLEND_ADDITIVE;
	}

	return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
}

gr_zbuffer_type render_determine_depth_mode(bool depth_testing, bool blending)
{
	if ( depth_testing ) {
		if ( blending ) {
			return ZBUFFER_TYPE_READ;
		}

		return ZBUFFER_TYPE_FULL;
	}

	return ZBUFFER_TYPE_NONE;
}

void render_set_unlit_material(material* mat_info, int texture, float alpha, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		mat_info->set_color(alpha, alpha, alpha, 1.0f);
	} else {
		mat_info->set_color(1.0f, 1.0f, 1.0f, alpha);
	}

	if ( texture >= 0 && bm_has_alpha_channel(texture) ) {
		mat_info->set_texture_type(material::TEX_TYPE_XPARENT);
	}
}

void render_set_unlit_color_material(material* mat_info, int texture, color *clr, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat_info->set_color(*clr);
}

void render_set_interface_material(material* mat_info, int texture, bool blended, float alpha)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_texture_type(material::TEX_TYPE_INTERFACE);

	mat_info->set_blend_mode(render_determine_blend_mode(texture, blended));
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);

	mat_info->set_color(1.0f, 1.0f, 1.0f, blended ? alpha : 1.0f);
}

void render_set_volume_emissive_material(particle_material* mat_info, int texture, bool point_sprites)
{
	mat_info->set_point_sprite_mode(point_sprites);
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);

	mat_info->set_blend_mode(render_determine_blend_mode(texture, true));

	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

void render_set_distortion_material(distortion_material *mat_info, int texture, bool thruster)
{
	mat_info->set_thruster_rendering(thruster);

	mat_info->set_depth_mode(ZBUFFER_TYPE_READ);

	mat_info->set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

void render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), (int)offsetof(vertex, texture_position));
	
	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;
	uint mask = 0;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), (int)offsetof(vertex, r));

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic)
{
	vertex_layout layout;

	if ( orthographic ) {
		layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	} else {
		layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), (int)offsetof(vertex, world));
	}

	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), (int)offsetof(vertex, texture_position));
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), (int)offsetof(vertex, r));

	if ( orthographic ) {
		gr_render_primitives_2d_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	} else {
		gr_render_primitives_immediate(mat, prim_type, &layout, n_verts, verts, n_verts * sizeof(vertex));
	}
}

void render_screen_points_textured(
	material *material_info,
	float x1, float y1, float u1, float v1,
	float x2, float y2, float u2, float v2 )
{
	float glVertices[4][4] = {
		{ x1, y1, u1, v1 },
		{ x1, y2, u1, v2 },
		{ x2, y1, u2, v1 },
		{ x2, y2, u2, v2 }
	};

	vertex vertices[4];

	vertices[0].screen.xyw.x = x1;
	vertices[0].screen.xyw.y = y1;
	vertices[0].texture_position.u = u1;
	vertices[0].texture_position.v = v1;

	vertices[1].screen.xyw.x = x1;
	vertices[1].screen.xyw.y = y2;
	vertices[1].texture_position.u = u1;
	vertices[1].texture_position.v = v2;

	vertices[2].screen.xyw.x = x2;
	vertices[2].screen.xyw.y = y1;
	vertices[2].texture_position.u = u2;
	vertices[2].texture_position.v = v1;

	vertices[3].screen.xyw.x = x2;
	vertices[3].screen.xyw.y = y2;
	vertices[3].texture_position.u = u2;
	vertices[3].texture_position.v = v2;

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), (int)offsetof(vertex, screen));
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), (int)offsetof(vertex, texture_position));
	
	gr_render_primitives_2d_immediate(material_info, PRIM_TYPE_TRISTRIP, &vert_def, 4, vertices, sizeof(vertex) * 4);
}

void render_screen_points(
	material *material_info,
	int x1, int y1, 
	int x2, int y2 )
{
	int glVertices[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::SCREEN_POS, 0, 0);
	
	gr_render_primitives_2d_immediate(material_info, PRIM_TYPE_TRISTRIP, &vert_def, 4, glVertices, sizeof(int) * 8);
}

void render_screen_points(
	material *material_info,
	float x1, float y1,
	float x2, float y2 )
{
	float glVertices[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, 0);
	
	gr_render_primitives_2d_immediate(material_info, PRIM_TYPE_TRISTRIP, &vert_def, 4, glVertices, sizeof(float) * 8);
}

// adapted from g3_draw_polygon()
void render_oriented_quad_internal(int texture, color *clr, float alpha, bool blending, vec3d *pos, matrix *ori, float width, float height)
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

	material material_params;

	if ( clr == NULL ) {
		render_set_unlit_material(&material_params, texture, alpha, blending, true);
	} else {
		color alpha_clr;
		gr_init_alphacolor(&alpha_clr, clr->red, clr->green, clr->blue, fl2i(alpha * 255.0f));
		render_set_unlit_color_material(&material_params, texture, clr, blending, true);
	}
	
	render_primitives_textured(&material_params, v, 4, PRIM_TYPE_TRIFAN, false);
}

void render_oriented_quad(int texture, vec3d *pos, matrix *ori, float width, float height)
{
	render_oriented_quad_internal(texture, NULL, 1.0f, false, pos, ori, width, height);
}

void render_oriented_quad(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height)
{
	render_oriented_quad_internal(texture, NULL, alpha, true, pos, ori, width, height);
}

void render_oriented_quad(int texture, float alpha, vec3d *pos, vec3d *norm, float width, float height)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	render_oriented_quad_internal(texture, NULL, alpha, true, pos, &m, width, height);
}

void render_oriented_quad_colored(int texture, color *clr, float alpha, vec3d *pos, matrix *ori, float width, float height)
{
	render_oriented_quad_internal(texture, clr, alpha, true, pos, ori, width, height);
}

void render_oriented_quad_colored(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height)
{
	render_oriented_quad_internal(texture, &gr_screen.current_color, alpha, true, pos, ori, width, height);
}

// adapted from gr_bitmap_list()
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
	float sw = 0.1f;

	for ( int i = 0; i < n_bm; i++ ) {
		// stuff coords	

		bitmap_2d_list* b = &list[i].screen_rect;
		texture_rect_list* t = &list[i].texture_rect;
		//tri one
		vertex *V = &vert_list[i*6];
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;
	
		//tri two
		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)b->y;	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v0;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)(b->x + b->w);
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u1;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;

		V++;
		V->screen.xyw.x = (float)b->x;
		V->screen.xyw.y = (float)(b->y + b->h);	
		V->screen.xyw.w = sw;
		V->texture_position.u = (float)t->u0;
		V->texture_position.v = (float)t->v1;
		V->flags = PF_PROJECTED;
		V->codes = 0;	
	}

	material material_params;

	render_set_unlit_material(&material_params, texture, 1.0f, true, false);
	render_primitives_textured(&material_params, vert_list, 6 * n_bm, PRIM_TYPE_TRIS, true);

	delete[] vert_list;
}

// adapted g3_draw_rotated_bitmap_3d()
void render_rotated_bitmap(int texture, float alpha, vertex *pnt, float angle, float rad)
{
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	angle -= Physics_viewer_bank;
	if ( angle < 0.0f ) {
		angle += PI2;
	} else if ( angle > PI2 ) {
		angle -= PI2;
	}

	vec3d PNT(pnt->world);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);
	vm_vec_normalize(&uvec);

	vm_vec_cross(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize(&rvec);

	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	//set up the UV coords
	P[0].texture_position.u = 0.0f;	P[0].texture_position.v = 0.0f;
	P[1].texture_position.u = 1.0f;	P[1].texture_position.v = 0.0f;
	P[2].texture_position.u = 1.0f;	P[2].texture_position.v = 1.0f;
	P[3].texture_position.u = 0.0f;	P[3].texture_position.v = 1.0f;

	material material_params;

	render_set_unlit_material(&material_params, texture, alpha, true, true);
	render_primitives_textured(&material_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from gr_opengl_scaler()
void render_bitmap_scaler(int texture, float alpha, bool blending, vertex *va, vertex *vb)
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	x0 = va->screen.xyw.x;
	y0 = va->screen.xyw.y;
	x1 = vb->screen.xyw.x;
	y1 = vb->screen.xyw.y;

	xmin = i2fl(gr_screen.clip_left);
	ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right);
	ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->texture_position.u; v0 = va->texture_position.v;
	u1 = vb->texture_position.u; v1 = vb->texture_position.v;

	// Check for obviously offscreen bitmaps...
	if ( (y1 <= y0) || (x1 <= x0) ) {
		return;
	}

	if ( (x1 < xmin ) || (x0 > xmax) ) {
		return;
	}

	if ( (y1 < ymin ) || (y0 > ymax) ) {
		return;
	}

	clipped_u0 = u0;
	clipped_v0 = v0;
	clipped_u1 = u1;
	clipped_v1 = v1;

	clipped_x0 = x0;
	clipped_y0 = y0;
	clipped_x1 = x1;
	clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if (x0 < xmin) {
		clipped_u0 = FIND_SCALED_NUM(xmin, x0, x1, u0, u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if (x1 > xmax) {
		clipped_u1 = FIND_SCALED_NUM(xmax, x0, x1, u0, u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if (y0 < ymin) {
		clipped_v0 = FIND_SCALED_NUM(ymin, y0, y1, v0, v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if (y1 > ymax) {
		clipped_v1 = FIND_SCALED_NUM(ymax, y0, y1, v0, v1);
		clipped_y1 = ymax;
	}

	dx0 = fl2i(clipped_x0);
	dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0);
	dy1 = fl2i(clipped_y1);

	if ( (dx1 <= dx0) || (dy1 <= dy0) ) {
		return;
	}

	vertex v[4];

	v[0].screen.xyw.x = clipped_x0;
	v[0].screen.xyw.y = clipped_y0;
	v[0].screen.xyw.w = va->screen.xyw.w;
	v[0].world.xyz.z = va->world.xyz.z;
	v[0].texture_position.u = clipped_u0;
	v[0].texture_position.v = clipped_v0;
	v[0].spec_r = 0;
	v[0].spec_g = 0;
	v[0].spec_b = 0;

	v[1].screen.xyw.x = clipped_x1;
	v[1].screen.xyw.y = clipped_y0;
	v[1].screen.xyw.w = va->screen.xyw.w;
	v[1].world.xyz.z = va->world.xyz.z;
	v[1].texture_position.u = clipped_u1;
	v[1].texture_position.v = clipped_v0;
	v[1].spec_r = 0;
	v[1].spec_g = 0;
	v[1].spec_b = 0;

	v[2].screen.xyw.x = clipped_x1;
	v[2].screen.xyw.y = clipped_y1;
	v[2].screen.xyw.w = va->screen.xyw.w;
	v[2].world.xyz.z = va->world.xyz.z;
	v[2].texture_position.u = clipped_u1;
	v[2].texture_position.v = clipped_v1;
	v[2].spec_r = 0;
	v[2].spec_g = 0;
	v[2].spec_b = 0;

	v[3].screen.xyw.x = clipped_x0;
	v[3].screen.xyw.y = clipped_y1;
	v[3].screen.xyw.w = va->screen.xyw.w;
	v[3].world.xyz.z = va->world.xyz.z;
	v[3].texture_position.u = clipped_u0;
	v[3].texture_position.v = clipped_v1;
	v[3].spec_r = 0;
	v[3].spec_g = 0;
	v[3].spec_b = 0;

	material material_params;

	render_set_unlit_material(&material_params, texture, alpha, blending, false);
	render_primitives_textured(&material_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

// adapted from g3_draw_bitmap()
void render_oriented_bitmap_2d(int texture, float alpha, bool blending, vertex *pnt, int orient, float rad)
{
	vertex va, vb;
	float t,w,h;
	float width, height;

	int bw, bh;

	bm_get_info( texture, &bw, &bh, NULL );

	if ( bw < bh )	{
		width = rad*2.0f;
		height = width*i2fl(bh)/i2fl(bw);
	} else if ( bw > bh )	{
		height = rad*2.0f;
		width = height*i2fl(bw)/i2fl(bh);
	} else {
		width = height = rad*2.0f;
	}

	Assert( G3_count == 1 );

	if ( pnt->codes & (CC_BEHIND|CC_OFF_USER) ) 
		return;

	if (!(pnt->flags&PF_PROJECTED))
		g3_project_vertex(pnt);

	if (pnt->flags & PF_OVERFLOW)
		return;

	t = (width * gr_screen.clip_width * 0.5f)/pnt->world.xyz.z;
	w = t*Matrix_scale.xyz.x;

	t = (height * gr_screen.clip_height * 0.5f)/pnt->world.xyz.z;
	h = t*Matrix_scale.xyz.y;

	float z,sw;
	z = pnt->world.xyz.z - rad/2.0f;
	if ( z <= 0.0f ) {
		z = 0.0f;
		sw = 0.0f;
	} else {
		sw = 1.0f / z;
	}

	va.screen.xyw.x = pnt->screen.xyw.x - w/2.0f;
	va.screen.xyw.y = pnt->screen.xyw.y - h/2.0f;
	va.screen.xyw.w = sw;
	va.world.xyz.z = z;

	vb.screen.xyw.x = va.screen.xyw.x + w;
	vb.screen.xyw.y = va.screen.xyw.y + h;
	vb.screen.xyw.w = sw;
	vb.world.xyz.z = z;

	if ( orient & 1 )	{
		va.texture_position.u = 1.0f;
		vb.texture_position.u = 0.0f;
	} else {
		va.texture_position.u = 0.0f;
		vb.texture_position.u = 1.0f;
	}

	if ( orient & 2 )	{
		va.texture_position.v = 1.0f;
		vb.texture_position.v = 0.0f;
	} else {
		va.texture_position.v = 0.0f;
		vb.texture_position.v = 1.0f;
	}

	render_bitmap_scaler(texture, alpha, blending, &va, &vb);
}

void render_oriented_bitmap_2d(int texture, vertex *pnt, int orient, float rad)
{
	render_oriented_bitmap_2d(texture, 1.0f, false, pnt, orient, rad);
}

void render_oriented_bitmap_2d(int texture, float alpha, vertex *pnt, int orient, float rad)
{
	render_oriented_bitmap_2d(texture, alpha, true, pnt, orient, rad);
}

// adapted from g3_draw_bitmap_3d
void render_oriented_bitmap(int texture, float alpha, vertex *pnt, int orient, float rad, float depth)
{
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and hieght rad

	vec3d PNT(pnt->world);
	vec3d p[4];
	vertex P[4];
	vec3d fvec, rvec, uvec;

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize(&fvec);

	uvec = View_matrix.vec.uvec;
	vm_vec_normalize(&uvec);
	rvec = View_matrix.vec.rvec;
	vm_vec_normalize(&rvec);

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);

	//move all the data from the vecs into the verts
	g3_transfer_vertex(&P[0], &p[3]);
	g3_transfer_vertex(&P[1], &p[2]);
	g3_transfer_vertex(&P[2], &p[1]);
	g3_transfer_vertex(&P[3], &p[0]);

	// set up the UV coords
	if ( orient & 1 ) {
		P[0].texture_position.u = 1.0f;
		P[1].texture_position.u = 0.0f;
		P[2].texture_position.u = 0.0f;
		P[3].texture_position.u = 1.0f;
	} else {
		P[0].texture_position.u = 0.0f;
		P[1].texture_position.u = 1.0f;
		P[2].texture_position.u = 1.0f;
		P[3].texture_position.u = 0.0f;
	}

	if ( orient & 2 ) {
		P[0].texture_position.v = 1.0f;
		P[1].texture_position.v = 1.0f;
		P[2].texture_position.v = 0.0f;
		P[3].texture_position.v = 0.0f;
	} else {
		P[0].texture_position.v = 0.0f;
		P[1].texture_position.v = 0.0f;
		P[2].texture_position.v = 1.0f;
		P[3].texture_position.v = 1.0f;
	}

	material material_params;

	render_set_unlit_material(&material_params, texture, alpha, true, true);
	render_primitives_textured(&material_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from g3_draw_laser_htl()
void render_laser(int texture, color *clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width)
{
	head_width *= 0.5f;
	tail_width *= 0.5f;
	vec3d uvec, fvec, rvec, center, reye, rfvec;

	vm_vec_sub( &fvec, headp, tailp );
	vm_vec_normalize_safe( &fvec );
	vm_vec_copy_scale(&rfvec, &fvec, -1.0f);

	vm_vec_avg( &center, headp, tailp ); //needed for the return value only
	vm_vec_sub(&reye, &Eye_position, &center);
	vm_vec_normalize(&reye);

	// code intended to prevent possible null vector normalize issue - start
	if ( vm_test_parallel(&reye, &fvec) ) {
		fvec.xyz.x = -reye.xyz.z;
		fvec.xyz.y = 0.0f;
		fvec.xyz.z = -reye.xyz.x;
	}

	if ( vm_test_parallel(&reye, &rfvec) ) {
		fvec.xyz.x = reye.xyz.z;
		fvec.xyz.y = 0.0f;
		fvec.xyz.z = reye.xyz.x;
	}
	// code intended to prevent possible null vector normalize issue - end

	vm_vec_cross(&uvec, &fvec, &reye);
	vm_vec_normalize(&uvec);
	vm_vec_cross(&fvec, &uvec, &reye);
	vm_vec_normalize(&fvec);
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_cross(&rvec, &uvec, &fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;
	vec3d start, end;
	vm_vec_scale_add(&start, headp, &fvec, -head_width);
	vm_vec_scale_add(&end, tailp, &fvec, tail_width);
	vec3d vecs[4];
	vertex pts[4];

	vm_vec_scale_add( &vecs[0], &start, &uvec, head_width );
	vm_vec_scale_add( &vecs[1], &end, &uvec, tail_width );
	vm_vec_scale_add( &vecs[2], &end, &uvec, -tail_width );
	vm_vec_scale_add( &vecs[3], &start, &uvec, -head_width );

	for ( i = 0; i < 4; i++ ) {
		g3_transfer_vertex( &pts[i], &vecs[i] );
	}

	pts[0].texture_position.u = 0.0f;
	pts[0].texture_position.v = 0.0f;
	pts[1].texture_position.u = 1.0f;
	pts[1].texture_position.v = 0.0f;
	pts[2].texture_position.u = 1.0f;
	pts[2].texture_position.v = 1.0f;
	pts[3].texture_position.u = 0.0f;
	pts[3].texture_position.v = 1.0f;

	pts[0].r = (ubyte)clr->red;
	pts[0].g = (ubyte)clr->green;
	pts[0].b = (ubyte)clr->blue;
	pts[0].a = 255;
	pts[1].r = (ubyte)clr->red;
	pts[1].g = (ubyte)clr->green;
	pts[1].b = (ubyte)clr->blue;
	pts[1].a = 255;
	pts[2].r = (ubyte)clr->red;
	pts[2].g = (ubyte)clr->green;
	pts[2].b = (ubyte)clr->blue;
	pts[2].a = 255;
	pts[3].r = (ubyte)clr->red;
	pts[3].g = (ubyte)clr->green;
	pts[3].b = (ubyte)clr->blue;
	pts[3].a = 255;

	material material_params;

	render_set_unlit_material(&material_params, texture, alpha, true, true);
	render_primitives_colored_textured(&material_params, pts, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from g3_draw_laser()
void render_laser_2d(int texture, color* clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
{
	float headx, heady, headr, tailx, taily, tailr;
	vertex pt1, pt2;
	float depth;

	Assert( G3_count == 1 );

	g3_rotate_vertex(&pt1,headp);

	g3_project_vertex(&pt1);
	if (pt1.flags & PF_OVERFLOW) 
		return;

	g3_rotate_vertex(&pt2,tailp);

	g3_project_vertex(&pt2);
	if (pt2.flags & PF_OVERFLOW) 
		return;

	if ( (pt1.codes & pt2.codes) != 0 )	{
		// Both off the same side
		return;
	}

	headx = pt1.screen.xyw.x;
	heady = pt1.screen.xyw.y;
	headr = (head_width*Matrix_scale.xyz.x*Canv_w2*pt1.screen.xyw.w);

	tailx = pt2.screen.xyw.x;
	taily = pt2.screen.xyw.y;
	tailr = (tail_width*Matrix_scale.xyz.x*Canv_w2*pt2.screen.xyw.w);

	float len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );

	// Cap the length if needed.
	if ( (max_len > 1.0f) && (len_2d > max_len) )	{
		float ratio = max_len / len_2d;
	
		tailx = headx + ( tailx - headx ) * ratio;
		taily = heady + ( taily - heady ) * ratio;
		tailr = headr + ( tailr - headr ) * ratio;

		len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );
	}

	depth = (pt1.world.xyz.z+pt2.world.xyz.z)*0.5f;

	float max_r  = headr;
	float a;
	if ( tailr > max_r ) 
		max_r = tailr;

	if ( max_r < 1.0f )
		max_r = 1.0f;

	float mx, my, w, h1,h2;

	if ( len_2d < max_r ) {

		h1 = headr + (max_r-len_2d);
		if ( h1 > max_r ) h1 = max_r;
		h2 = tailr + (max_r-len_2d);
		if ( h2 > max_r ) h2 = max_r;

		len_2d = max_r;
		if ( fl_abs(tailx - headx) > 0.01f )	{
			a = (float)atan2( taily-heady, tailx-headx );
		} else {
			a = 0.0f;
		}

		w = len_2d;

	} else {
		a = atan2_safe( taily-heady, tailx-headx );

		w = len_2d;

		h1 = headr;
		h2 = tailr;
	}
	
	mx = (tailx+headx)/2.0f;
	my = (taily+heady)/2.0f;

	// Draw box with width 'w' and height 'h' at angle 'a' from horizontal
	// centered around mx, my

	if ( h1 < 1.0f ) h1 = 1.0f;
	if ( h2 < 1.0f ) h2 = 1.0f;

	float sa, ca;

	sa = (float)sin(a);
	ca = (float)cos(a);

	vertex v[4];
	vertex *vertlist[4] = { &v[3], &v[2], &v[1], &v[0] };
	memset(v,0,sizeof(vertex)*4);

	if ( depth < 0.0f ) depth = 0.0f;
	
	v[0].screen.xyw.x = (-w/2.0f)*ca + (-h1/2.0f)*sa + mx;
	v[0].screen.xyw.y = (-w/2.0f)*sa - (-h1/2.0f)*ca + my;
	v[0].world.xyz.z = pt1.world.xyz.z;
	v[0].screen.xyw.w = pt1.screen.xyw.w;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].b = 191;

	v[1].screen.xyw.x = (w/2.0f)*ca + (-h2/2.0f)*sa + mx;
	v[1].screen.xyw.y = (w/2.0f)*sa - (-h2/2.0f)*ca + my;
	v[1].world.xyz.z = pt2.world.xyz.z;
	v[1].screen.xyw.w = pt2.screen.xyw.w;
	v[1].texture_position.u = 1.0f;
	v[1].texture_position.v = 0.0f;
	v[1].b = 191;

	v[2].screen.xyw.x = (w/2.0f)*ca + (h2/2.0f)*sa + mx;
	v[2].screen.xyw.y = (w/2.0f)*sa - (h2/2.0f)*ca + my;
	v[2].world.xyz.z = pt2.world.xyz.z;
	v[2].screen.xyw.w = pt2.screen.xyw.w;
	v[2].texture_position.u = 1.0f;
	v[2].texture_position.v = 1.0f;
	v[2].b = 191;

	v[3].screen.xyw.x = (-w/2.0f)*ca + (h1/2.0f)*sa + mx;
	v[3].screen.xyw.y = (-w/2.0f)*sa - (h1/2.0f)*ca + my;
	v[3].world.xyz.z = pt1.world.xyz.z;
	v[3].screen.xyw.w = pt1.screen.xyw.w;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 1.0f;
	v[3].b = 191;

	color alpha_clr;
	gr_init_alphacolor(&alpha_clr, clr->red, clr->green, clr->blue, fl2i(alpha * 255.0f));

	material material_params;

	render_set_unlit_color_material(&material_params, texture, &alpha_clr, true, false);
	render_primitives_textured(&material_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

void render_laser_2d(int texture, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
{
	color clr;
	gr_init_alphacolor(&clr, 255, 255, 255, 255);

	render_laser_2d(texture, &clr, alpha, headp, head_width, tailp, tail_width, max_len);
}

// adapted from gr_bitmap()
void render_bitmap_internal(int texture, bool blended, float alpha, int _x, int _y, int resize_mode)
{
	int _w, _h;
	float x, y, w, h;
	vertex verts[4];

	if (gr_screen.mode == GR_STUB) {
		return;
	}

	bm_get_info(texture, &_w, &_h, NULL, NULL, NULL);

	x = i2fl(_x);
	y = i2fl(_y);
	w = i2fl(_w);
	h = i2fl(_h);

	// I will tidy this up later - RT
	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_posf(&x, &y, &w, &h, resize_mode);
	}

	memset(verts, 0, sizeof(verts));

	verts[0].screen.xyw.x = x;
	verts[0].screen.xyw.y = y;
	verts[0].texture_position.u = 0.0f;
	verts[0].texture_position.v = 0.0f;

	verts[1].screen.xyw.x = x + w;
	verts[1].screen.xyw.y = y;
	verts[1].texture_position.u = 1.0f;
	verts[1].texture_position.v = 0.0f;

	verts[2].screen.xyw.x = x + w;
	verts[2].screen.xyw.y = y + h;
	verts[2].texture_position.u = 1.0f;
	verts[2].texture_position.v = 1.0f;

	verts[3].screen.xyw.x = x;
	verts[3].screen.xyw.y = y + h;
	verts[3].texture_position.u = 0.0f;
	verts[3].texture_position.v = 1.0f;

	material material_params;

	render_set_interface_material(&material_params, texture, blended, alpha);
	render_primitives_textured(&material_params, verts, 4, PRIM_TYPE_TRIFAN, true);
}

void render_bitmap(int texture, int _x, int _y, int resize_mode)
{
	bool blended = false;
	float alpha = 1.0f;

	render_bitmap_internal(texture, blended, alpha, _x, _y, resize_mode);
}

void render_bitmap_blended(int texture, float alpha, int _x, int _y, int resize_mode)
{
	bool blended = true;

	render_bitmap_internal(texture, blended, alpha, _x, _y, resize_mode);
}

// adapted from g3_draw_rod()
void render_rod(color *clr, int num_points, vec3d *pvecs, float width)
{
	const int MAX_ROD_VERTS = 100;
	vec3d uvec, fvec, rvec;
	vec3d vecs[2];
	vertex pts[MAX_ROD_VERTS];
	int i, nv = 0;

	Assert( num_points >= 2 );
	Assert( (num_points * 2) <= MAX_ROD_VERTS );

	for (i = 0; i < num_points; i++) {
		vm_vec_sub(&fvec, &View_position, &pvecs[i]);
		vm_vec_normalize_safe(&fvec);

		int first = i+1;
		int second = i-1;

		if (i == 0) {
			first = 1;
			second = 0;
		} else if (i == num_points-1) {
			first = i;
		}

		vm_vec_sub(&rvec, &pvecs[first], &pvecs[second]);
		vm_vec_normalize_safe(&rvec);

		vm_vec_cross(&uvec, &rvec, &fvec);

		vm_vec_scale_add(&vecs[0], &pvecs[i], &uvec, width * 0.5f);
		vm_vec_scale_add(&vecs[1], &pvecs[i], &uvec, -width * 0.5f);

		if (nv > MAX_ROD_VERTS - 2) {
			Warning(LOCATION, "Hit high-water mark (%i) in g3_draw_rod()!!\n", MAX_ROD_VERTS);
			break;
		}

		g3_transfer_vertex( &pts[nv], &vecs[0] );
		g3_transfer_vertex( &pts[nv+1], &vecs[1] );

		pts[nv].texture_position.u = 1.0f;
		pts[nv].texture_position.v = i2fl(i);
		pts[nv].r = clr->red;
		pts[nv].g = clr->green;
		pts[nv].b = clr->blue;
		pts[nv].a = clr->alpha;

		pts[nv+1].texture_position.u = 0.0f;
		pts[nv+1].texture_position.v = i2fl(i);
		pts[nv+1].r = clr->red;
		pts[nv+1].g = clr->green;
		pts[nv+1].b = clr->blue;
		pts[nv+1].a = clr->alpha;

		nv += 2;
	}

	// we should always have at least 4 verts, and there should always be an even number
	Assert( (nv >= 4) && !(nv % 2) );

	material material_params;

	material_params.set_depth_mode(ZBUFFER_TYPE_READ);
	material_params.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	render_primitives_colored(&material_params, pts, nv, PRIM_TYPE_TRISTRIP, false);
}

// adapted from g3_draw_2d_rect()
void render_colored_rect(color *clr, int x, int y, int w, int h, int resize_mode)
{
	if (resize_mode != GR_RESIZE_NONE) {
		gr_resize_screen_pos(&x, &y, &w, &h, resize_mode);
	}

	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};

	memset(v,0,sizeof(vertex)*4);

	float sw = 0.1f;

	// stuff coords		
	v[0].screen.xyw.x = i2fl(x);
	v[0].screen.xyw.y = i2fl(y);
	v[0].screen.xyw.w = sw;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)clr->red;
	v[0].g = (ubyte)clr->green;
	v[0].b = (ubyte)clr->blue;
	v[0].a = (ubyte)clr->alpha;

	v[1].screen.xyw.x = i2fl(x + w);
	v[1].screen.xyw.y = i2fl(y);	
	v[1].screen.xyw.w = sw;
	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)clr->red;
	v[1].g = (ubyte)clr->green;
	v[1].b = (ubyte)clr->blue;
	v[1].a = (ubyte)clr->alpha;

	v[2].screen.xyw.x = i2fl(x + w);
	v[2].screen.xyw.y = i2fl(y + h);
	v[2].screen.xyw.w = sw;
	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)clr->red;
	v[2].g = (ubyte)clr->green;
	v[2].b = (ubyte)clr->blue;
	v[2].a = (ubyte)clr->alpha;

	v[3].screen.xyw.x = i2fl(x);
	v[3].screen.xyw.y = i2fl(y + h);
	v[3].screen.xyw.w = sw;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;				
	v[3].r = (ubyte)clr->red;
	v[3].g = (ubyte)clr->green;
	v[3].b = (ubyte)clr->blue;
	v[3].a = (ubyte)clr->alpha;

	material material_params;
	material_params.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_params.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_params.set_cull_mode(false);
	material_params.set_texture_source(TEXTURE_SOURCE_NONE);

	// draw the polys
	render_primitives_colored(&material_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

void render_colored_rect(int x, int y, int w, int h, int resize_mode)
{
	render_colored_rect(&gr_screen.current_color, x, y, w, h, resize_mode);
}

void render_colored_rect(shader *shade_clr, int x, int y, int w, int h, int resize_mode)
{
	color clr;
	gr_init_alphacolor(&clr, shade_clr->r, shade_clr->g, shade_clr->b, shade_clr->c);

	render_colored_rect(&clr, x, y, w, h, resize_mode);
}

// adapted from g3_draw_2d_shield_icon()
void render_shield_icon(color *clr, coord2d coords[6], int resize_mode)
{
	vertex v[6];
	vertex *verts[6] = {&v[0], &v[1], &v[2], &v[3], &v[4], &v[5]};

	memset(v,0,sizeof(vertex)*6);

	if (resize_mode != GR_RESIZE_NONE) {
		gr_resize_screen_pos(&coords[0].x, &coords[0].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[1].x, &coords[1].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[2].x, &coords[2].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[3].x, &coords[3].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[4].x, &coords[4].y, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&coords[5].x, &coords[5].y, NULL, NULL, resize_mode);
	}

	float sw = 0.1f;

	// stuff coords
	v[0].screen.xyw.x = i2fl(coords[0].x);
	v[0].screen.xyw.y = i2fl(coords[0].y);
	v[0].screen.xyw.w = sw;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;
	v[0].r = (ubyte)clr->red;
	v[0].g = (ubyte)clr->green;
	v[0].b = (ubyte)clr->blue;
	v[0].a = 0;

	v[1].screen.xyw.x = i2fl(coords[1].x);
	v[1].screen.xyw.y = i2fl(coords[1].y);
	v[1].screen.xyw.w = sw;
	v[1].texture_position.u = 0.0f;
	v[1].texture_position.v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;
	v[1].r = (ubyte)clr->red;
	v[1].g = (ubyte)clr->green;
	v[1].b = (ubyte)clr->blue;
	v[1].a = (ubyte)clr->alpha;

	v[2].screen.xyw.x = i2fl(coords[2].x);
	v[2].screen.xyw.y = i2fl(coords[2].y);
	v[2].screen.xyw.w = sw;
	v[2].texture_position.u = 0.0f;
	v[2].texture_position.v = 0.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;
	v[2].r = (ubyte)clr->red;
	v[2].g = (ubyte)clr->green;
	v[2].b = (ubyte)clr->blue;
	v[2].a = 0;

	v[3].screen.xyw.x = i2fl(coords[3].x);
	v[3].screen.xyw.y = i2fl(coords[3].y);
	v[3].screen.xyw.w = sw;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 0.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;
	v[3].r = (ubyte)clr->red;
	v[3].g = (ubyte)clr->green;
	v[3].b = (ubyte)clr->blue;
	v[3].a = (ubyte)clr->alpha;

	v[4].screen.xyw.x = i2fl(coords[4].x);
	v[4].screen.xyw.y = i2fl(coords[4].y);
	v[4].screen.xyw.w = sw;
	v[4].texture_position.u = 0.0f;
	v[4].texture_position.v = 0.0f;
	v[4].flags = PF_PROJECTED;
	v[4].codes = 0;
	v[4].r = (ubyte)clr->red;
	v[4].g = (ubyte)clr->green;
	v[4].b = (ubyte)clr->blue;
	v[4].a = 0;

	v[5].screen.xyw.x = i2fl(coords[5].x);
	v[5].screen.xyw.y = i2fl(coords[5].y);
	v[5].screen.xyw.w = sw;
	v[5].texture_position.u = 0.0f;
	v[5].texture_position.v = 0.0f;
	v[5].flags = PF_PROJECTED;
	v[5].codes = 0;
	v[5].r = (ubyte)clr->red;
	v[5].g = (ubyte)clr->green;
	v[5].b = (ubyte)clr->blue;
	v[5].a = 0;

	material material_instance;
	material_instance.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_instance.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_instance.set_cull_mode(false);
	material_instance.set_color(1.0f, 1.0f, 1.0f, 1.0f);
	
	// draw the polys
	render_primitives_colored(&material_instance, v, 4, PRIM_TYPE_TRISTRIP, true);
}

void render_shield_icon(coord2d coords[6], int resize_mode)
{
	render_shield_icon(&gr_screen.current_color, coords, resize_mode);
}

// adapted from gr_opengl_line()
void render_line(color *clr, int x1,int y1,int x2,int y2, int resize_mode)
{
	int do_resize;
	float sx1, sy1;
	float sx2, sy2;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);
	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);


	INT_CLIPLINE(x1, y1, x2, y2, clip_left, clip_top, clip_right, clip_bottom, return, ;, ;);

	sx1 = i2fl(x1 + offset_x);
	sy1 = i2fl(y1 + offset_y);
	sx2 = i2fl(x2 + offset_x);
	sy2 = i2fl(y2 + offset_y);


	if (do_resize) {
		gr_resize_screen_posf(&sx1, &sy1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&sx2, &sy2, NULL, NULL, resize_mode);
	}

	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NONE);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	mat.set_color(*clr);
	mat.set_cull_mode(false);

	if ( (x1 == x2) && (y1 == y2) ) {

		float vert[3]= {sx1, sy1, -0.99f};

		vertex_layout vert_def;

		vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, 0);
		
		gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_POINTS, &vert_def, 1, vert, sizeof(float) * 3);

		return;
	}

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	float line[6] = {
		sx2, sy2, -0.99f,
		sx1, sy1, -0.99f
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

	gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_LINES, &vert_def, 2, line, sizeof(float) * 6);
}

void render_line(int x1, int y1, int x2, int y2, int resize_mode)
{
	render_line(&gr_screen.current_color, x1, y1, x2, y2, resize_mode);
}

// adapted from gr_opengl_line_htl()
void render_line_3d(color *clr, bool depth_testing, vec3d *start, vec3d *end)
{
	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NONE);
	mat.set_depth_mode((depth_testing) ? ZBUFFER_TYPE_READ : ZBUFFER_TYPE_NONE);
	mat.set_color(*clr);
	mat.set_cull_mode(false);

    if (clr->is_alphacolor) {
		mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	} else {
        mat.set_blend_mode(ALPHA_BLEND_NONE);
    }

	float line[6] = {
		start->xyz.x,	start->xyz.y,	start->xyz.z,
		end->xyz.x,		end->xyz.y,		end->xyz.z
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

	gr_render_primitives_immediate(&mat, PRIM_TYPE_LINES, &vert_def, 2, line, sizeof(float) * 6);
}

void render_line_3d(bool depth_testing, vec3d *start, vec3d *end)
{
	render_line_3d(&gr_screen.current_color, depth_testing, start, end);
}

// adapted from gr_opengl_pixel()
void render_pixel(color *clr, int x, int y, int resize_mode)
{
	render_line(clr, x, y, x, y, resize_mode);
}

void render_pixel(int x, int y, int resize_mode)
{
	render_pixel(&gr_screen.current_color, x, y, resize_mode);
}

// adapted from opengl_bitmap_ex_internal()
void render_bitmap_ex_internal(int texture, float alpha, int x, int y, int w, int h, int sx, int sy, int resize_mode)
{
	if ( (w < 1) || (h < 1) ) {
		return;
	}

	float u_scale = 1.0f, v_scale = 1.0f;
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	mat.set_cull_mode(false);

	mat.set_texture_map(TM_BASE_TYPE, texture);
	mat.set_color(255, 255, 255, (ubyte)fl2i(alpha * 255.0f));

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info(texture, &bw, &bh);

	u0 = u_scale * (i2fl(sx) / i2fl(bw));
	v0 = v_scale * (i2fl(sy) / i2fl(bh));

	u1 = u_scale * (i2fl(sx+w) / i2fl(bw));
	v1 = v_scale * (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if (do_resize) {
		gr_resize_screen_posf(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&x2, &y2, NULL, NULL, resize_mode);
	}

	render_screen_points_textured(&mat, x1, y1, u0, v0, x2, y2, u1, v1);
}

void render_bitmap_ex(int texture, float alpha, int x, int y, int w, int h, int sx, int sy, int resize_mode)
{
	if ( texture < 0 ) {
		mprintf(("WARNING: trying to draw with invalid texture (%i)!\n", texture));
		return;
	}

	int reclip;
#ifndef NDEBUG
	int count = 0;
#endif

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;

	int bw, bh, do_resize;

	bm_get_info(texture, &bw, &bh);

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;

#ifndef NDEBUG
		if (count > 1) {
			Int3();
		}

		count++;
#endif

		if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
			return;
		}

		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
			return;
		}

		if ( dx1 < clip_left ) {
			sx += clip_left-dx1;
			dx1 = clip_left;
		}

		if ( dy1 < clip_top ) {
			sy += clip_top-dy1;
			dy1 = clip_top;
		}

		if ( dx2 > clip_right ) {
			dx2 = clip_right;
		}

		if ( dy2 > clip_bottom ) {
			dy2 = clip_bottom;
		}

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2 - dx1 + 1;
		h = dy2 - dy1 + 1;

		if ( (sx + w) > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( (sy + h) > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( (w < 1) || (h < 1) ) {
			// clipped away!
			return;
		}
	} while (reclip);

	// Make sure clipping algorithm works
#ifndef NDEBUG
	Assert( w > 0 );
	Assert( h > 0 );
	Assert( w == (dx2 - dx1 + 1) );
	Assert( h == (dy2 - dy1 + 1) );
	Assert( sx >= 0 );
	Assert( sy >= 0 );
	Assert( (sx + w) <= bw );
	Assert( (sy + h) <= bh );
	Assert( dx2 >= dx1 );
	Assert( dy2 >= dy1 );
	Assert( (dx1 >= clip_left) && (dx1 <= clip_right) );
	Assert( (dx2 >= clip_left) && (dx2 <= clip_right) );
	Assert( (dy1 >= clip_top) && (dy1 <= clip_bottom) );
	Assert( (dy2 >= clip_top) && (dy2 <= clip_bottom) );
#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	render_bitmap_ex_internal(texture, alpha, dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode);
}

// adapted from opengl_aabitmap_ex_internal()
void render_aabitmap_ex_internal(int texture, color *clr, int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror)
{
	if ( (w < 1) || (h < 1) ) {
		return;
	}

	if ( clr == NULL || !clr->is_alphacolor ) {
		return;
	}

	if ( texture < 0 ) {
		mprintf(("WARNING: trying to draw with invalid texture (%i)!\n", texture));
		return;
	}

	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	mat.set_cull_mode(false);

	mat.set_texture_map(TM_BASE_TYPE, texture);
	mat.set_texture_type(material::TEX_TYPE_AABITMAP);

	float u_scale = 1.0f;
	float v_scale = 1.0f;
	
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info(texture, &bw, &bh);

	u0 = u_scale * (i2fl(sx) / i2fl(bw));
	v0 = v_scale * (i2fl(sy) / i2fl(bh));

	u1 = u_scale * (i2fl(sx+w) / i2fl(bw));
	v1 = v_scale * (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if (do_resize) {
		gr_resize_screen_posf(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&x2, &y2, NULL, NULL, resize_mode);
	}

	mat.set_color(*clr);

	if (mirror) {
		float temp = u0;
		u0 = u1;
		u1 = temp;
	}

	render_screen_points_textured(&mat, x1,y1,u0,v0, x2,y2,u1,v1);
}

// adapted from gr_opengl_aabitmap_ex()
void render_aabitmap_ex(int texture, color *clr, int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror)
{
	if ( texture < 0 ) {
		mprintf(("WARNING: trying to draw with invalid texture (%i)!\n", texture));
		return;
	}

	int reclip;
#ifndef NDEBUG
	int count = 0;
#endif

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;

	int bw, bh, do_resize;

	bm_get_info(texture, &bw, &bh);

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;

#ifndef NDEBUG
		if (count > 1) {
			Int3();
		}

		count++;
#endif

		if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
			return;
		}

		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
			return;
		}

		if (dx1 < clip_left) {
			sx += clip_left - dx1;
			dx1 = clip_left;
		}

		if (dy1 < clip_top) {
			sy += clip_top - dy1;
			dy1 = clip_top;
		}

		if (dx2 > clip_right) {
			dx2 = clip_right;
		}

		if (dy2 > clip_bottom) {
			dy2 = clip_bottom;
		}


		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2 - dx1 + 1;
		h = dy2 - dy1 + 1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( (w < 1) || (h < 1) ) {
			// clipped away!
			return;
		}
	} while (reclip);

	// Make sure clipping algorithm works
#ifndef NDEBUG
	Assert( w > 0 );
	Assert( h > 0 );
	Assert( w == (dx2-dx1+1) );
	Assert( h == (dy2-dy1+1) );
	Assert( sx >= 0 );
	Assert( sy >= 0 );
	Assert( sx+w <= bw );
	Assert( sy+h <= bh );
	Assert( dx2 >= dx1 );
	Assert( dy2 >= dy1 );
	Assert( (dx1 >= clip_left) && (dx1 <= clip_right) );
	Assert( (dx2 >= clip_left) && (dx2 <= clip_right) );
	Assert( (dy1 >= clip_top) && (dy1 <= clip_bottom) );
	Assert( (dy2 >= clip_top) && (dy2 <= clip_bottom) );
#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	render_aabitmap_ex_internal(texture, clr, dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, mirror);
}

void render_aabitmap_ex(int texture, int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror)
{
	render_aabitmap_ex(texture, &gr_screen.current_color, x, y, w, h, sx, sy, resize_mode, mirror);
}

// adapted from gr_opengl_aabitmap()
void render_aabitmap(int texture, color *clr, int x, int y, int resize_mode, bool mirror)
{
	if ( texture < 0 ) {
		mprintf(("WARNING: trying to draw with invalid texture (%i)!\n", texture));
		return;
	}

	int w, h, do_resize;

	bm_get_info(texture, &w, &h);

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;
	int sx = 0, sy = 0;

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
		return;
	}

	if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
		return;
	}

	if (dx1 < clip_left) {
		sx = clip_left - dx1;
		dx1 = clip_left;
	}

	if (dy1 < clip_top) {
		sy = clip_top - dy1;
		dy1 = clip_top;
	}

	if (dx2 > clip_right) {
		dx2 = clip_right;
	}

	if (dy2 > clip_bottom) {
		dy2 = clip_bottom;
	}

	if ( (sx < 0) || (sy < 0) ) {
		return;
	}

	if ( (sx >= w) || (sy >= h) ) {
		return;
	}

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	render_aabitmap_ex_internal(texture, clr, dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, mirror);
}

void render_aabitmap(int texture, int x, int y, int resize_mode, bool mirror)
{
	render_aabitmap(texture, &gr_screen.current_color, x, y, resize_mode, mirror);
}

#define MAX_VERTS_PER_DRAW 120
struct string_vert { float x,y,u,v; };
static string_vert String_render_buff[MAX_VERTS_PER_DRAW];

// adapted from gr_opengl_string()
void render_string(color *clr, float sx, float sy, const char *s, int resize_mode)
{
	int width, spacing, letter;
	float x, y;
	bool do_resize;
	float bw, bh;
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	float u_scale = 1.0f, v_scale = 1.0f;

	if ( !Current_font || (*s == 0) ) {
		return;
	}

	material mat;

	mat.set_texture_map(TM_BASE_TYPE, Current_font->bitmap_id);
	mat.set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat.set_texture_type(material::TEX_TYPE_AABITMAP);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	mat.set_cull_mode(false);

	int buffer_offset = 0;

	int ibw, ibh;

	bm_get_info(Current_font->bitmap_id, &ibw, &ibh);

	bw = i2fl(ibw);
	bh = i2fl(ibh);

	// set color!
	mat.set_color(*clr);

//	if ( (gr_screen.custom_size && resize) || (gr_screen.rendering_to_texture != -1) ) {
	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = true;
	} else {
		do_resize = false;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	x = sx;
	y = sy;

	if (sx == (float)0x8000) {
		// centered
		x = (float)get_centered_x(s, !do_resize);
	} else {
		x = sx;
	}

	spacing = 0;

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(string_vert), (int)offsetof(string_vert, x));
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(string_vert), (int)offsetof(string_vert, u));

	// pick out letter coords, draw it, goto next letter and do the same
	while (*s)	{
		x += spacing;

		while (*s == '\n')	{
			s++;
			y += Current_font->h;

			if (sx == (float)0x8000) {
				// centered
				x = (float)get_centered_x(s, !do_resize);
			} else {
				x = sx;
			}
		}

		if (*s == 0) {
			break;
		}

		letter = get_char_width(s[0], s[1], &width, &spacing);
		s++;

		// not in font, draw as space
		if (letter < 0) {
			continue;
		}

		float xd, yd, xc, yc;
		float wc, hc;

		// Check if this character is totally clipped
		if ( (x + width) < clip_left ) {
			continue;
		}

		if ( (y + Current_font->h) < clip_top ) {
			continue;
		}

		if (x > clip_right) {
			continue;
		}

		if (y > clip_bottom) {
			continue;
		}

		xd = yd = 0;

		if (x < clip_left) {
			xd = clip_left - x;
		}

		if (y < clip_top) {
			yd = clip_top - y;
		}

		xc = x + xd;
		yc = y + yd;

		wc = width - xd;
		hc = Current_font->h - yd;

		if ( (xc + wc) > clip_right ) {
			wc = clip_right - xc;
		}

		if ( (yc + hc) > clip_bottom ) {
			hc = clip_bottom - yc;
		}

		if ( (wc < 1) || (hc < 1) ) {
			continue;
		}

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		x1 = xc + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
		y1 = yc + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);
		x2 = x1 + wc;
		y2 = y1 + hc;

		if (do_resize) {
			gr_resize_screen_posf( &x1, &y1, NULL, NULL, resize_mode );
			gr_resize_screen_posf( &x2, &y2, NULL, NULL, resize_mode );
		}

		u0 = u_scale * (i2fl(u+xd) / bw);
		v0 = v_scale * (i2fl(v+yd) / bh);

		u1 = u_scale * (i2fl((u+xd)+wc) / bw);
		v1 = v_scale * (i2fl((v+yd)+hc) / bh);

		if ( buffer_offset == MAX_VERTS_PER_DRAW ) {
			gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_TRIS, &vert_def, buffer_offset, String_render_buff, sizeof(string_vert) * buffer_offset);
			buffer_offset = 0;
		}

		String_render_buff[buffer_offset].x = (float)x1;
		String_render_buff[buffer_offset].y = (float)y1;
		String_render_buff[buffer_offset].u = u0;
		String_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		String_render_buff[buffer_offset].x = (float)x1;
		String_render_buff[buffer_offset].y = (float)y2;
		String_render_buff[buffer_offset].u = u0;
		String_render_buff[buffer_offset].v = v1;
		buffer_offset++;

		String_render_buff[buffer_offset].x = (float)x2;
		String_render_buff[buffer_offset].y = (float)y1;
		String_render_buff[buffer_offset].u = u1;
		String_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		String_render_buff[buffer_offset].x = (float)x1;
		String_render_buff[buffer_offset].y = (float)y2;
		String_render_buff[buffer_offset].u = u0;
		String_render_buff[buffer_offset].v = v1;
		buffer_offset++;

		String_render_buff[buffer_offset].x = (float)x2;
		String_render_buff[buffer_offset].y = (float)y1;
		String_render_buff[buffer_offset].u = u1;
		String_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		String_render_buff[buffer_offset].x = (float)x2;
		String_render_buff[buffer_offset].y = (float)y2;
		String_render_buff[buffer_offset].u = u1;
		String_render_buff[buffer_offset].v = v1;
		buffer_offset++;
	}

	if ( buffer_offset ) {
		gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_TRIS, &vert_def, buffer_offset, String_render_buff, sizeof(string_vert) * buffer_offset);
	}
}

void render_string(color *clr, int sx, int sy, const char *s, int resize_mode)
{
	render_string(clr, i2fl(sx), i2fl(sy), s, resize_mode);
}

void render_string(float sx, float sy, const char *s, int resize_mode)
{
	render_string(&gr_screen.current_color, sx, sy, s, resize_mode);
}

void render_string(int sx, int sy, const char *s, int resize_mode)
{
	render_string(&gr_screen.current_color, sx, sy, s, resize_mode);
}

// adapted from gr_opengl_aaline
void render_aaline(color *clr, vertex *v1, vertex *v2)
{
// -- AA OpenGL lines.  Looks good but they are kinda slow so this is disabled until an option is implemented - taylor
//	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
//	glEnable( GL_LINE_SMOOTH );
//	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
//	glLineWidth( 1.0 );

	vertex temp1 = *v1;
	vertex temp2 = *v2;
	vertex *ptr1 = &temp1;
	vertex *ptr2 = &temp2;

	clip_line(&ptr1, &ptr2, temp1.codes | temp2.codes, 0);

	vertex p1 = *ptr1;
	vertex p2 = *ptr2;

	if ( ptr1->flags & PF_TEMP_POINT )
		free_temp_point(ptr1);

	if ( ptr2->flags & PF_TEMP_POINT )
		free_temp_point(ptr2);

	if ( !( v1->flags & PF_PROJECTED ) ) {
		g3_project_vertex(&p1);
	}

	if ( !( v2->flags & PF_PROJECTED ) ) {
		g3_project_vertex(&p2);
	}

	if ( p1.flags & PF_OVERFLOW && p2.flags & PF_OVERFLOW ) {
		return;
	}

	float x1 = p1.screen.xyw.x;
	float y1 = p1.screen.xyw.y;
	float x2 = p2.screen.xyw.x;
	float y2 = p2.screen.xyw.y;
	float sx1, sy1;
	float sx2, sy2;

	FL_CLIPLINE(x1, y1, x2, y2, (float)gr_screen.clip_left, (float)gr_screen.clip_top, (float)gr_screen.clip_right, (float)gr_screen.clip_bottom, return, ;, ;);

	sx1 = x1 + (float)gr_screen.offset_x;
	sy1 = y1 + (float)gr_screen.offset_y;
	sx2 = x2 + (float)gr_screen.offset_x;
	sy2 = y2 + (float)gr_screen.offset_y;

	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NONE);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	mat.set_color(*clr);
	mat.set_cull_mode(false);

	if ( (x1 == x2) && (y1 == y2) ) {
		float vert[3]= {sx1, sy1, -0.99f};

		vertex_layout vert_def;

		vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

		gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_POINTS, &vert_def, 1, vert, sizeof(float) * 3);

		return;
	}

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	float line[6] = {
		sx2, sy2, -0.99f,
		sx1, sy1, -0.99f
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

	gr_render_primitives_2d_immediate(&mat, PRIM_TYPE_LINES, &vert_def, 2, line, sizeof(float) * 6);

//	glDisable( GL_LINE_SMOOTH );
}

void render_aaline(vertex *v1, vertex *v2)
{
	render_aaline(&gr_screen.current_color, v1, v2);
}

// adapted from gr_opengl_gradient()
void render_gradient(color *clr, int x1, int y1, int x2, int y2, int resize_mode)
{
	int swapped = 0;

	if ( !clr->is_alphacolor ) {
		render_line(clr, x1, y1, x2, y2, resize_mode);
		return;
	}

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&x2, &y2, NULL, NULL, resize_mode);
	}

	INT_CLIPLINE(x1, y1, x2, y2, gr_screen.clip_left, gr_screen.clip_top, gr_screen.clip_right, gr_screen.clip_bottom, return, ;, swapped = 1);

	material mat;

	mat.set_texture_source(TEXTURE_SOURCE_NONE);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat.set_depth_mode(ZBUFFER_TYPE_NONE);

	ubyte aa = swapped ? 0 : clr->alpha;
	ubyte ba = swapped ? clr->alpha : 0;

	float sx1, sy1, sx2, sy2;

	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}
	
	vertex vertices[2];

	vertices[0].screen.xyw.x = sx2;
	vertices[0].screen.xyw.y = sy2;
	vertices[0].r = clr->red;
	vertices[0].g = clr->green;
	vertices[0].b = clr->blue;
	vertices[0].a = ba;

	vertices[1].screen.xyw.x = sx1;
	vertices[1].screen.xyw.y = sy1;
	vertices[1].r = clr->red;
	vertices[1].g = clr->green;
	vertices[1].b = clr->blue;
	vertices[1].a = aa;
	
	render_primitives_colored(&mat, vertices, 2, PRIM_TYPE_LINES, true);
}

void render_gradient(int x1, int y1, int x2, int y2, int resize_mode)
{
	render_gradient(&gr_screen.current_color, x1, y1, x2, y2, resize_mode);
}

// adapted from gr_opengl_arc()
void render_arc(color *clr, int xc, int yc, float r, float angle_start, float angle_end, bool fill, float linewidth, int resize_mode)
{
	// Ensure that angle_start < angle_end
	if (angle_end < angle_start) {
		float temp = angle_start;
		angle_start = angle_end;
		angle_end = temp;
	}

	float arc_length_ratio;
	arc_length_ratio = MIN(angle_end - angle_start, 360.0f) / 360.0f;

	int segments = 4 + (int)(r * arc_length_ratio); // seems like a good approximation
	float theta = 2 * PI / float(segments - 1) * arc_length_ratio;
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x1 = cosf(ANG_TO_RAD(angle_start));
	float y1 = sinf(ANG_TO_RAD(angle_start));
	float x2 = x1;
	float y2 = y1;

	float halflinewidth = 0.0f;
	float inner_rad = 0.0f; // only used if fill==false
	float outer_rad = r;

	if (!fill) {
		halflinewidth = linewidth / 2.0f;
		inner_rad = r - halflinewidth;
		outer_rad = r + halflinewidth;
	}

	int do_resize = 0;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
		do_resize = 1;
	}

	// Big clip
	if ( (xc+outer_rad) < gr_screen.clip_left ) {
		return;
	}

	if ( (xc-outer_rad) > gr_screen.clip_right ) {
		return;
	}

	if ( (yc+outer_rad) < gr_screen.clip_top ) {
		return;
	}

	if ( (yc-outer_rad) > gr_screen.clip_bottom ) {
		return;
	}

	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);

	material material_params;

	material_params.set_texture_source(TEXTURE_SOURCE_NONE);
	material_params.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_params.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_params.set_cull_mode(false);
	material_params.set_color(*clr);

	float *arc;

	if (fill) {
		arc = new float[segments * 2 + 2];

		arc[0] = i2fl(xc);
		arc[1] = i2fl(yc);

		for (int i=2; i < segments * 2 + 2; i+=2) {
			arc[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
			arc[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

			t = x2;
			x2 = c * x1 - s * y1;
			y2 = s * t + c * y1;

			x1 = x2;
			y1 = y2;
		}

		vertex_layout vert_def;
		vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, 0);

		gr_render_primitives_2d_immediate(&material_params, PRIM_TYPE_TRIFAN, &vert_def, segments + 1, arc, sizeof(float) * (segments * 2 + 2));
	} else {
		arc = new float[segments * 4];

		for (int i=0; i < segments * 4; i+=4) {
			arc[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
			arc[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

			arc[i+2] = i2fl(xc + (x2 * inner_rad) + offset_x);
			arc[i+3] = i2fl(yc + (y2 * inner_rad) + offset_y);

			t = x2;
			x2 = c * x1 - s * y1;
			y2 = s * t + c * y1;

			x1 = x2;
			y1 = y2;
		}

		vertex_layout vert_def;
		vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, 0);

		gr_render_primitives_2d_immediate(&material_params, PRIM_TYPE_QUADSTRIP, &vert_def, segments * 2, arc, sizeof(float) * segments * 4);
	}

	delete [] arc;
}

// adapted from gr_opengl_circle()
void render_circle(color *clr, int xc, int yc, int d, int resize_mode)
{
	render_arc(clr, xc, yc, d / 2.0f, 0.0f, 360.0f, true, 0.0f, resize_mode);
}

void render_circle(int xc, int yc, int d, int resize_mode)
{
	render_circle(&gr_screen.current_color, xc, yc, d, resize_mode);
}

// adapted from gr_opengl_unfilled_circle()
void render_unfilled_circle(color *clr, float linewidth, int xc, int yc, int d, int resize_mode)
{
	int r = d / 2;
	int segments = 4 + (int)(r); // seems like a good approximation
	float theta = 2 * PI / float(segments - 1);
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x1 = 1.0f;
	float y1 = 0.0f;
	float x2 = x1;
	float y2 = y1;
	
	float halflinewidth = linewidth / 2.0f;
	float inner_rad = r - halflinewidth;
	float outer_rad = r + halflinewidth;

	int do_resize = 0;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
		do_resize = 1;
	}

	// Big clip
	if ( (xc+outer_rad) < gr_screen.clip_left ) {
		return;
	}

	if ( (xc-outer_rad) > gr_screen.clip_right ) {
		return;
	}

	if ( (yc+outer_rad) < gr_screen.clip_top ) {
		return;
	}

	if ( (yc-outer_rad) > gr_screen.clip_bottom ) {
		return;
	}

	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);

	material material_params;

	material_params.set_texture_source(TEXTURE_SOURCE_NONE);
	material_params.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_params.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_params.set_color(*clr);

	float *circle = new float[segments * 4];

	for (int i=0; i < segments * 4; i+=4) {
		circle[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
		circle[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

		circle[i+2] = i2fl(xc + (x2 * inner_rad) + offset_x);
		circle[i+3] = i2fl(yc + (y2 * inner_rad) + offset_y);

		t = x2;
		x2 = c * x1 - s * y1;
		y2 = s * t + c * y1;

		x1 = x2;
		y1 = y2;
	}

	vertex_layout vert_def;
	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, 0);

	gr_render_primitives_2d_immediate(&material_params, PRIM_TYPE_QUADSTRIP, &vert_def, segments * 2, circle, sizeof(float) * segments * 4);

	delete [] circle;
}

void render_unfilled_circle(float linewidth, int xc, int yc, int d, int resize_mode)
{
	render_unfilled_circle(&gr_screen.current_color, linewidth, xc, yc, d, resize_mode);
}

// adapted from gr_opengl_curve()
void render_curve(color *clr, int xc, int yc, int r, int direction, int resize_mode)
{
	int a, b, p;

	if (resize_mode != GR_RESIZE_NONE) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
	}

	if ( (xc + r) < gr_screen.clip_left ) {
		return;
	}

	if ( (yc + r) < gr_screen.clip_top ) {
		return;
	}

	p = 3 - (2 * r);
	a = 0;
	b = r;

	switch (direction) {
		case 0: {
			yc += r;
			xc += r;

			while (a < b) {
				// Draw the first octant
				render_line(clr, xc - b + 1, yc - a, xc - b, yc - a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					render_line(clr, xc - a + 1, yc - b, xc - a, yc - b, GR_RESIZE_NONE);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 1: {
			yc += r;

			while (a < b) {
				// Draw the first octant
				render_line(clr, xc + b - 1, yc - a, xc + b, yc - a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					render_line(clr, xc + a - 1, yc - b, xc + a, yc - b, GR_RESIZE_NONE);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 2: {
			xc += r;

			while (a < b) {
				// Draw the first octant
				render_line(clr, xc - b + 1, yc + a, xc - b, yc + a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					render_line(clr, xc - a + 1, yc + b, xc - a, yc + b, GR_RESIZE_NONE);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 3: {
			while (a < b) {
				// Draw the first octant
				render_line(clr, xc + b - 1, yc + a, xc + b, yc + a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					render_line(clr, xc + a - 1, yc + b, xc + a, yc + b, GR_RESIZE_NONE);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}
	}
}

void render_curve(int xc, int yc, int r, int direction, int resize_mode)
{
	render_curve(&gr_screen.current_color, xc, yc, r, direction, resize_mode);
}

/**
 * Given endpoints, and thickness, calculate coords of the endpoint
 * Adapted from gr_pline_helper()
 */
void render_pline_helper(vec3d *out, vec3d *in1, vec3d *in2, int thickness)
{
	vec3d slope;

	// slope of the line
	if(vm_vec_same(in1, in2)) {
		slope = vmd_zero_vector;
	} else {
		vm_vec_sub(&slope, in2, in1);
		float temp = -slope.xyz.x;
		slope.xyz.x = slope.xyz.y;
		slope.xyz.y = temp;
		vm_vec_normalize(&slope);
	}
	// get the points
	vm_vec_scale_add(out, in1, &slope, (float)thickness);
}


/**
 * Special function for drawing polylines.
 *
 * This function is specifically intended for polylines where each section 
 * is no more than 90 degrees away from a previous section.
 * Moreover, it is _really_ intended for use with 45 degree angles. 
 * Adapted from gr_pline_special()
 */
void render_pline_special(color *clr, SCP_vector<vec3d> *pts, int thickness,int resize_mode)
{
	vec3d s1, s2, e1, e2, dir;
	vec3d last_e1, last_e2;
	vertex v[4];
	vertex *verts[4] = {&v[0], &v[1], &v[2], &v[3]};
	int idx;
	int started_frame = 0;

	int num_pts = pts->size();

	// if we have less than 2 pts, bail
	if(num_pts < 2) {
		return;
	}

	extern int G3_count;
	if(G3_count == 0) {
		g3_start_frame(1);
		started_frame = 1;
	}
	
	float sw = 0.1f;

	material material_def;

	material_def.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_def.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_def.set_cull_mode(false);

	// draw each section
	last_e1 = vmd_zero_vector;
	last_e2 = vmd_zero_vector;
	int j;
	for(idx=0; idx<num_pts-1; idx++) {
		// get the start and endpoints
		s1 = pts->at(idx);													// start 1 (on the line)
		e1 = pts->at(idx+1);												// end 1 (on the line)
		render_pline_helper(&s2, &s1, &e1, thickness);	// start 2
		vm_vec_sub(&dir, &e1, &s1);
		vm_vec_add(&e2, &s2, &dir);											// end 2
		
		// stuff coords
		v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
		v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
		v[0].screen.xyw.w = sw;
		v[0].texture_position.u = 0.5f;
		v[0].texture_position.v = 0.5f;
		v[0].flags = PF_PROJECTED;
		v[0].codes = 0;
		v[0].r = clr->red;
		v[0].g = clr->green;
		v[0].b = clr->blue;
		v[0].a = clr->alpha;

		v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
		v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
		v[1].screen.xyw.w = sw;
		v[1].texture_position.u = 0.5f;
		v[1].texture_position.v = 0.5f;
		v[1].flags = PF_PROJECTED;
		v[1].codes = 0;
		v[1].r = clr->red;
		v[1].g = clr->green;
		v[1].b = clr->blue;
		v[1].a = clr->alpha;

		v[2].screen.xyw.x = (float)ceil(e2.xyz.x);
		v[2].screen.xyw.y = (float)ceil(e2.xyz.y);
		v[2].screen.xyw.w = sw;
		v[2].texture_position.u = 0.5f;
		v[2].texture_position.v = 0.5f;
		v[2].flags = PF_PROJECTED;
		v[2].codes = 0;
		v[2].r = clr->red;
		v[2].g = clr->green;
		v[2].b = clr->blue;
		v[2].a = clr->alpha;

		v[3].screen.xyw.x = (float)ceil(e1.xyz.x);
		v[3].screen.xyw.y = (float)ceil(e1.xyz.y);
		v[3].screen.xyw.w = sw;
		v[3].texture_position.u = 0.5f;
		v[3].texture_position.v = 0.5f;
		v[3].flags = PF_PROJECTED;
		v[3].codes = 0;
		v[3].r = clr->red;
		v[3].g = clr->green;
		v[3].b = clr->blue;
		v[3].a = clr->alpha;

		//We could really do this better...but oh well. _WMC
		if(resize_mode != GR_RESIZE_NONE) {
			for(j=0;j<4;j++) {
				gr_resize_screen_posf(&v[j].screen.xyw.x,&v[j].screen.xyw.y,NULL,NULL,resize_mode);
			}
		}

		// draw the polys
		//g3_draw_poly_constant_sw(4, verts, TMAP_FLAG_GOURAUD | TMAP_FLAG_RGB, 0.1f);
		render_primitives_colored(&material_def, v, 4, PRIM_TYPE_TRIFAN, true);

		// if we're past the first section, draw a "patch" triangle to fill any gaps
		if(idx > 0) {
			// stuff coords
			v[0].screen.xyw.x = (float)ceil(s1.xyz.x);
			v[0].screen.xyw.y = (float)ceil(s1.xyz.y);
			v[0].screen.xyw.w = sw;
			v[0].texture_position.u = 0.5f;
			v[0].texture_position.v = 0.5f;
			v[0].flags = PF_PROJECTED;
			v[0].codes = 0;
			v[0].r = clr->red;
			v[0].g = clr->green;
			v[0].b = clr->blue;
			v[0].a = clr->alpha;

			v[1].screen.xyw.x = (float)ceil(s2.xyz.x);
			v[1].screen.xyw.y = (float)ceil(s2.xyz.y);
			v[1].screen.xyw.w = sw;
			v[1].texture_position.u = 0.5f;
			v[1].texture_position.v = 0.5f;
			v[1].flags = PF_PROJECTED;
			v[1].codes = 0;
			v[1].r = clr->red;
			v[1].g = clr->green;
			v[1].b = clr->blue;
			v[1].a = clr->alpha;

			v[2].screen.xyw.x = (float)ceil(last_e2.xyz.x);
			v[2].screen.xyw.y = (float)ceil(last_e2.xyz.y);
			v[2].screen.xyw.w = sw;
			v[2].texture_position.u = 0.5f;
			v[2].texture_position.v = 0.5f;
			v[2].flags = PF_PROJECTED;
			v[2].codes = 0;
			v[2].r = clr->red;
			v[2].g = clr->green;
			v[2].b = clr->blue;
			v[2].a = clr->alpha;

			//Inefficiency or flexibility? you be the judge -WMC
			if(resize_mode != GR_RESIZE_NONE) {
				for(j=0;j<3;j++) {
					gr_resize_screen_posf(&v[j].screen.xyw.x,&v[j].screen.xyw.y,NULL,NULL,resize_mode);
				}
			}

			render_primitives_colored(&material_def, v, 3, PRIM_TYPE_TRIFAN, true);
		}

		// store our endpoints
		last_e1 = e1;
		last_e2 = e2;
	}

	if(started_frame) {
		g3_end_frame();
	}
}

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
// adapted from g3_draw_sphere()
void render_sphere_fast(color *clr, vertex *pnt, float rad)
{
	Assert( G3_count == 1 );

	if ( !( pnt->codes & CC_BEHIND ) ) {
		if ( !( pnt->flags & PF_PROJECTED ) ) {
			g3_project_vertex(pnt);
		}

		if (! (pnt->codes & PF_OVERFLOW) ) {
			float r2,t;
			r2 = rad*Matrix_scale.xyz.x;
			t=r2*Canv_w2/pnt->world.xyz.z;

			render_circle(clr, fl2i(pnt->screen.xyw.x),fl2i(pnt->screen.xyw.y),fl2i(t*2.0f),GR_RESIZE_NONE);
		}
	}
}

// adapted from g3_draw_sphere_ez()
void render_sphere_fast(color *clr, vec3d *pnt, float rad)
{
	vertex pt;
	ubyte flags;

	Assert( G3_count == 1 );

	flags = g3_rotate_vertex(&pt, pnt);

	if ( flags == 0 ) {
		g3_project_vertex(&pt);

		if ( !( pt.flags & PF_OVERFLOW ) ) {
			render_sphere_fast( clr, &pt, rad );
		}
	}
}

void render_sphere_fast(vertex *pnt, float rad)
{
	render_sphere_fast(&gr_screen.current_color, pnt, rad);
}

void render_sphere_fast(vec3d *pnt, float rad)
{
	render_sphere_fast(&gr_screen.current_color, pnt, rad);
}

void render_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct, int resize_mode)
{
	render_bitmap_blended(bmap1, 1.0f - pct, x1, y1, resize_mode);
	render_bitmap_blended(bmap2, pct, x2, y2, resize_mode);
}

void render_sphere(color *clr, vec3d* position, float radius)
{
	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	material material_def;

	material_def.set_texture_source(TEXTURE_SOURCE_NONE);
	material_def.set_blend_mode(ALPHA_BLEND_NONE);
	material_def.set_depth_mode(ZBUFFER_TYPE_FULL);
	material_def.set_color(*clr);

	gr_sphere(&material_def, radius);

	g3_done_instance(true);
}

void render_sphere(vec3d* position, float radius)
{
	render_sphere(&gr_screen.current_color, position, radius);
}

//draws a horizon. takes eax=sky_color, edx=ground_color
void render_horizon_line()
{
	int s1, s2;
	int cpnt;
	horz_pt horz_pts[4];		// 0 = left, 1 = right
	vec3d horizon_vec;
	float up_right, down_right,down_left,up_left;

	Assert( G3_count == 1 );

	//compute horizon_vector	
	horizon_vec.xyz.x = Unscaled_matrix.vec.rvec.xyz.y*Matrix_scale.xyz.y*Matrix_scale.xyz.z;
	horizon_vec.xyz.y = Unscaled_matrix.vec.uvec.xyz.y*Matrix_scale.xyz.x*Matrix_scale.xyz.z;
	horizon_vec.xyz.z = Unscaled_matrix.vec.fvec.xyz.y*Matrix_scale.xyz.x*Matrix_scale.xyz.y;

	// now compute values & flag for 4 corners.
	up_right = horizon_vec.xyz.x + horizon_vec.xyz.y + horizon_vec.xyz.z;
	down_right = horizon_vec.xyz.x - horizon_vec.xyz.y + horizon_vec.xyz.z;
	down_left = -horizon_vec.xyz.x - horizon_vec.xyz.y + horizon_vec.xyz.z;
	up_left = -horizon_vec.xyz.x + horizon_vec.xyz.y + horizon_vec.xyz.z;

	//check flags for all sky or all ground.
	if ( (up_right<0.0f)&&(down_right<0.0f)&&(down_left<0.0f)&&(up_left<0.0f) )	{
		return;
	}

	if ( (up_right>0.0f)&&(down_right>0.0f)&&(down_left>0.0f)&&(up_left>0.0f) )	{
		return;
	}

	// check for intesection with each of four edges & compute horizon line
	cpnt = 0;
	
	// check intersection with left edge
	s1 = up_left > 0.0f;
	s2 = down_left > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = 0.0f;
		horz_pts[cpnt].y = fl_abs(up_left * Canv_h2 / horizon_vec.xyz.y);
		horz_pts[cpnt].edge = 0;
		cpnt++;
	}

	// check intersection with top edge
	s1 = up_left > 0.0f;
	s2 = up_right > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = fl_abs(up_left * Canv_w2 / horizon_vec.xyz.x);
		horz_pts[cpnt].y = 0.0f;
		horz_pts[cpnt].edge = 1;
		cpnt++;
	}

	// check intersection with right edge
	s1 = up_right > 0.0f;
	s2 = down_right > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = i2fl(Canvas_width)-1;
		horz_pts[cpnt].y = fl_abs(up_right * Canv_h2 / horizon_vec.xyz.y);
		horz_pts[cpnt].edge = 2;
		cpnt++;
	}
	
	//check intersection with bottom edge
	s1 = down_right > 0.0f;
	s2 = down_left > 0.0f;
	if ( s1 != s2 )	{
		horz_pts[cpnt].x = fl_abs(down_left * Canv_w2 / horizon_vec.xyz.x);
		horz_pts[cpnt].y = i2fl(Canvas_height)-1;
		horz_pts[cpnt].edge = 3;
		cpnt++;
	}

	if ( cpnt != 2 )	{
		mprintf(( "HORZ: Wrong number of points (%d)\n", cpnt ));
		return;
	}

	//make sure first edge is left
	if ( horz_pts[0].x > horz_pts[1].x )	{
		horz_pt tmp;
		tmp = horz_pts[0];
		horz_pts[0] = horz_pts[1];
		horz_pts[1] = tmp;
	}

	// draw from left to right.
	render_line( fl2i(horz_pts[0].x),fl2i(horz_pts[0].y),fl2i(horz_pts[1].x),fl2i(horz_pts[1].y), GR_RESIZE_NONE );
}

void render_flash(int r, int g, int b)
{
	if ( !(r || g || b) ) {
		return;
	}

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	
	material material_def;

	material_def.set_texture_source(TEXTURE_SOURCE_NONE);
	material_def.set_blend_mode(ALPHA_BLEND_ALPHA_ADDITIVE);
	material_def.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_def.set_color( r, g, b, 255 );

	int x1 = (gr_screen.clip_left + gr_screen.offset_x);
	int y1 = (gr_screen.clip_top + gr_screen.offset_y);
	int x2 = (gr_screen.clip_right + gr_screen.offset_x) + 1;
	int y2 = (gr_screen.clip_bottom + gr_screen.offset_y) + 1;

	render_screen_points(&material_def, x1, y1, x2, y2);
}

void render_flash_alpha(int r, int g, int b, int a)
{
	if ( !(r || g || b || a) ) {
		return;
	}

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(a, 0, 255);

	material material_def;

	material_def.set_texture_source(TEXTURE_SOURCE_NONE);
	material_def.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	material_def.set_depth_mode(ZBUFFER_TYPE_NONE);
	material_def.set_color( r, g, b, a );

	int x1 = (gr_screen.clip_left + gr_screen.offset_x);
	int y1 = (gr_screen.clip_top + gr_screen.offset_y);
	int x2 = (gr_screen.clip_right + gr_screen.offset_x) + 1;
	int y2 = (gr_screen.clip_bottom + gr_screen.offset_y) + 1;

	render_screen_points(&material_def, x1, y1, x2, y2);
}