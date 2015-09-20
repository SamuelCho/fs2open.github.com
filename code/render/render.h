/*
 * Copyright (C) Freespace Open 2015.  All rights reserved.
 *
 * All source code herein is the property of Freespace Open. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

gr_alpha_blend render_determine_blend_mode(int base_bitmap, bool is_transparent);
gr_zbuffer_type render_determine_depth_mode(bool depth_testing, bool is_transparent);