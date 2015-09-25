/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "render/batching.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "graphics/material.h"
#include "render/render.h"

static SCP_map<batch_info, primitive_batch> Batching_primitives;

vertex_layout Effect_vertex_layout;
vertex_layout Effect_vertex_no_radius_layout;
vertex_layout Point_sprite_vertex_layout;

primitive_batch_buffer Batch_effect_vertex_queue;
primitive_batch_buffer Batch_effect_vertex_no_radius_queue;
primitive_batch_buffer Batch_point_sprite_queue;

void primitive_batch::add_triangle(effect_vertex* v0, effect_vertex* v1, effect_vertex *v2)
{

}

void primitive_batch::add_point_sprite(particle_pnt *sprite)
{

}

int primitive_batch::load_buffer_triangles(effect_vertex* buffer, int n_verts)
{
	int verts_to_render = Triangle_verts.size();

	for ( int i = 0; i < verts_to_render; ++i) {
		buffer[n_verts+i] = Triangle_verts[i];
	}

	return verts_to_render;
}

int primitive_batch::load_buffer_points(particle_pnt* buffer, int n_verts)
{
	int verts_to_render = Point_sprite_verts.size();
	int i;

	for ( i = 0; i < verts_to_render; ++i) {
		buffer[n_verts+i] = Point_sprite_verts[i];
	}

	return verts_to_render;
}

void primitive_batch::clear()
{
	Triangle_verts.clear();
	Point_sprite_verts.clear();
}

void batching_init_buffers()
{
	Batch_effect_vertex_queue.buffer_num = gr_create_stream_buffer();
	Batch_effect_vertex_no_radius_queue.buffer_num = gr_create_stream_buffer();
	Batch_point_sprite_queue.buffer_num = gr_create_stream_buffer();

	Batch_effect_vertex_queue.buffer_ptr = NULL;
	Batch_effect_vertex_no_radius_queue.buffer_ptr = NULL;
	Batch_point_sprite_queue.buffer_ptr = NULL;

	Batch_effect_vertex_queue.buffer_size = 0;
	Batch_effect_vertex_no_radius_queue.buffer_size = 0;
	Batch_point_sprite_queue.buffer_size = 0;

	Batch_effect_vertex_queue.layout = &Effect_vertex_layout;
	Batch_effect_vertex_no_radius_queue.layout = &Effect_vertex_no_radius_layout;
	Batch_point_sprite_queue.layout = &Point_sprite_vertex_layout;

	Batch_effect_vertex_queue.triangles = true;
	Batch_effect_vertex_no_radius_queue.triangles = true;
	Batch_point_sprite_queue.triangles = false;
}

void batching_init_vertex_layouts()
{
	int offset = 0;
	int stride = sizeof(effect_vertex);

	Effect_vertex_layout.add_vertex_component(vertex_format_data::POSITION3, stride, (ubyte*)offset);
	Effect_vertex_no_radius_layout.add_vertex_component(vertex_format_data::POSITION3, stride, (ubyte*)offset);
	offset += sizeof(vec3d);

	Effect_vertex_layout.add_vertex_component(vertex_format_data::TEX_COORD, stride, (ubyte*)offset);
	Effect_vertex_no_radius_layout.add_vertex_component(vertex_format_data::TEX_COORD, stride, (ubyte*)offset);
	offset += sizeof(uv_pair);

	Effect_vertex_layout.add_vertex_component(vertex_format_data::RADIUS, stride, (ubyte*)offset);
	offset += sizeof(float);

	Effect_vertex_layout.add_vertex_component(vertex_format_data::COLOR4, stride, (ubyte*)offset);
	Effect_vertex_no_radius_layout.add_vertex_component(vertex_format_data::COLOR4, stride, (ubyte*)offset);
	offset += sizeof(ubyte) * 4;

	offset = 0;
	stride = sizeof(particle_pnt);

	Point_sprite_vertex_layout.add_vertex_component(vertex_format_data::POSITION3, stride, (ubyte*)offset);
	offset += sizeof(vec3d);

	Point_sprite_vertex_layout.add_vertex_component(vertex_format_data::RADIUS, stride, (ubyte*)offset);
	offset += sizeof(float);

	Point_sprite_vertex_layout.add_vertex_component(vertex_format_data::UVEC, stride, (ubyte*)offset);
	offset += sizeof(vec3d);
}

