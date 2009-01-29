/*
 * RadeonHD R6xx, R7xx DRI driver
 *
 * Copyright (C) 2008-2009  Alexander Deucher
 * Copyright (C) 2008-2009  Matthias Hopf
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __R600_INIT_H__
#define __R600_INIT_H__

/*
 * Hardware setup
 */

void reset_cb (void);
void reset_td_samplers (void);
void reset_sampler_const (adapter_t *adapt);
void reset_dx9_alu_consts (adapter_t *adapt, uint32_t offset, int count);
void reset_bool_loop_const (adapter_t *adapt);

void sq_setup (adapter_t *adapt, sq_config_t *sq_conf);
void set_render_target (adapter_t *adapt, cb_config_t *cb_conf);
void set_depth_target (adapter_t *adapt, db_config_t *db_conf);
void fs_setup (adapter_t *adapt, shader_config_t *fs_conf);
void vs_setup (adapter_t *adapt, shader_config_t *vs_conf);
void ps_setup (adapter_t *adapt, shader_config_t *ps_conf);
void set_alu_consts (int offset, int count, float *const_buf);
void set_vtx_resource (adapter_t *adapt, vtx_resource_t *vtx_res);
void set_tex_resource (adapter_t *adapt, tex_resource_t *tex_res);
void set_tex_sampler (adapter_t *adapt, tex_sampler_t *tex_samp);

void set_viewport(adapter_t *adapt, uint32_t width, uint32_t height, uint32_t depth);

void set_default_state (adapter_t *adapt);

void draw_immd (adapter_t *adapt, draw_config_t *draw_conf, uint32_t *indices);
void draw_auto (adapter_t *adapt, draw_config_t *draw_conf);
void wait_3d_idle (void);
void wait_3d_idle_clean (void);
void start_3d (adapter_t *adapt);

void set_screen_scissor(int x1, int y1, int x2, int y2);
void set_vport_scissor(int id, int x1, int y1, int x2, int y2);
void set_generic_scissor(int x1, int y1, int x2, int y2);
void set_window_scissor(int x1, int y1, int x2, int y2);
void set_clip_rect(int id, int x1, int y1, int x2, int y2);

#define TEX_WIDTH	16
#define TEX_HEIGHT	16
#define TEX_PITCH	32
uint32_t *create_sample_texture (int width, int height, int pitch);

#endif
