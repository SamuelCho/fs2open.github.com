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
void render_set_volume_emissive_material(particle_material* mat_info, int texture, bool point_sprites);
void render_set_distortion_material(distortion_material *mat_info, int texture, bool thruster);

void render_primitives_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void render_primitives_colored(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);
void render_primitives_colored_textured(material* mat, vertex* verts, int n_verts, primitive_type prim_type, bool orthographic = false);

void render_rod(color *clr, int num_points, vec3d *pvecs, float width);

void render_laser(vec3d *p0, float width1, vec3d *p1, float width2, color *clr, int texture, float alpha);
void render_laser_2d(vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len, int texture, color* clr, float alpha);
void render_laser_2d(vec3d *headp, float head_width, vec3d *tailp, float tail_width, float max_len, int texture, float alpha);

void render_oriented_bitmap(int texture, float alpha, vertex *pnt, int orient, float rad, float depth);
void render_oriented_bitmap_2d(int texture, vertex *pnt, int orient, float rad);

void render_oriented_quad(int texture, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad(int texture, float alpha, vec3d *pos, vec3d *norm, float width, float height);
void render_oriented_quad_colored(int texture, float alpha, vec3d *pos, matrix *ori, float width, float height);
void render_oriented_quad_colored(int texture, color *clr, float alpha, vec3d *pos, matrix *ori, float width, float height);

void render_string(color *clr, float sx, float sy, const char *s, int resize_mode = GR_RESIZE_FULL);
void render_string(float sx, float sy, const char *s, int resize_mode = GR_RESIZE_FULL);
void render_string(color *clr, int sx, int sy, const char *s, int resize_mode = GR_RESIZE_FULL);
void render_string(int sx, int sy, const char *s, int resize_mode = GR_RESIZE_FULL);

void render_bitmap(int texture, int _x, int _y, int resize_mode = GR_RESIZE_FULL);
void render_bitmap(int texture, float alpha, int _x, int _y, int resize_mode = GR_RESIZE_FULL);
void render_bitmap_ex(int texture, int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL);
void render_aabitmap(int texture, color *clr, int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false);
void render_aabitmap(int texture, int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false);
void render_aabitmap_ex(int texture, color *clr, int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false);
void render_aabitmap_ex(int texture, int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false);

void render_aaline(color *clr, vertex *v1, vertex *v2);
void render_aaline(vertex *v1, vertex *v2);

void render_circle(color *clr, int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL);
void render_circle(int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL);

void render_arc(color *clr, int xc, int yc, float r, float angle_start, float angle_end, bool fill, float linewidth = 1.0f, int resize_mode = GR_RESIZE_FULL);

void render_curve(color *clr, int xc, int yc, int r, int direction, int resize_mode = GR_RESIZE_FULL);
void render_curve(int xc, int yc, int r, int direction, int resize_mode = GR_RESIZE_FULL);

void render_unfilled_circle(color *clr, float linewidth, int xc, int yc, int d, int resize_mode = GR_RESIZE_FULL);

void render_colored_rect(color *clr, int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);
void render_colored_rect(int x, int y, int w, int h, int resize_mode = GR_RESIZE_FULL);

void render_line(int x1,int y1,int x2,int y2, int resize_mode = GR_RESIZE_FULL);

void render_gradient(color *clr, int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);
void render_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);

void render_pline_special(color *clr, SCP_vector<vec3d> *pts, int thickness, int resize_mode = GR_RESIZE_FULL);

void render_sphere_fast(color *clr, vertex *pnt, float rad);
void render_sphere_fast(color *clr, vec3d *pnt, float rad);
void render_sphere_fast(vertex *pnt, float rad);
void render_sphere_fast(vec3d *pnt, float rad);