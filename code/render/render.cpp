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
#include "render/3dinternal.h"
#include "graphics/material.h"
#include "render/render.h"

gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool is_transparent)
{
	if ( is_transparent ) {
		if ( base_bitmap >= 0 && bm_has_alpha_channel(base_bitmap) ) {
			return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		}

		return ALPHA_BLEND_ADDITIVE;
	}

	return ALPHA_BLEND_ALPHA_BLEND_ALPHA;
}

gr_zbuffer_type render_determine_depth_mode(bool depth_testing, bool is_transparent)
{
	if ( depth_testing ) {
		if ( is_transparent ) {
			return ZBUFFER_TYPE_READ;
		}

		return ZBUFFER_TYPE_FULL;
	}

	return ZBUFFER_TYPE_NONE;
}

void render_set_unlit_material(material* mat_info, int texture, bool blending, bool depth_testing)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);

	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, blending);
	gr_zbuffer_type depth_mode = render_determine_depth_mode(depth_testing, blending);

	mat_info->set_blend_mode(blend_mode);
	mat_info->set_depth_mode(depth_mode);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);
	mat_info->set_color(255, 255, 255, 255);
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
}

void render_set_unlit_material(material* mat_info, int texture, color *clr, bool blending, bool depth_testing)
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

void render_set_interface_material(material* mat_info, int texture)
{
	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_texture_type(material::TEX_TYPE_INTERFACE);

	mat_info->set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	mat_info->set_depth_mode(ZBUFFER_TYPE_NONE);
	mat_info->set_cull_mode(false);
	mat_info->set_texture_source(TEXTURE_SOURCE_NO_FILTERING);

	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
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

	mat_info->set_blend_mode(render_determine_blend_mode(texture, true));

	mat_info->set_texture_map(TM_BASE_TYPE, texture);
	mat_info->set_cull_mode(false);
	mat_info->set_color(1.0f, 1.0f, 1.0f, 1.0f);
}

// reproduces rendering behavior when using flags TMAP_FLAG_TEXTURED | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_HTL_3D_UNLIT
void render_colored_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, depth_testing);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, color *clr, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, clr, blending, depth_testing);

	material_info.set_color(*clr);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
}

void render_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, float alpha, bool blending, bool depth_testing)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, alpha, blending, depth_testing);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts, -1);
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

	gr_render_primitives_2d(&material_info, prim_type, &layout, 0, n_verts);
}

void render_primitives_2d(vertex* verts, int n_verts, primitive_type prim_type, int texture, color *clr, bool blending)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_unlit_material(&material_info, texture, blending, false);

	material_info.set_color(*clr);

	gr_render_primitives_2d(&material_info, prim_type, &layout, 0, n_verts);
}

void render_primitives_2d(vertex* verts, int n_verts, primitive_type prim_type, int texture, float alpha, bool blending)
{
	color clr;

	gr_init_alphacolor(&clr, 255, 255, 255, fl2i(alpha*255.0f));

	render_primitives_2d(verts, n_verts, prim_type, texture, &clr, blending);
}

void render_primitives_interface(vertex* verts, int n_verts, primitive_type prim_type, int texture)
{
	vertex_layout layout;

	layout.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);
	layout.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

	material material_info;

	render_set_interface_material(&material_info, texture);

	gr_render_primitives(&material_info, prim_type, &layout, 0, n_verts);
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

	render_primitives(v, 4, PRIM_TYPE_TRIFAN, texture, 255, true, true);
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

void render_rotated_bitmap(vertex *pnt, float angle, float rad, int texture, float alpha)
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

	vm_vec_crossprod(&rvec, &View_matrix.vec.fvec, &uvec);
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

	render_primitives(P, 4, PRIM_TYPE_TRIFAN, texture, alpha, true, true);
}

void render_bitmap_scaler(vertex *va, vertex *vb, int texture, float alpha, bool blending)
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

	render_primitives_2d(v, 4, PRIM_TYPE_TRIFAN, texture, alpha, blending);
}

void render_oriented_bitmap_2d(vertex *pnt, int orient, float rad, int texture, float alpha, bool blending)
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

	render_bitmap_scaler(&va, &vb, texture, alpha, blending);
}

void render_oriented_bitmap_2d(vertex *pnt, int orient, float rad, int texture)
{
	render_oriented_bitmap_2d(pnt, orient, rad, texture, 1.0f, false);
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

	render_primitives(P, 4, PRIM_TYPE_TRIFAN, texture, alpha, true, true);
}

