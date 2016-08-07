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
#include "freespace.h"
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

void render_set_unlit_color_material(material* mat_info, int texture, color *clr, float alpha, bool blending, bool depth_testing)
{
	color clr_with_alpha;
	gr_init_alphacolor(&clr_with_alpha, clr->red, clr->green, clr->blue, fl2i(alpha * 255.0f));

	render_set_unlit_color_material(mat_info, texture, &clr_with_alpha, blending, depth_testing);
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
void render_oriented_quad_internal(material* material_params, vec3d *pos, matrix *ori, float width, float height)
{
	//idiot-proof
	if ( width == 0 || height == 0 )
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

	for ( int i = 0; i < NUM_VERTICES; i++ ) {
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
	
	render_primitives_textured(material_params, v, 4, PRIM_TYPE_TRIFAN, false);
}

void render_oriented_quad(material* mat_info, vec3d *pos, matrix *ori, float width, float height)
{
	render_oriented_quad_internal(mat_info, pos, ori, width, height);
}

void render_oriented_quad(material* mat_info, vec3d *pos, vec3d *norm, float width, float height)
{
	matrix m;
	vm_vector_2_matrix(&m, norm, NULL, NULL);

	render_oriented_quad_internal(mat_info, pos, &m, width, height);
}

// adapted g3_draw_rotated_bitmap_3d()
//void render_rotated_bitmap(int texture, float alpha, vertex *pnt, float angle, float rad)
void render_rotated_bitmap(material *mat_params, vertex *pnt, float angle, float rad)
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

	render_primitives_textured(mat_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from gr_opengl_scaler()
void render_bitmap_scaler(material *mat_params, vertex *va, vertex *vb)
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

	render_primitives_textured(mat_params, v, 4, PRIM_TYPE_TRIFAN, true);
}

// adapted from g3_draw_bitmap()
//void render_oriented_bitmap_2d(int texture, float alpha, bool blending, vertex *pnt, int orient, float rad)
void render_oriented_bitmap_2d(material *mat_params, vertex *pnt, int orient, float rad)
{
	vertex va, vb;
	float t,w,h;
	float width, height;

	int bw, bh;

	bm_get_info(mat_params->get_texture_map(TM_BASE_TYPE), &bw, &bh, NULL );

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

	render_bitmap_scaler(mat_params, &va, &vb);
}

// adapted from g3_draw_bitmap_3d
void render_oriented_bitmap(material *mat_params, vertex *pnt, int orient, float rad, float depth)
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

	render_primitives_textured(mat_params, P, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from g3_draw_laser_htl()
void render_laser(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width)
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

	render_primitives_textured(mat_params, pts, 4, PRIM_TYPE_TRIFAN, false);
}

// adapted from g3_draw_laser()
//void render_laser_2d(int texture, color* clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
void render_laser_2d(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len)
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

	render_primitives_textured(mat_params, v, 4, PRIM_TYPE_TRIFAN, true);
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