void batching_determine_color(color *clr, int texture, float alpha)
{
	gr_alpha_blend blend_mode = render_determine_blend_mode(texture, true);

	if ( blend_mode == ALPHA_BLEND_ADDITIVE ) {
		gr_init_alphacolor(clr, fl2i(255.0f*alpha), fl2i(255.0f*alpha), fl2i(255.0f*alpha), 255);
	} else {
		gr_init_alphacolor(clr, 255, 255, 255, fl2i(255.0f*alpha));
	}
}

primitive_batch* batching_find_batch(int texture, batch_info::render_type batch_type)
{
	batch_info query;

	query.texture = texture;
	query.selected_render_type = batch_type;

	SCP_map<batch_info, primitive_batch>::iterator iter = Batching_primitives.find(query);

	if ( iter == Batching_primitives.end() ) {
		return &Batching_primitives[query];
	} else {
		return &iter->second;
	}
}

void batching_add_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_color(&clr, texture, alpha);

	batching_add_bitmap(batch, pnt, orient, rad, &clr, 0.0f);
}

void batching_add_volume_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::VOLUME_EMISSIVE);

	color clr;
	batching_determine_color(&clr, texture, alpha);

	batching_add_bitmap(batch, pnt, orient, rad, &clr, 0.0f);
}

void batching_add_distortion_bitmap(int texture, vertex *pnt, int orient, float rad, float alpha)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::DISTORTION);

	color clr;
	batching_determine_color(&clr, texture, alpha);

	batching_add_bitmap(batch, pnt, orient, rad, &clr, 0.0f);
}

void batching_add_beam(int texture, vec3d *start, vec3d *end, float width, float intensity)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_color(&clr, texture, intensity);

	batching_add_beam(batch, start, end, width, &clr, 0.0f);
}

void batch_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	if (texture < 0) {
		Int3();
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	batching_add_laser(batch, p0, width1, p1, width2, r, g, b);
}

void batching_add_polygon(int texture, vec3d *pos, matrix *orient, float width, float height, float alpha)
{
	if (texture < 0) {
		Int3();
		return;
	}

	primitive_batch *batch = batching_find_batch(texture, batch_info::FLAT_EMISSIVE);

	color clr;
	batching_determine_color(&clr, texture, alpha);

	batching_add_polygon(batch, texture, pos, orient, width, height, &clr);
}

void batching_add_bitmap(primitive_batch *batch, vertex *pnt, int orient, float rad, color *clr, float depth)
{
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(pnt->world);
	vec3d p[4];
	vec3d fvec, rvec, uvec;
	effect_vertex verts[6];

	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// get an up vector in the general direction of what we want
	uvec = View_matrix.vec.uvec;

	// make a right vector from the f and up vector, this r vec is exactly what we want, so...
	vm_vec_crossprod(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize_safe(&rvec);

	// fix the u vec with it
	vm_vec_crossprod(&uvec, &View_matrix.vec.fvec, &rvec);

	// move the center of the sprite based on the depth parameter
	if ( depth != 0.0f )
		vm_vec_scale_add(&PNT, &PNT, &fvec, depth);

	// move one of the verts to the left
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);

	// and one to the right
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	// now move all oof the verts to were they need to be
	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);

	//move all the data from the vecs into the verts
	//tri 1
	verts[5].position = p[3];
	verts[4].position = p[2];
	verts[3].position = p[1];

	//tri 2
	verts[2].position = p[3];
	verts[1].position = p[1];
	verts[0].position = p[0];

	// set up the UV coords
	if ( orient & 1 ) {
		// tri 1
		verts[5].tex_coord.u = 1.0f;
		verts[4].tex_coord.u = 0.0f;
		verts[3].tex_coord.u = 0.0f;

		// tri 2
		verts[2].tex_coord.u = 1.0f;
		verts[1].tex_coord.u = 0.0f;
		verts[0].tex_coord.u = 1.0f;
	} else {
		// tri 1
		verts[5].tex_coord.u = 0.0f;
		verts[4].tex_coord.u = 1.0f;
		verts[3].tex_coord.u = 1.0f;

		// tri 2
		verts[2].tex_coord.u = 0.0f;
		verts[1].tex_coord.u = 1.0f;
		verts[0].tex_coord.u = 0.0f;
	}

	if ( orient & 2 ) {
		// tri 1
		verts[5].tex_coord.v = 1.0f;
		verts[4].tex_coord.v = 1.0f;
		verts[3].tex_coord.v = 0.0f;

		// tri 2
		verts[2].tex_coord.v = 1.0f;
		verts[1].tex_coord.v = 0.0f;
		verts[0].tex_coord.v = 0.0f;
	} else {
		// tri 1
		verts[5].tex_coord.v = 0.0f;
		verts[4].tex_coord.v = 0.0f;
		verts[3].tex_coord.v = 1.0f;

		// tri 2
		verts[2].tex_coord.v = 0.0f;
		verts[1].tex_coord.v = 1.0f;
		verts[0].tex_coord.v = 1.0f;
	}

	for (int i = 0; i < 6 ; i++) {
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		verts[i].radius = radius;
	}

	batch->add_triangle(&verts[2], &verts[1], &verts[0]);
}