void render_laser(vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b, int texture)
{
	width1 *= 0.5f;
	width2 *= 0.5f;
	vec3d uvec, fvec, rvec, center, reye, rfvec;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );
	vm_vec_copy_scale(&rfvec, &fvec, -1.0f);

	vm_vec_avg( &center, p0, p1 ); //needed for the return value only
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

	vm_vec_crossprod(&uvec, &fvec, &reye);
	vm_vec_normalize(&uvec);
	vm_vec_crossprod(&fvec, &uvec, &reye);
	vm_vec_normalize(&fvec);
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec, &uvec, &fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;
	vec3d start, end;
	vm_vec_scale_add(&start, p0, &fvec, -width1);
	vm_vec_scale_add(&end, p1, &fvec, width2);
	vec3d vecs[4];
	vertex pts[4];

	vm_vec_scale_add( &vecs[0], &start, &uvec, width1 );
	vm_vec_scale_add( &vecs[1], &end, &uvec, width2 );
	vm_vec_scale_add( &vecs[2], &end, &uvec, -width2 );
	vm_vec_scale_add( &vecs[3], &start, &uvec, -width1 );

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

	pts[0].r = (ubyte)r;
	pts[0].g = (ubyte)g;
	pts[0].b = (ubyte)b;
	pts[0].a = 255;
	pts[1].r = (ubyte)r;
	pts[1].g = (ubyte)g;
	pts[1].b = (ubyte)b;
	pts[1].a = 255;
	pts[2].r = (ubyte)r;
	pts[2].g = (ubyte)g;
	pts[2].b = (ubyte)b;
	pts[2].a = 255;
	pts[3].r = (ubyte)r;
	pts[3].g = (ubyte)g;
	pts[3].b = (ubyte)b;
	pts[3].a = 255;

	render_colored_primitives(pts, 4, PRIM_TYPE_TRIFAN, texture, true, true);
}

void render_bitmap(int _x, int _y, int texture, int resize_mode)
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
	
	render_primitives_interface(verts, 4, PRIM_TYPE_TRIFAN, texture);
}

int render_rod(vec3d *p0,float width1,vec3d *p1,float width2, vertex * verts, uint tmap_flags)
{
	vec3d uvec, fvec, rvec, center;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );

	vm_vec_avg( &center, p0, p1 );
	vm_vec_sub( &rvec, &Eye_position, &center );
	vm_vec_normalize( &rvec );

	vm_vec_crossprod(&uvec,&fvec,&rvec);
			
	//normalize new perpendicular vector
	vm_vec_normalize(&uvec);
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec,&uvec,&fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;
	vec3d vecs[4];
	vertex pts[4];
	vertex *ptlist[4] = { &pts[3], &pts[2], &pts[1], &pts[0] };

	vm_vec_scale_add( &vecs[0], p0, &uvec, width1/2.0f );
	vm_vec_scale_add( &vecs[1], p1, &uvec, width2/2.0f );
	vm_vec_scale_add( &vecs[2], p1, &uvec, -width2/2.0f );
	vm_vec_scale_add( &vecs[3], p0, &uvec, -width1/2.0f );
	
	for (i=0; i<4; i++ )	{
		if ( verts )	{
			pts[i] = verts[i];
		}

		g3_transfer_vertex( &pts[i], &vecs[i] );
	}
	ptlist[0]->texture_position.u = 0.0f;
	ptlist[0]->texture_position.v = 0.0f;
	ptlist[0]->r = gr_screen.current_color.red;
	ptlist[0]->g = gr_screen.current_color.green;
	ptlist[0]->b = gr_screen.current_color.blue;
	ptlist[0]->a = gr_screen.current_color.alpha;

	ptlist[1]->texture_position.u = 1.0f;
	ptlist[1]->texture_position.v = 0.0f;
	ptlist[1]->r = gr_screen.current_color.red;
	ptlist[1]->g = gr_screen.current_color.green;
	ptlist[1]->b = gr_screen.current_color.blue;
	ptlist[1]->a = gr_screen.current_color.alpha;

	ptlist[2]->texture_position.u = 1.0f;
	ptlist[2]->texture_position.v = 1.0f;
	ptlist[2]->r = gr_screen.current_color.red;
	ptlist[2]->g = gr_screen.current_color.green;
	ptlist[2]->b = gr_screen.current_color.blue;
	ptlist[2]->a = gr_screen.current_color.alpha;

	ptlist[3]->texture_position.u = 0.0f;
	ptlist[3]->texture_position.v = 1.0f;
	ptlist[3]->r = gr_screen.current_color.red;
	ptlist[3]->g = gr_screen.current_color.green;
	ptlist[3]->b = gr_screen.current_color.blue;
	ptlist[3]->a = gr_screen.current_color.alpha;

	return g3_draw_poly(4,ptlist,tmap_flags);
}

void render_rod(int num_points, vec3d *pvecs, float width, color *clr)
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

		vm_vec_crossprod(&uvec, &rvec, &fvec);

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

	material material_instance;
	material_instance.set_depth_mode(ZBUFFER_TYPE_READ);
	material_instance.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &pts[0].world.xyz.x);
	layout.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &pts[0].r);

	gr_render_primitives(&material_instance, PRIM_TYPE_TRISTRIP, &layout, 0, nv);
}