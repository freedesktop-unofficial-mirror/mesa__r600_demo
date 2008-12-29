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

/*
 * Subsystem state definitions
 */

#ifndef __R600_STATE_H__
#define __R600_STATE_H__

typedef int bool_t;


/* Sequencer / thread handling */
typedef struct {
    int ps_prio;
    int vs_prio;
    int gs_prio;
    int es_prio;
    int num_ps_gprs;
    int num_vs_gprs;
    int num_gs_gprs;
    int num_es_gprs;
    int num_temp_gprs;
    int num_ps_threads;
    int num_vs_threads;
    int num_gs_threads;
    int num_es_threads;
    int num_ps_stack_entries;
    int num_vs_stack_entries;
    int num_gs_stack_entries;
    int num_es_stack_entries;
    int num_qd_pipes;					/* ? does this fit in here */
} sq_config_t;

/* Color buffer / render target */
typedef struct {
    int id;
    int w;
    int h;
    uint64_t base;
    int format;
    int endian;
    int array_mode;						// tiling
    int number_type;
    int read_size;
    int comp_swap;
    int tile_mode;
    int blend_clamp;
    int clear_color;
    int blend_bypass;
    int blend_float32;
    int simple_float;
    int round_mode;
    int tile_compact;
    int source_format;
} cb_config_t;

/* Depth buffer */
typedef struct {
    int w;
    int h;
    uint64_t base;
    int format;
    int read_size;
    int array_mode;						// tiling
    int tile_surface_en;
    int tile_compact;
    int zrange_precision;
} db_config_t;

/* Shader */
typedef struct {
    uint64_t shader_addr;
    int num_gprs;
    int stack_size;
    int dx10_clamp;
    int fetch_cache_lines;
    int clamp_consts;
    int export_mode;
    int uncached_first_inst;
} shader_config_t;

/* Vertex buffer / vtx resource */
typedef struct {
    int id;
    uint64_t vb_addr;
    uint32_t vtx_num_entries;
    uint32_t vtx_size_dw;
    int clamp_x;
    int format;
    int num_format_all;
    int format_comp_all;
    int srf_mode_all;
    int endian;
    int mem_req_size;
} vtx_resource_t;

/* Texture resource */
typedef struct {
    int id;
    int w;
    int h;
    int pitch;
    int depth;
    int dim;
    int tile_mode;
    int tile_type;
    int format;
    uint64_t base;
    uint64_t mip_base;
    int format_comp_x;
    int format_comp_y;
    int format_comp_z;
    int format_comp_w;
    int num_format_all;
    int srf_mode_all;
    int force_degamma;
    int endian;
    int request_size;
    int dst_sel_x;
    int dst_sel_y;
    int dst_sel_z;
    int dst_sel_w;
    int base_level;
    int last_level;
    int base_array;
    int last_array;
    int mpeg_clamp;
    int perf_modulation;
    int interlaced;
} tex_resource_t;

/* Texture sampler */
typedef struct {
    int				id;
    /* Clamping */
    int				clamp_x, clamp_y, clamp_z;
    int		       		border_color;
    /* Filtering */
    int				xy_mag_filter, xy_min_filter;
    int				z_filter;
    int				mip_filter;
    bool_t			high_precision_filter;	/* ? */
    int				perf_mip;		/* ? 0-7 */
    int				perf_z;			/* ? 3 */
    /* LoD selection */
    int				min_lod, max_lod;	/* 0-0x3ff */
    int                         lod_bias;		/* 0-0xfff (signed?) */
    int                         lod_bias2;		/* ? 0-0xfff (signed?) */
    bool_t			lod_uses_minor_axis;	/* ? */
    /* Other stuff */
    bool_t			point_sampling_clamp;	/* ? */
    bool_t			tex_array_override;	/* ? */
    bool_t                      mc_coord_truncate;	/* ? */
    bool_t			force_degamma;		/* ? */
    bool_t			fetch_4;		/* ? */
    bool_t			sample_is_pcf;		/* ? */
    bool_t			type;			/* ? */
    int				depth_compare;		/* only depth textures? */
    int				chroma_key;
} tex_sampler_t;

/* Draw command */
typedef struct {
    uint32_t prim_type;
    uint32_t vgt_draw_initiator;
    uint32_t index_type;
    uint32_t num_instances;
    uint32_t num_indices;
} draw_config_t;

#endif
