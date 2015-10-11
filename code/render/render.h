/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "graphics/grinternal.h"

gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool is_transparent);
gr_zbuffer_type render_determine_depth_mode(bool depth_testing, bool is_transparent);

void render_set_unlit_material(material* mat_info, int texture, float alpha, bool blending, bool depth_testing);
void render_set_unlit_material(material* mat_info, int texture, bool blending, bool depth_testing);
void render_set_volume_emissive_material(particle_material* mat_info, int texture, bool point_sprites);
void render_set_distortion_material(distortion_material *mat_info, int texture, bool thruster);

void render_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, float alpha, bool blending, bool depth_testing);
void render_colored_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, bool blending, bool depth_testing);
void render_colored_primitives(vertex* verts, int n_verts, primitive_type prim_type, int texture, float alpha, bool depth_testing);
void render_rod(int num_points, vec3d *pvecs, float width, color *clr);
void render_laser(vec3d *p0, float width1, vec3d *p1, float width2, color *clr, int texture, float alpha);
void render_laser_2d(vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len, int texture, color* clr, float alpha);
void render_laser_2d(vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len, int texture, float alpha);
void render_oriented_bitmap(int texture, float alpha, vertex *pnt, int orient, float rad, float depth);
void render_oriented_bitmap_2d(vertex *pnt, int orient, float rad, int texture);