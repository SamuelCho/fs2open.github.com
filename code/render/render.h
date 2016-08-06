/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _RENDER_H
#define _RENDER_H

#include "graphics/grinternal.h"
#include "graphics/material.h"

gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool is_transparent);
gr_zbuffer_type render_determine_depth_mode(bool depth_testing, bool is_transparent);

void render_set_unlit_material(material* mat_info, int texture, float alpha, bool blending, bool depth_testing);
void render_set_volume_emissive_material(particle_material* mat_info, int texture, bool point_sprites);
void render_set_distortion_material(distortion_material *mat_info, int texture, bool thruster);

void render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);

void render_rod(color *clr, int num_points, vec3d *pvecs, float width);

void render_laser(int texture, color *clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width);
void render_laser_2d(int texture, color* clr, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len);
void render_laser_2d(int texture, float alpha, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len);

void render_rotated_bitmap(int texture, float alpha, vertex *pnt, float angle, float rad);

void render_oriented_bitmap(int texture, float alpha, vertex *pnt, int orient, float rad, float depth = 0.0f);
void render_oriented_bitmap_2d(int texture, float alpha, vertex *pnt, int orient, float rad);
void render_oriented_bitmap_2d(int texture, vertex *pnt, int orient, float rad);

void render_oriented_quad(int texture, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad(int texture, float alpha, vec3d *pos, vec3d *norm, float width, float height);
void render_oriented_quad_colored(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad_colored(int texture, color *clr, float alpha, vec3d *pos, matrix *ori, float width, float height);

void render_line_3d(color *clr, bool depth_testing, vec3d *start, vec3d *end);
void render_line_3d(bool depth_testing, vec3d *start, vec3d *end);

void render_sphere(color *clr, vec3d* position, float radius);
void render_sphere(vec3d* position, float radius);

void render_shield_icon(color *clr, coord2d coords[6], int resize_mode = GR_RESIZE_FULL);
void render_shield_icon(coord2d coords[6], int resize_mode = GR_RESIZE_FULL);

#endif