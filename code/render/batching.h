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
#include "graphics/material.h"

struct batch_info {
	enum render_type {
		FLAT_EMISSIVE,
		VOLUME_EMISSIVE,
		DISTORTION,
		DISTORTION_THRUSTER,
		NUM_RENDER_TYPES
	};

	render_type selected_render_type;
	int texture;

	bool operator < (const batch_info& batch) const {
		if ( selected_render_type != batch.selected_render_type ) {
			
			return selected_render_type < batch.selected_render_type;
		}

		return texture < batch.texture;
	}
};

class primitive_batch
{
	batch_info render_info;

	SCP_vector<effect_vertex> Triangle_verts;
	SCP_vector<particle_pnt> Point_sprite_verts;

public:
	void add_triangle(effect_vertex* v0, effect_vertex* v1, effect_vertex *v2);
	void add_point_sprite(particle_pnt *sprite);

	int load_buffer_points(particle_pnt* buffer, int n_verts);
	int load_buffer_triangles(effect_vertex* buffer, int n_verts);

	int num_triangle_vertices_to_render();
	int num_points_to_render();

	void clear();
};

struct primitive_batch_item {
	batch_info batch_item_info;
	vertex_layout *layout;
	bool triangles;
	int buffer_num;
	int offset;
	int n_verts;

	primitive_batch *batch;
};

struct primitive_batch_buffer {
	vertex_layout *layout;
	int buffer_num;

	void* buffer_ptr;
	int buffer_size;

	int desired_buffer_size;

	bool triangles;

	SCP_vector<primitive_batch_item> items;
};

primitive_batch* batching_find_batch(int texture, batch_info::render_type batch_type);
void batching_add_bitmap(primitive_batch *batch, vertex *pnt, int orient, float rad, color *clr, float depth);
void batching_add_laser(primitive_batch *batch, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b);
void batching_add_laser(int texture, vec3d *p0, float width1, vec3d *p1, float width2, int r, int g, int b);

void batching_add_beam(primitive_batch *batch, vec3d *start, vec3d *end, float width, color *clr, float offset);

int batching_add_polygon(primitive_batch *batch, int texture, vec3d *pos, matrix *orient, float width, float height, color *clr);

void batching_render_buffer(primitive_batch_buffer *buffer, bool distortion);