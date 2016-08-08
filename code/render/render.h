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

gr_alpha_blend material_determine_blend_mode(int base_bitmap, bool is_transparent);
gr_zbuffer_type material_determine_depth_mode(bool depth_testing, bool is_transparent);

void material_set_interface(material* mat_info, int texture, bool blended, float alpha);
void material_set_unlit(material* mat_info, int texture, float alpha, bool blending, bool depth_testing);
void material_set_unlit_color(material* mat_info, int texture, color *clr, bool blending, bool depth_testing);
void material_set_unlit_color(material* mat_info, int texture, color *clr, float alpha, bool blending, bool depth_testing);
void material_set_unlit_volume(particle_material* mat_info, int texture, bool point_sprites);
void material_set_distortion(distortion_material *mat_info, int texture, bool thruster);

void g3_render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void g3_render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void g3_render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);

void g3_render_rod(color *clr, int num_points, vec3d *pvecs, float width);

void g3_render_laser(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width);
void g3_render_laser_2d(material *mat_params, vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len);

void g3_render_rect_screen_aligned_rotated(material *mat_params, vertex *pnt, float angle, float rad);

void g3_render_rect_screen_aligned(material *mat_params, vertex *pnt, int orient, float rad, float depth = 0.0f);
void g3_render_rect_screen_aligned_2d(material *mat_params, vertex *pnt, int orient, float rad);

void g3_render_rect_oriented(material* mat_info, vec3d *pos, matrix *ori, float width, float height);
void g3_render_rect_oriented(material* mat_info, vec3d *pos, vec3d *norm, float width, float height);

void g3_render_line_3d(color *clr, bool depth_testing, vec3d *start, vec3d *end);
void g3_render_line_3d(bool depth_testing, vec3d *start, vec3d *end);

void g3_render_sphere(color *clr, vec3d* position, float radius);
void g3_render_sphere(vec3d* position, float radius);

void g3_render_shield_icon(color *clr, coord2d coords[6], int resize_mode = GR_RESIZE_FULL);
void g3_render_shield_icon(coord2d coords[6], int resize_mode = GR_RESIZE_FULL);

#endif