void batching_add_point_bitmap(primitive_batch *batch, vertex *position, int orient, float rad, float depth)
{
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(position->world);
	vec3d fvec;

	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// move the center of the sprite based on the depth parameter
	if ( depth != 0.0f )
		vm_vec_scale_add(&PNT, &PNT, &fvec, depth);

	particle_pnt new_particle;
	vec3d up;

	new_particle.position = position->world;
	new_particle.size = rad;

	int direction = orient % 4;

	switch ( direction ) {
	case 0:
		up.xyz.x = 0.0f;
		up.xyz.y = 1.0f;
		up.xyz.z = 0.0f;
		break;
	case 1:
		up.xyz.x = 0.0f;
		up.xyz.y = -1.0f;
		up.xyz.z = 0.0f;
		break;
	case 2:
		up.xyz.x = -1.0f;
		up.xyz.y = 0.0f;
		up.xyz.z = 0.0f;
		break;
	case 3:
		up.xyz.x = 1.0f;
		up.xyz.y = 0.0f;
		up.xyz.z = 0.0f;
		break;
	}

	new_particle.up = up;

	batch->add_point_sprite(&new_particle);
}

void batching_add_bitmap(primitive_batch *batch, vertex *pnt, float rad, float angle, color *clr, float depth)
{
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	extern float Physics_viewer_bank;
	angle -= Physics_viewer_bank;

	if ( angle < 0.0f )
		angle += PI2;
	else if ( angle > PI2 )
		angle -= PI2;

	vec3d PNT(pnt->world);
	vec3d p[4];
	vec3d fvec, rvec, uvec;
	effect_vertex verts[6];

	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	vm_rot_point_around_line(&uvec, &View_matrix.vec.uvec, angle, &vmd_zero_vector, &View_matrix.vec.fvec);

	vm_vec_crossprod(&rvec, &View_matrix.vec.fvec, &uvec);
	vm_vec_normalize_safe(&rvec);
	vm_vec_crossprod(&uvec, &View_matrix.vec.fvec, &rvec);

	vm_vec_scale_add(&PNT, &PNT, &fvec, depth);
	vm_vec_scale_add(&p[0], &PNT, &rvec, rad);
	vm_vec_scale_add(&p[2], &PNT, &rvec, -rad);

	vm_vec_scale_add(&p[1], &p[2], &uvec, rad);
	vm_vec_scale_add(&p[3], &p[0], &uvec, -rad);
	vm_vec_scale_add(&p[0], &p[0], &uvec, rad);
	vm_vec_scale_add(&p[2], &p[2], &uvec, -rad);


	//move all the data from the vecs into the verts
	//tri 1
	verts[5].position = p[3];
	verts[4].position = p[2];
	verts[3].position = p[1];

	//tri 2
	verts[2].position = p[3];
	verts[1].position = p[1];
	verts[0].position = p[0];

	//tri 1
	verts[5].tex_coord.u = 0.0f;	verts[5].tex_coord.v = 0.0f;
	verts[4].tex_coord.u = 1.0f;	verts[4].tex_coord.v = 0.0f;
	verts[3].tex_coord.u = 1.0f;	verts[3].tex_coord.v = 1.0f;

	//tri 2
	verts[2].tex_coord.u = 0.0f;	verts[2].tex_coord.v = 0.0f;
	verts[1].tex_coord.u = 1.0f;	verts[1].tex_coord.v = 1.0f;
	verts[0].tex_coord.u = 0.0f;	verts[0].tex_coord.v = 1.0f;

	for (int i = 0; i < 6 ; i++) {
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		verts[i].radius = radius;
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_add_point_bitmap(primitive_batch *batch, vertex *position, float rad, float angle, float depth)
{
	float radius = rad;
	rad *= 1.41421356f;//1/0.707, becase these are the points of a square or width and height rad

	vec3d PNT(position->world);
	vec3d fvec;

	// get the direction from the point to the eye
	vm_vec_sub(&fvec, &View_position, &PNT);
	vm_vec_normalize_safe(&fvec);

	// move the center of the sprite based on the depth parameter
	if ( depth != 0.0f )
		vm_vec_scale_add(&PNT, &PNT, &fvec, depth);

	particle_pnt new_particle;
	vec3d up;

	vm_rot_point_around_line(&up, &vmd_y_vector, angle, &vmd_zero_vector, &vmd_z_vector);

	new_particle.position = position->world;
	new_particle.size = rad;
	new_particle.up = up;

	batch->add_point_sprite(&new_particle);
}

int batching_add_polygon(primitive_batch *batch, int texture, vec3d *pos, matrix *orient, float width, float height, color *clr)
{
	//idiot-proof
	if(width == 0 || height == 0)
		return 0;

	Assert(pos != NULL);
	Assert(orient != NULL);

	//Let's begin.

	const int NUM_VERTICES = 4;
	vec3d p[NUM_VERTICES] = { ZERO_VECTOR };
	effect_vertex v[NUM_VERTICES];

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
		vm_vec_unrotate(&tmp, &p[i], orient);
		//Move to point in space
		vm_vec_add2(&tmp, pos);

		v[i].position = tmp;

		v[i].r = clr->red;
		v[i].g = clr->green;
		v[i].b = clr->blue;
		v[i].a = clr->alpha;
	}

	v[0].tex_coord.u = 1.0f;
	v[0].tex_coord.v = 0.0f;

	v[1].tex_coord.u = 0.0f;
	v[1].tex_coord.v = 0.0f;

	v[2].tex_coord.u = 0.0f;
	v[2].tex_coord.v = 1.0f;

	v[3].tex_coord.u = 1.0f;
	v[3].tex_coord.v = 1.0f;

	batch->add_triangle(&v[0], &v[1], &v[2]);
	batch->add_triangle(&v[0], &v[2], &v[3]);
}

void batching_add_beam(primitive_batch *batch, vec3d *start, vec3d *end, float width, color *clr, float offset)
{
	vec3d p[4];
	effect_vertex verts[6];

	vec3d fvec, uvecs, uvece, evec;

	vm_vec_sub(&fvec, start, end);
	vm_vec_normalize_safe(&fvec);

	vm_vec_sub(&evec, &View_position, start);
	vm_vec_normalize_safe(&evec);

	vm_vec_crossprod(&uvecs, &fvec, &evec);
	vm_vec_normalize_safe(&uvecs);

	vm_vec_sub(&evec, &View_position, end);
	vm_vec_normalize_safe(&evec);

	vm_vec_crossprod(&uvece, &fvec, &evec);
	vm_vec_normalize_safe(&uvece);

	vm_vec_scale_add(&p[0], start, &uvecs, width);
	vm_vec_scale_add(&p[1], end, &uvece, width);
	vm_vec_scale_add(&p[2], end, &uvece, -width);
	vm_vec_scale_add(&p[3], start, &uvecs, -width);

	//move all the data from the vecs into the verts
	//tri 1
	verts[0].position = p[3];
	verts[1].position = p[2];
	verts[2].position = p[1];

	//tri 2
	verts[3].position = p[3];
	verts[4].position = p[1];
	verts[5].position = p[0];

	//set up the UV coords
	//tri 1
	verts[0].tex_coord.u = 0.0f; verts[0].tex_coord.v = 0.0f;
	verts[1].tex_coord.u = 1.0f; verts[1].tex_coord.v = 0.0f;
	verts[2].tex_coord.u = 1.0f; verts[2].tex_coord.v = 1.0f;

	//tri 2
	verts[3].tex_coord.u = 0.0f; verts[3].tex_coord.v = 0.0f;
	verts[4].tex_coord.u = 1.0f; verts[4].tex_coord.v = 1.0f;
	verts[5].tex_coord.u = 0.0f; verts[5].tex_coord.v = 1.0f;

	for(int i = 0; i < 6; i++){
		verts[i].r = clr->red;
		verts[i].g = clr->green;
		verts[i].b = clr->blue;
		verts[i].a = clr->alpha;

		if(offset > 0.0f) {
			verts[i].radius = offset;
		} else {
			verts[i].radius = width;
		}
	}

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

float batching_add_laser(primitive_batch *batch, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b)
{
	width1 *= 0.5f;
	width2 *= 0.5f;

	vec3d uvec, fvec, rvec, center, reye;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );

	vm_vec_avg( &center, p0, p1 ); // needed for the return value only
	vm_vec_sub(&reye, &Eye_position, &center);
	vm_vec_normalize(&reye);

	// compute the up vector
	vm_vec_crossprod(&uvec, &fvec, &reye);
	vm_vec_normalize_safe(&uvec);
	// ... the forward vector
	vm_vec_crossprod(&fvec, &uvec, &reye);
	vm_vec_normalize_safe(&fvec);
	// now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec, &uvec, &fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	vec3d start, end;

	vm_vec_scale_add(&start, p0, &fvec, -width1);
	vm_vec_scale_add(&end, p1, &fvec, width2);

	vec3d vecs[4];
	effect_vertex verts[6];

	vm_vec_scale_add( &vecs[0], &end, &uvec, width2 );
	vm_vec_scale_add( &vecs[1], &start, &uvec, width1 );
	vm_vec_scale_add( &vecs[2], &start, &uvec, -width1 );
	vm_vec_scale_add( &vecs[3], &end, &uvec, -width2 );

	verts[0].position = vecs[0];
	verts[1].position = vecs[1];
	verts[2].position = vecs[2];

	verts[3].position = vecs[0];
	verts[4].position = vecs[2];
	verts[5].position = vecs[3];

	verts[0].tex_coord.u = 1.0f;
	verts[0].tex_coord.v = 0.0f;
	verts[1].tex_coord.u = 0.0f;
	verts[1].tex_coord.v = 0.0f;
	verts[2].tex_coord.u = 0.0f;
	verts[2].tex_coord.v = 1.0f;

	verts[3].tex_coord.u = 1.0f;
	verts[3].tex_coord.v = 0.0f;
	verts[4].tex_coord.u = 0.0f;
	verts[4].tex_coord.v = 1.0f;
	verts[5].tex_coord.u = 1.0f;
	verts[5].tex_coord.v = 1.0f;

	verts[0].r = (ubyte)r;
	verts[0].g = (ubyte)g;
	verts[0].b = (ubyte)b;
	verts[0].a = 255;
	verts[1].r = (ubyte)r;
	verts[1].g = (ubyte)g;
	verts[1].b = (ubyte)b;
	verts[1].a = 255;
	verts[2].r = (ubyte)r;
	verts[2].g = (ubyte)g;
	verts[2].b = (ubyte)b;
	verts[2].a = 255;
	verts[3].r = (ubyte)r;
	verts[3].g = (ubyte)g;
	verts[3].b = (ubyte)b;
	verts[3].a = 255;
	verts[4].r = (ubyte)r;
	verts[4].g = (ubyte)g;
	verts[4].b = (ubyte)b;
	verts[4].a = 255;
	verts[5].r = (ubyte)r;
	verts[5].g = (ubyte)g;
	verts[5].b = (ubyte)b;
	verts[5].a = 255;

	batch->add_triangle(&verts[0], &verts[1], &verts[2]);
	batch->add_triangle(&verts[3], &verts[4], &verts[5]);
}

void batching_render_batch_item(primitive_batch_item *item)
{
	if ( item->batch_item_info.selected_render_type == batch_info::VOLUME_EMISSIVE ) { // Cmdline_softparticles
		particle_material material_info;
		render_set_volume_emissive_material(&material_info, item->batch_item_info.texture, !item->triangles);

	} else if ( item->batch_item_info.selected_render_type == batch_info::DISTORTION || item->batch_item_info.selected_render_type == batch_info::DISTORTION_THRUSTER ) {
		distortion_material material_info;

		render_set_distortion_material(&material_info, item->batch_item_info.texture, item->batch_item_info.selected_render_type == batch_info::DISTORTION_THRUSTER)
	} else {
		material material_info;

		render_set_unlit_material(&material_info, item->batch_item_info.texture, true, true);

		gr_render_primitives(&material_info, PRIM_TYPE_TRIS, item->layout, item->offset, item->n_verts, item->buffer_num);
	}
}

void batching_allocate_and_load_buffer(primitive_batch_buffer *draw_queue, bool triangles)
{
	Assert(draw_queue != NULL);

	if ( draw_queue->buffer_size < draw_queue->desired_buffer_size ) {
		if ( draw_queue->buffer_ptr != NULL ) {
			vm_free(draw_queue->buffer_ptr);
		}

		draw_queue->buffer_size = draw_queue->desired_buffer_size;
		draw_queue->buffer_ptr = vm_malloc(draw_queue->desired_buffer_size);
	}

	draw_queue->desired_buffer_size = 0;

	if ( draw_queue->buffer_num >= 0 ) {
		draw_queue->layout->set_base_vertex_ptr(NULL);
	} else {
		draw_queue->layout->set_base_vertex_ptr(draw_queue->buffer_ptr);
	}

	int offset = 0;
	size_t num_items = draw_queue->items.size();

	for ( size_t i = 0; i < num_items; ++i ) {
		primitive_batch_item *item = &draw_queue->items[i];

		item->offset = offset;

		if ( triangles ) {
			item->n_verts = item->batch->load_buffer_triangles((effect_vertex*)draw_queue->buffer_ptr, offset);
		} else {
			item->n_verts = item->batch->load_buffer_points((particle_pnt*)draw_queue->buffer_ptr, offset);
		}
		
		offset += item->n_verts;
	}

	if ( draw_queue->buffer_num >= 0 ) {
		gr_update_buffer_object(draw_queue->buffer_num, draw_queue->buffer_size, draw_queue->buffer_ptr);
	}
}

void batching_load_buffers(bool distortion)
{
	SCP_map<batch_info, primitive_batch>::iterator bi;

	Batch_effect_vertex_queue.desired_buffer_size = 0;
	Batch_effect_vertex_no_radius_queue.desired_buffer_size = 0;
	Batch_point_sprite_queue.desired_buffer_size = 0;

	// assign primitive batch items
	for ( bi = Batching_primitives.begin(); bi != Batching_primitives.end(); ++bi ) {
		if ( bi->first.selected_render_type == batch_info::DISTORTION || bi->first.selected_render_type == batch_info::DISTORTION ) {
			if ( !distortion ) {
				continue;
			}
		} else {
			if ( distortion ) {
				continue;
			}
		}

		int num_tri_verts = bi->second.num_triangle_vertices_to_render();
		int num_point_verts = bi->second.num_points_to_render();

		if ( num_tri_verts > 0 ) {
			primitive_batch_buffer *buffer;

			if ( bi->first.selected_render_type == batch_info::FLAT_EMISSIVE ) {
				buffer = &Batch_effect_vertex_no_radius_queue;
			} else {
				buffer = &Batch_effect_vertex_queue;
			}

			primitive_batch_item draw_item;

			draw_item.batch_item_info = bi->first;
			draw_item.offset = -1;
			draw_item.n_verts = num_tri_verts;
			draw_item.triangles = true;
			draw_item.batch = &bi->second;

			buffer->desired_buffer_size += num_tri_verts * sizeof(effect_vertex);
			buffer->items.push_back(draw_item);
		}

		if ( num_point_verts > 0 ) {
			primitive_batch_buffer *buffer;

			buffer = &Batch_point_sprite_queue;

			primitive_batch_item draw_item;

			draw_item.batch_item_info = bi->first;
			draw_item.offset = -1;
			draw_item.n_verts = num_point_verts;
			draw_item.triangles = false;
			draw_item.batch = &bi->second;

			buffer->desired_buffer_size += num_point_verts * sizeof(particle_pnt);
			buffer->items.push_back(draw_item);
		}
	}

	batching_allocate_and_load_buffer(&Batch_effect_vertex_queue, true);
	batching_allocate_and_load_buffer(&Batch_effect_vertex_no_radius_queue, true);
	batching_allocate_and_load_buffer(&Batch_point_sprite_queue, false);	
}

void batching_render_all(bool distortion)
{
	batching_load_buffers(distortion);

	batching_render_buffer(&Batch_point_sprite_queue, distortion);
	batching_render_buffer(&Batch_effect_vertex_no_radius_queue, distortion);
	batching_render_buffer(&Batch_effect_vertex_queue, distortion);
	
	gr_clear_states();
}

void batching_render_buffer(primitive_batch_buffer *buffer, bool distortion)
{
	size_t num_batches = buffer->items.size();

	for ( int j = 0; j < batch_info::NUM_RENDER_TYPES; ++j ) {
		for ( size_t i = 0; i < num_batches; ++i ) {
			if ( buffer->items[i].batch_item_info.selected_render_type != j ) {
				continue;
			}

			batching_render_batch_item(&buffer->items[i]);
		}
	}
}