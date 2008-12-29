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
 * Hardware setup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"
#include "r600_state.h"


/* Define to clear all constants, samplers, etc.
 * Not necessary for working conditions, but helps debugging. */
#define CLEAN_SETUP 1

/* Define to use SQ_TEX_SAMPLER_WORD instead of SQ_TEX_SAMPLER_*
 * during texture sampler setup. Shouldn't change anything, except
 * for _WORD needing less register accesses. */
#define USE_TEX_SAMPLER_WORD 1

/*
 * Helpers
 */
static inline int count_bits (uint32_t val)
{
    int i, ret = 0;
    for (i = 0; i < 32; i++) {
	ret += val & 1;
	val >>= 1;
    }
    return ret; 
}

/*
 * Initializations cleanups
 */

void reset_cb()
{
    int i;
    CMD_BUFFER_PREAMBLE (8+1);

    PACK0 (CB_COLOR0_INFO, 8);
    for (i = 0; i < 8; i++)
	E32 (0);
}

void reset_td_samplers()
{
    int i;
    CMD_BUFFER_PREAMBLE (2 * (4*TD_PS_SAMPLER0_BORDER_RED_num+1) + 2 * ELEN_WAIT_3D_IDLE);

    EMIT_WAIT_3D_IDLE ();
    PACK0 (TD_PS_SAMPLER0_BORDER_RED, 4*TD_PS_SAMPLER0_BORDER_RED_num);
    for (i = 0; i < 4*TD_PS_SAMPLER0_BORDER_RED_num; i++)
	E32 (0);
    PACK0 (TD_VS_SAMPLER0_BORDER_RED, 4*TD_VS_SAMPLER0_BORDER_RED_num);
    for (i = 0; i < 4*TD_VS_SAMPLER0_BORDER_RED_num; i++)
	E32 (0);
    EMIT_WAIT_3D_IDLE ();
}

void reset_sampler_const ()
{
    int i;
#ifdef USE_TEX_SAMPLER_WORD
    CMD_BUFFER_PREAMBLE (SQ_TEX_SAMPLER_WORD_all_num * 4);

    for (i = 0; i < SQ_TEX_SAMPLER_WORD_all_num; i++) {
	PACK0 (SQ_TEX_SAMPLER_WORD + i * SQ_TEX_SAMPLER_WORD_offset, 3);
	E32   (SQ_TEX_DEPTH_COMPARE_LESSEQUAL << DEPTH_COMPARE_FUNCTION_shift);
	E32   (MAX_LOD_mask);
	E32   (0);
    }
#else
    CMD_BUFFER_PREAMBLE (SQ_TEX_SAMPLER_REGS_all_num * 17);

    for (i = 0; i < SQ_TEX_SAMPLER_REGS_all_num; i++) {
	int j;
	PACK0 (SQ_TEX_SAMPLER_REGS + i * SQ_TEX_SAMPLER_REGS_offset, 16);
	for (j = 0; j < 12; j++)
	    E32   (0);
	E32   (SQ_TEX_SAMPLER_MAX_LOD_0__DATA_mask);
	E32   (SQ_TEX_DEPTH_COMPARE_LESSEQUAL);
	E32   (0);
	E32   (0);
    }
#endif
}

void reset_dx9_alu_consts()
{
    int i;
    const int count = SQ_ALU_CONSTANT_all_num * (SQ_ALU_CONSTANT_offset >> 2);
    CMD_BUFFER_PREAMBLE (count + 1);

    PACK0 (SQ_ALU_CONSTANT, count);
    for (i = 0; i < count; i++)
	EFLOAT (0.0);
}

void reset_bool_loop_const()
{
    int i;
    CMD_BUFFER_PREAMBLE (SQ_BOOL_CONST_0_num * 2 + SQ_LOOP_CONST_all_num + 1);

    for (i = 0; i < SQ_BOOL_CONST_0_num; i++)
	EREG (SQ_BOOL_CONST_0 + (i << 2), 0);
    PACK0 (SQ_LOOP_CONST, SQ_LOOP_CONST_all_num);
    for (i = 0; i < SQ_LOOP_CONST_all_num; i++)
	E32 (0);
}

void start_3d(adapter_t *adapt)
{
    CMD_BUFFER_PREAMBLE (3 + ELEN_WAIT_3D_IDLE_CLEAN);

    if (adapt->chipset <= CHIPSET_RV670) {
	CMD_BUFFER_ALLOC (2);
	PACK3 (IT_START_3D_CMDBUF, 1);
	E32   (0);
    }

    PACK3 (IT_CONTEXT_CONTROL, 2);
    E32   (0x80000000);
    E32   (0x80000000);

    EMIT_WAIT_3D_IDLE_CLEAN ();
}

/*
 * Setup of functional groups
 */

// asic stack/thread/gpr limits - need to query the drm
void sq_setup(adapter_t *adapt, sq_config_t *sq_conf)
{
    uint32_t sq_config, sq_gpr_resource_mgmt_1, sq_gpr_resource_mgmt_2;
    uint32_t sq_thread_resource_mgmt, sq_stack_resource_mgmt_1, sq_stack_resource_mgmt_2;
    CMD_BUFFER_PREAMBLE (7 + 2*2);

    if ((adapt->chipset == CHIPSET_RV610) ||
	(adapt->chipset == CHIPSET_RV620) ||
	(adapt->chipset == CHIPSET_RS780) ||
	(adapt->chipset == CHIPSET_RV710))
	sq_config = 0;						// no VC
    else
	sq_config = VC_ENABLE_bit;

    sq_config |= (DX9_CONSTS_bit |
		  ALU_INST_PREFER_VECTOR_bit |
		  (sq_conf->ps_prio << PS_PRIO_shift) |
		  (sq_conf->vs_prio << VS_PRIO_shift) |
		  (sq_conf->gs_prio << GS_PRIO_shift) |
		  (sq_conf->es_prio << ES_PRIO_shift));

    sq_gpr_resource_mgmt_1 = ((sq_conf->num_ps_gprs << NUM_PS_GPRS_shift) |
			      (sq_conf->num_vs_gprs << NUM_VS_GPRS_shift) |
			      (sq_conf->num_temp_gprs << NUM_CLAUSE_TEMP_GPRS_shift));
    sq_gpr_resource_mgmt_2 = ((sq_conf->num_gs_gprs << NUM_GS_GPRS_shift) |
			      (sq_conf->num_es_gprs << NUM_ES_GPRS_shift));

    sq_thread_resource_mgmt = ((sq_conf->num_ps_threads << NUM_PS_THREADS_shift) |
			       (sq_conf->num_vs_threads << NUM_VS_THREADS_shift) |
			       (sq_conf->num_gs_threads << NUM_GS_THREADS_shift) |
			       (sq_conf->num_es_threads << NUM_ES_THREADS_shift));

    sq_stack_resource_mgmt_1 = ((sq_conf->num_ps_stack_entries << NUM_PS_STACK_ENTRIES_shift) |
				(sq_conf->num_vs_stack_entries << NUM_VS_STACK_ENTRIES_shift));

    sq_stack_resource_mgmt_2 = ((sq_conf->num_gs_stack_entries << NUM_GS_STACK_ENTRIES_shift) |
				(sq_conf->num_es_stack_entries << NUM_ES_STACK_ENTRIES_shift));

    PACK0 (SQ_CONFIG, 6);
    E32   (sq_config);
    E32   (sq_gpr_resource_mgmt_1);
    E32   (sq_gpr_resource_mgmt_2);
    E32   (sq_thread_resource_mgmt);
    E32   (sq_stack_resource_mgmt_1);
    E32   (sq_stack_resource_mgmt_2);

    /* These regs live in VGT space, but should be programmed according to
     * the number of available qd pipes, never to be touched again */
    // XXX: move to drm
    EREG  (VGT_OUT_DEALLOC_CNTL,            sq_conf->num_qd_pipes*4);
    EREG  (VGT_VERTEX_REUSE_BLOCK_CNTL,     sq_conf->num_qd_pipes*4 - 2);
}

void set_render_target(adapter_t *adapt, cb_config_t *cb_conf)
{
    uint32_t cb_color_info;
    CMD_BUFFER_PREAMBLE (7*2);

    cb_color_info = ((cb_conf->endian      << ENDIAN_shift)				|
		     (cb_conf->format      << CB_COLOR0_INFO__FORMAT_shift)		|
		     (cb_conf->array_mode  << CB_COLOR0_INFO__ARRAY_MODE_shift)		|
		     (cb_conf->number_type << NUMBER_TYPE_shift)			|
		     (cb_conf->comp_swap   << COMP_SWAP_shift)				|
		     (cb_conf->tile_mode   << CB_COLOR0_INFO__TILE_MODE_shift));
    if (cb_conf->read_size)
	cb_color_info |= CB_COLOR0_INFO__READ_SIZE_bit;
    if (cb_conf->blend_clamp)
	cb_color_info |= BLEND_CLAMP_bit;
    if (cb_conf->clear_color)
	cb_color_info |= CLEAR_COLOR_bit;
    if (cb_conf->blend_bypass)
	cb_color_info |= BLEND_BYPASS_bit;
    if (cb_conf->blend_float32)
	cb_color_info |= BLEND_FLOAT32_bit;
    if (cb_conf->simple_float)
	cb_color_info |= SIMPLE_FLOAT_bit;
    if (cb_conf->round_mode)
	cb_color_info |= CB_COLOR0_INFO__ROUND_MODE_bit;
    if (cb_conf->tile_compact)
	cb_color_info |= TILE_COMPACT_bit;
    if (cb_conf->source_format)
	cb_color_info |= SOURCE_FORMAT_bit;

    EREG ((CB_COLOR0_BASE + (4 * cb_conf->id)), (cb_conf->base >> 8));
    // rv6xx workaround
    if ((adapt->chipset > CHIPSET_R600) &&
	(adapt->chipset < CHIPSET_RV770)) {
	CMD_BUFFER_ALLOC (2);
	PACK3 (IT_SURFACE_BASE_UPDATE, 1);
	E32   ((2 << cb_conf->id));
    }
    // pitch only for ARRAY_LINEAR_GENERAL, other tiling modes require addrlib
    EREG ((CB_COLOR0_SIZE + (4 * cb_conf->id)), ((((cb_conf->w / 8) - 1) << PITCH_TILE_MAX_shift)	|
						 ((((cb_conf->w * cb_conf->h)/64)-1) << SLICE_TILE_MAX_shift)));
    EREG ((CB_COLOR0_VIEW + (4 * cb_conf->id)), ((0    << SLICE_START_shift)		|
						 (0    << SLICE_MAX_shift)));
    EREG ((CB_COLOR0_INFO + (4 * cb_conf->id)), cb_color_info);
    EREG ((CB_COLOR0_TILE + (4 * cb_conf->id)), (0     >> 8));	// CMASK per-tile data base/256
    EREG ((CB_COLOR0_FRAG + (4 * cb_conf->id)), (0     >> 8));	// FMASK per-tile data base/256
    EREG ((CB_COLOR0_MASK + (4 * cb_conf->id)), ((0    << CMASK_BLOCK_MAX_shift)	|
						 (0    << FMASK_TILE_MAX_shift)));
}

void set_depth_target(adapter_t *adapt, db_config_t *db_conf)
{
    uint32_t db_depth_info;
    CMD_BUFFER_PREAMBLE (4*2);

    db_depth_info = ((db_conf->format << DB_DEPTH_INFO__FORMAT_shift) |
		     (db_conf->array_mode << DB_DEPTH_INFO__ARRAY_MODE_shift));
    if (db_conf->read_size)
	db_depth_info |= DB_DEPTH_INFO__READ_SIZE_bit;
    if (db_conf->tile_surface_en)
	db_depth_info |= TILE_SURFACE_ENABLE_bit;
    if (db_conf->tile_compact)
	db_depth_info |= TILE_COMPACT_bit;
    if (db_conf->zrange_precision)
	db_depth_info |= ZRANGE_PRECISION_bit;

    EREG  (DB_DEPTH_BASE, (db_conf->base >> 8));
    // rv6xx workaround
    if ((adapt->chipset > CHIPSET_R600) &&
	(adapt->chipset < CHIPSET_RV770)) {
	CMD_BUFFER_ALLOC (2);
	PACK3 (IT_SURFACE_BASE_UPDATE, 1);
	E32   (DEPTH_BASE);
    }
    EREG  (DB_DEPTH_INFO, db_depth_info);
    EREG  (DB_DEPTH_SIZE, ((((db_conf->w / 8) - 1) << PITCH_TILE_MAX_shift) |
			   (((db_conf->w * db_conf->h) - 1) << SLICE_TILE_MAX_shift)));
    EREG  (DB_DEPTH_VIEW, ((0 << SLICE_START_shift) |
			   (0 << SLICE_MAX_shift)));
}

void cp_set_surface_sync()
{
    CMD_BUFFER_PREAMBLE (4*2 + 7 + 2 + 2);

    EREG  (CP_COHER_CNTL,                       0x19800000);
    EREG  (CP_COHER_SIZE,                       0xFFFFFFFF);
    EREG  (CP_COHER_BASE,                       0x00000000);
    PACK3 (IT_WAIT_REG_MEM, 6);
    E32   (0x00000003);						// ME, Register, EqualTo
    E32   (CP_COHER_STATUS >> 2);
    E32   (0);
    E32   (0);							// Ref value
    E32   (STATUS_bit);						// Ref mask
    E32   (10);							// Wait interval
    PACK3 (IT_EVENT_WRITE, 1);
    E32   (PIPELINESTAT_STOP);
    PACK3 (IT_EVENT_WRITE, 1);
    E32   (PERFCOUNTER_STOP);
}

void fs_setup(adapter_t *adapt, shader_config_t *fs_conf)
{
    uint32_t sq_pgm_resources;
    sq_pgm_resources = ((fs_conf->num_gprs << NUM_GPRS_shift) |
			(fs_conf->stack_size << STACK_SIZE_shift));
    CMD_BUFFER_PREAMBLE (6);

    if (fs_conf->dx10_clamp)
	sq_pgm_resources |= SQ_PGM_RESOURCES_FS__DX10_CLAMP_bit;

    EREG (SQ_PGM_START_FS, fs_conf->shader_addr >> 8);
    EREG (SQ_PGM_RESOURCES_FS, sq_pgm_resources);
    EREG (SQ_PGM_CF_OFFSET_FS, 0);
}

void vs_setup(adapter_t *adapt, shader_config_t *vs_conf)
{
    uint32_t sq_pgm_resources;
    sq_pgm_resources = ((vs_conf->num_gprs << NUM_GPRS_shift) |
			(vs_conf->stack_size << STACK_SIZE_shift));
    CMD_BUFFER_PREAMBLE (6);

    if (vs_conf->dx10_clamp)
	sq_pgm_resources |= SQ_PGM_RESOURCES_VS__DX10_CLAMP_bit;
    if (vs_conf->fetch_cache_lines)
	sq_pgm_resources |= (vs_conf->fetch_cache_lines << FETCH_CACHE_LINES_shift);
    if (vs_conf->uncached_first_inst)
	sq_pgm_resources |= UNCACHED_FIRST_INST_bit;

    EREG (SQ_PGM_START_VS, vs_conf->shader_addr >> 8);
    EREG (SQ_PGM_RESOURCES_VS, sq_pgm_resources);
    EREG (SQ_PGM_CF_OFFSET_VS, 0);
}

void ps_setup(adapter_t *adapt, shader_config_t *ps_conf)
{
    uint32_t sq_pgm_resources;
    sq_pgm_resources = ((ps_conf->num_gprs << NUM_GPRS_shift) |
			(ps_conf->stack_size << STACK_SIZE_shift));
    CMD_BUFFER_PREAMBLE (8);

    if (ps_conf->dx10_clamp)
	sq_pgm_resources |= SQ_PGM_RESOURCES_PS__DX10_CLAMP_bit;
    if (ps_conf->fetch_cache_lines)
	sq_pgm_resources |= (ps_conf->fetch_cache_lines << FETCH_CACHE_LINES_shift);
    if (ps_conf->uncached_first_inst)
	sq_pgm_resources |= UNCACHED_FIRST_INST_bit;
    if (ps_conf->clamp_consts)
	sq_pgm_resources |= CLAMP_CONSTS_bit;

    EREG (SQ_PGM_START_PS, ps_conf->shader_addr >> 8);
    EREG (SQ_PGM_RESOURCES_PS, sq_pgm_resources);
    EREG (SQ_PGM_EXPORTS_PS, ps_conf->export_mode);
    EREG (SQ_PGM_CF_OFFSET_PS, 0);
}

void set_alu_consts(int offset, int count, float *const_buf)
{
    int i;
    const int countreg = count * (SQ_ALU_CONSTANT_offset >> 2);
    CMD_BUFFER_PREAMBLE (countreg + 1);

    PACK0 (SQ_ALU_CONSTANT + offset * SQ_ALU_CONSTANT_offset, countreg);
    for (i = 0; i < countreg; i++)
	EFLOAT (const_buf[i]);
}

void set_vtx_resource(adapter_t *adapt, vtx_resource_t *res)
{
    uint32_t sq_vtx_constant_word2;
    CMD_BUFFER_PREAMBLE (8);

    sq_vtx_constant_word2 = ((((res->vb_addr) >> 32) & BASE_ADDRESS_HI_mask) |
			     ((res->vtx_size_dw << 2) << SQ_VTX_CONSTANT_WORD2_0__STRIDE_shift) |
			     (res->format << SQ_VTX_CONSTANT_WORD2_0__DATA_FORMAT_shift) |
			     (res->num_format_all << SQ_VTX_CONSTANT_WORD2_0__NUM_FORMAT_ALL_shift) |
			     (res->endian << SQ_VTX_CONSTANT_WORD2_0__ENDIAN_SWAP_shift));
    if (res->clamp_x)
	    sq_vtx_constant_word2 |= SQ_VTX_CONSTANT_WORD2_0__CLAMP_X_bit;

    if (res->format_comp_all)
	    sq_vtx_constant_word2 |= SQ_VTX_CONSTANT_WORD2_0__FORMAT_COMP_ALL_bit;

    if (res->srf_mode_all)
	    sq_vtx_constant_word2 |= SQ_VTX_CONSTANT_WORD2_0__SRF_MODE_ALL_bit;

    PACK0 (SQ_VTX_RESOURCE + res->id * SQ_VTX_RESOURCE_offset, 7);

    E32 (res->vb_addr & 0xffffffff);				// 0: BASE_ADDRESS
    E32 ((res->vtx_num_entries << 2) - 1);			// 1: SIZE
    E32 (sq_vtx_constant_word2);	// 2: BASE_HI, STRIDE, CLAMP, FORMAT, ENDIAN
    E32 (res->mem_req_size << MEM_REQUEST_SIZE_shift);		// 3: MEM_REQUEST_SIZE ?!?
    E32 (0);							// 4: n/a
    E32 (0);							// 5: n/a
    E32 (SQ_TEX_VTX_VALID_BUFFER << SQ_VTX_CONSTANT_WORD6_0__TYPE_shift);	// 6: TYPE
}

void set_tex_resource(adapter_t *adapt, tex_resource_t *tex_res)
{
    uint32_t sq_tex_resource_word0, sq_tex_resource_word1, sq_tex_resource_word4;
    uint32_t sq_tex_resource_word5, sq_tex_resource_word6;
    CMD_BUFFER_PREAMBLE (8);

    sq_tex_resource_word0 = ((tex_res->dim << DIM_shift) |
			     (tex_res->tile_mode << SQ_TEX_RESOURCE_WORD0_0__TILE_MODE_shift));

    if (tex_res->w)
	sq_tex_resource_word0 |= (((((tex_res->pitch + 7) >> 3) - 1) << PITCH_shift) |
				  ((tex_res->w - 1) << TEX_WIDTH_shift));

    if (tex_res->tile_type)
	sq_tex_resource_word0 |= TILE_TYPE_bit;

    sq_tex_resource_word1 = (tex_res->format << SQ_TEX_RESOURCE_WORD1_0__DATA_FORMAT_shift);

    if (tex_res->h)
	sq_tex_resource_word1 |= ((tex_res->h - 1) << TEX_HEIGHT_shift);
    if (tex_res->depth)
	sq_tex_resource_word1 |= ((tex_res->depth - 1) << TEX_DEPTH_shift);

    sq_tex_resource_word4 = ((tex_res->format_comp_x << FORMAT_COMP_X_shift) |
			     (tex_res->format_comp_y << FORMAT_COMP_Y_shift) |
			     (tex_res->format_comp_z << FORMAT_COMP_Z_shift) |
			     (tex_res->format_comp_w << FORMAT_COMP_W_shift) |
			     (tex_res->num_format_all << SQ_TEX_RESOURCE_WORD4_0__NUM_FORMAT_ALL_shift) |
			     (tex_res->endian << SQ_TEX_RESOURCE_WORD4_0__ENDIAN_SWAP_shift) |
			     (tex_res->request_size << REQUEST_SIZE_shift) |
			     (tex_res->dst_sel_x << SQ_TEX_RESOURCE_WORD4_0__DST_SEL_X_shift) |
			     (tex_res->dst_sel_y << SQ_TEX_RESOURCE_WORD4_0__DST_SEL_Y_shift) |
			     (tex_res->dst_sel_z << SQ_TEX_RESOURCE_WORD4_0__DST_SEL_Z_shift) |
			     (tex_res->dst_sel_w << SQ_TEX_RESOURCE_WORD4_0__DST_SEL_W_shift) |
			     (tex_res->base_level << BASE_LEVEL_shift));

    if (tex_res->srf_mode_all)
	sq_tex_resource_word4 |= SQ_TEX_RESOURCE_WORD4_0__SRF_MODE_ALL_bit;
    if (tex_res->force_degamma)
	sq_tex_resource_word4 |= SQ_TEX_RESOURCE_WORD4_0__FORCE_DEGAMMA_bit;

    sq_tex_resource_word5 = ((tex_res->last_level << LAST_LEVEL_shift) |
			     (tex_res->base_array << BASE_ARRAY_shift) |
			     (tex_res->last_array << LAST_ARRAY_shift));

    sq_tex_resource_word6 = ((tex_res->mpeg_clamp << MPEG_CLAMP_shift) |
			     (tex_res->perf_modulation << PERF_MODULATION_shift) |
			     (SQ_TEX_VTX_VALID_TEXTURE << SQ_TEX_RESOURCE_WORD6_0__TYPE_shift));

    if (tex_res->interlaced)
	sq_tex_resource_word6 |= INTERLACED_bit;

    PACK0 (SQ_TEX_RESOURCE + tex_res->id * SQ_TEX_RESOURCE_offset, 7);
    E32   (sq_tex_resource_word0);
    E32   (sq_tex_resource_word1);
    E32   (((tex_res->base) >> 8));
    E32   (((tex_res->mip_base) >> 8));
    E32   (sq_tex_resource_word4);
    E32   (sq_tex_resource_word5);
    E32   (sq_tex_resource_word6);
}

void set_tex_sampler (adapter_t *adapt, tex_sampler_t *s)
{
    uint32_t sq_tex_sampler_word0, sq_tex_sampler_word1, sq_tex_sampler_word2;
    CMD_BUFFER_PREAMBLE (4);

    sq_tex_sampler_word0 = ((s->clamp_x       << SQ_TEX_SAMPLER_WORD0_0__CLAMP_X_shift)		|
			    (s->clamp_y       << CLAMP_Y_shift)					|
			    (s->clamp_z       << CLAMP_Z_shift)					|
			    (s->xy_mag_filter << XY_MAG_FILTER_shift)				|
			    (s->xy_min_filter << XY_MIN_FILTER_shift)				|
			    (s->z_filter      << Z_FILTER_shift)	                        |
			    (s->mip_filter    << MIP_FILTER_shift)				|
			    (s->border_color  << BORDER_COLOR_TYPE_shift)			|
			    (s->depth_compare << DEPTH_COMPARE_FUNCTION_shift)			|
			    (s->chroma_key    << CHROMA_KEY_shift));
    if (s->point_sampling_clamp)
	sq_tex_sampler_word0 |= POINT_SAMPLING_CLAMP_bit;
    if (s->tex_array_override)
	sq_tex_sampler_word0 |= TEX_ARRAY_OVERRIDE_bit;
    if (s->lod_uses_minor_axis)
	sq_tex_sampler_word0 |= LOD_USES_MINOR_AXIS_bit;

    sq_tex_sampler_word1 = ((s->min_lod       << MIN_LOD_shift)					|
			    (s->max_lod       << MAX_LOD_shift)					|
			    (s->lod_bias      << SQ_TEX_SAMPLER_WORD1_0__LOD_BIAS_shift));

    sq_tex_sampler_word2 = ((s->lod_bias2     << LOD_BIAS_SEC_shift)	|
			    (s->perf_mip      << PERF_MIP_shift)	|
			    (s->perf_z        << PERF_Z_shift));
    if (s->mc_coord_truncate)
	sq_tex_sampler_word2 |= MC_COORD_TRUNCATE_bit;
    if (s->force_degamma)
	sq_tex_sampler_word2 |= SQ_TEX_SAMPLER_WORD2_0__FORCE_DEGAMMA_bit;
    if (s->high_precision_filter)
	sq_tex_sampler_word2 |= HIGH_PRECISION_FILTER_bit;
    if (s->fetch_4)
	sq_tex_sampler_word2 |= FETCH_4_bit;
    if (s->sample_is_pcf)
	sq_tex_sampler_word2 |= SAMPLE_IS_PCF_bit;
    if (s->type)
	sq_tex_sampler_word2 |= SQ_TEX_SAMPLER_WORD2_0__TYPE_bit;

    PACK0 (SQ_TEX_SAMPLER_WORD + s->id * SQ_TEX_SAMPLER_WORD_offset, 3);
    E32   (sq_tex_sampler_word0);
    E32   (sq_tex_sampler_word1);
    E32   (sq_tex_sampler_word2);
}

//XXX deal with clip offsets in clip setup

void set_screen_scissor(int x1, int y1, int x2, int y2)
{
    CMD_BUFFER_PREAMBLE (2 * 2);
    
    EREG  (PA_SC_SCREEN_SCISSOR_TL,              ((x1 << PA_SC_SCREEN_SCISSOR_TL__TL_X_shift) |
						  (y1 << PA_SC_SCREEN_SCISSOR_TL__TL_Y_shift)));
    EREG  (PA_SC_SCREEN_SCISSOR_BR,              ((x2 << PA_SC_SCREEN_SCISSOR_BR__BR_X_shift) |
						  (y2 << PA_SC_SCREEN_SCISSOR_BR__BR_Y_shift)));
}

void set_vport_scissor(int id, int x1, int y1, int x2, int y2)
{
    CMD_BUFFER_PREAMBLE (2 * 2);

    EREG  (PA_SC_VPORT_SCISSOR_0_TL +
	   id * PA_SC_VPORT_SCISSOR_0_TL_offset, ((x1 << PA_SC_VPORT_SCISSOR_0_TL__TL_X_shift) |
						  (y1 << PA_SC_VPORT_SCISSOR_0_TL__TL_Y_shift) |
						  WINDOW_OFFSET_DISABLE_bit));
    EREG  (PA_SC_VPORT_SCISSOR_0_BR +
	   id * PA_SC_VPORT_SCISSOR_0_BR_offset, ((x2 << PA_SC_VPORT_SCISSOR_0_BR__BR_X_shift) |
						  (y2 << PA_SC_VPORT_SCISSOR_0_BR__BR_Y_shift)));
}

void set_generic_scissor(int x1, int y1, int x2, int y2)
{
    CMD_BUFFER_PREAMBLE (2 * 2);

    EREG  (PA_SC_GENERIC_SCISSOR_TL,            ((x1 << PA_SC_GENERIC_SCISSOR_TL__TL_X_shift) |
						 (y1 << PA_SC_GENERIC_SCISSOR_TL__TL_Y_shift) |
						 WINDOW_OFFSET_DISABLE_bit));
    EREG  (PA_SC_GENERIC_SCISSOR_BR,            ((x2 << PA_SC_GENERIC_SCISSOR_BR__BR_X_shift) |
						 (y2 << PA_SC_GENERIC_SCISSOR_TL__TL_Y_shift)));
}

void set_window_scissor(int x1, int y1, int x2, int y2)
{
    CMD_BUFFER_PREAMBLE (2 * 2);

    EREG  (PA_SC_WINDOW_SCISSOR_TL,             ((x1 << PA_SC_WINDOW_SCISSOR_TL__TL_X_shift) |
						 (y1 << PA_SC_WINDOW_SCISSOR_TL__TL_Y_shift) |
						 WINDOW_OFFSET_DISABLE_bit));
    EREG  (PA_SC_WINDOW_SCISSOR_BR,             ((x2 << PA_SC_WINDOW_SCISSOR_BR__BR_X_shift) |
						 (y2 << PA_SC_WINDOW_SCISSOR_BR__BR_Y_shift)));
}

void set_clip_rect(int id, int x1, int y1, int x2, int y2)
{
    CMD_BUFFER_PREAMBLE (2 * 2);

    EREG  (PA_SC_CLIPRECT_0_TL +
	   id * PA_SC_CLIPRECT_0_TL_offset,     ((x1 << PA_SC_CLIPRECT_0_TL__TL_X_shift) |
						 (y1 << PA_SC_CLIPRECT_0_TL__TL_Y_shift)));
    EREG  (PA_SC_CLIPRECT_0_BR +
	   id * PA_SC_CLIPRECT_0_BR_offset,     ((x2 << PA_SC_CLIPRECT_0_BR__BR_X_shift) |
						 (y2 << PA_SC_CLIPRECT_0_BR__BR_Y_shift)));
}

/*
 * Setup of default state
 */

void set_default_state(adapter_t *adapt)
{
    tex_resource_t tex_res;
    shader_config_t fs_conf;
    sq_config_t sq_conf;
    int i;
    uint32_t reg;
    CMD_BUFFER_PREAMBLE (ELEN_WAIT_3D_IDLE);

    memset(&tex_res, 0, sizeof(tex_resource_t));
    memset(&fs_conf, 0, sizeof(shader_config_t));

    EMIT_WAIT_3D_IDLE();

    // ASIC specific setup, see drm
    CMD_BUFFER_ALLOC (5*2);
    if (adapt->chipset <= CHIPSET_RV670) {
	EREG  (TA_CNTL_AUX,                     (( 3 << GRADIENT_CREDIT_shift)		|
						 (28 << TD_FIFO_CREDIT_shift)));
	EREG  (VC_ENHANCE,                      0);
	EREG  (R7xx_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, 0);
	EREG  (DB_DEBUG,                        0x82000000); /* ? */
	EREG  (DB_WATERMARKS,		        ((4  << DEPTH_FREE_shift)		|
						 (16 << DEPTH_FLUSH_shift)		|
						 (0  << FORCE_SUMMARIZE_shift)		|
						 (4  << DEPTH_PENDING_FREE_shift)	|
						 (16 << DEPTH_CACHELINE_FREE_shift)	|
						 0));
    } else {
	EREG (TA_CNTL_AUX,                      (( 2 << GRADIENT_CREDIT_shift)		|
						 (28 << TD_FIFO_CREDIT_shift)));
	EREG (VC_ENHANCE,                       0);
	EREG (R7xx_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ, VS_PC_LIMIT_ENABLE_bit);
	EREG (DB_DEBUG,                         0);
	EREG (DB_WATERMARKS,                    ((4  << DEPTH_FREE_shift)		|
						 (16 << DEPTH_FLUSH_shift)		|
						 (0  << FORCE_SUMMARIZE_shift)		|
						 (4  << DEPTH_PENDING_FREE_shift)	|
						 (4  << DEPTH_CACHELINE_FREE_shift)	|
						 0));
    }

#ifdef CLEAN_SETUP
    reset_td_samplers();
    reset_dx9_alu_consts();
    reset_bool_loop_const ();
    reset_sampler_const ();
#endif

    // SQ
    sq_conf.ps_prio = 0;
    sq_conf.vs_prio = 1;
    sq_conf.gs_prio = 2;
    sq_conf.es_prio = 3;
    // need to set stack/thread/gpr limits based on the asic
    // for now just set them low enough so any card will work
    // see r600_cp.c in the drm
    // maybe add a GET_PARAM for these to the drm
    if (adapt->chipset <= CHIPSET_RV670) {
	    sq_conf.num_ps_gprs = 100;
	    sq_conf.num_vs_gprs = 24;
	    sq_conf.num_temp_gprs = 4;
	    sq_conf.num_gs_gprs = 0;
	    sq_conf.num_es_gprs = 0;
	    sq_conf.num_ps_threads = 136;
	    sq_conf.num_vs_threads = 48;
	    sq_conf.num_gs_threads = 4;
	    sq_conf.num_es_threads = 4;
	    sq_conf.num_ps_stack_entries = 40;
	    sq_conf.num_vs_stack_entries = 40;
	    sq_conf.num_gs_stack_entries = 0;
	    sq_conf.num_es_stack_entries = 0;
    } else {
	    sq_conf.num_ps_gprs = 100;
	    sq_conf.num_vs_gprs = 24;
	    sq_conf.num_temp_gprs = 4;
	    sq_conf.num_gs_gprs = 0;
	    sq_conf.num_es_gprs = 0;
	    sq_conf.num_ps_threads = 136;
	    sq_conf.num_vs_threads = 48;
	    sq_conf.num_gs_threads = 0;
	    sq_conf.num_es_threads = 0;
	    sq_conf.num_ps_stack_entries = 128;
	    sq_conf.num_vs_stack_entries = 128;
	    sq_conf.num_gs_stack_entries = 0;
	    sq_conf.num_es_stack_entries = 0;
    }

    // XXX: move to drm
    reg = reg_read32 (CC_GC_SHADER_PIPE_CONFIG);
    if (adapt->chipset <= CHIPSET_RV770)
	sq_conf.num_qd_pipes = R6XX_MAX_QD_PIPES - count_bits (reg & INACTIVE_QD_PIPES_mask);
    else
	sq_conf.num_qd_pipes = 4;				/* TODO: Verfiy (This is from TCore) */

    sq_setup(adapt, &sq_conf);

    CMD_BUFFER_ALLOC (2*2 + 10 + 9*2 + 2*2);

    EREG  (SQ_VTX_BASE_VTX_LOC,                 0);
    EREG  (SQ_VTX_START_INST_LOC,               0);

    PACK0 (SQ_ESGS_RING_ITEMSIZE, 9);
    E32   (0);							// SQ_ESGS_RING_ITEMSIZE
    E32   (0);							// SQ_GSVS_RING_ITEMSIZE
    E32   (0);							// SQ_ESTMP_RING_ITEMSIZE
    E32   (0);							// SQ_GSTMP_RING_ITEMSIZE
    E32   (0);							// SQ_VSTMP_RING_ITEMSIZE
    E32   (0);							// SQ_PSTMP_RING_ITEMSIZE
    E32   (0);							// SQ_FBUF_RING_ITEMSIZE
    E32   (0);							// SQ_REDUC_RING_ITEMSIZE
    E32   (0);							// SQ_GS_VERT_ITEMSIZE

    // DB
    EREG  (DB_DEPTH_INFO,                       0);
    EREG  (DB_STENCIL_CLEAR,                    0);
    EREG  (DB_DEPTH_CLEAR,                      0);
    EREG  (DB_STENCILREFMASK,                   0);
    EREG  (DB_STENCILREFMASK_BF,                0);
    EREG  (DB_DEPTH_CONTROL,                    0);
    EREG  (DB_RENDER_CONTROL,                   STENCIL_COMPRESS_DISABLE_bit | DEPTH_COMPRESS_DISABLE_bit);
    if (adapt->chipset <= CHIPSET_RV670)
	EREG  (DB_RENDER_OVERRIDE,              FORCE_SHADER_Z_ORDER_bit);
    else
	EREG  (DB_RENDER_OVERRIDE,              0);
    EREG  (DB_ALPHA_TO_MASK,                    ((2 << ALPHA_TO_MASK_OFFSET0_shift)	|
						 (2 << ALPHA_TO_MASK_OFFSET1_shift)	|
						 (2 << ALPHA_TO_MASK_OFFSET2_shift)	|
						 (2 << ALPHA_TO_MASK_OFFSET3_shift)));

    // SX
    EREG  (SX_ALPHA_TEST_CONTROL,               0);
    EREG  (SX_ALPHA_REF,                        0);

    // CB
    reset_cb();

    CMD_BUFFER_ALLOC (5 + CB_BLEND0_CONTROL_num+1 + 2);

    PACK0 (CB_BLEND_RED, 4);
    E32   (0x00000000);
    E32   (0x00000000);
    E32   (0x00000000);
    E32   (0x00000000);

#if CLEAN_SETUP
    /* CB_COLOR_CONTROL.PER_MRT_BLEND is off */
    // RV6xx+ have per-MRT blend
    if (adapt->chipset > CHIPSET_R600) {
	PACK0 (CB_BLEND0_CONTROL, CB_BLEND0_CONTROL_num);
	for   (i = 0; i < CB_BLEND0_CONTROL_num; i++)
	    E32 (0);
    }
#endif
    EREG  (CB_BLEND_CONTROL,                    0);

    if (adapt->chipset <= CHIPSET_RV670) {
	CMD_BUFFER_ALLOC (4);
	PACK0 (CB_FOG_RED, 3);
	E32   (0x00000000);
	E32   (0x00000000);
	E32   (0x00000000);
    }
    
    CMD_BUFFER_ALLOC (2 + 5);
    EREG  (CB_COLOR_CONTROL,                    0);
    PACK0 (CB_CLRCMP_CONTROL, 4);
    E32   (1 << CLRCMP_FCN_SEL_shift);				// CB_CLRCMP_CONTROL: use CLRCMP_FCN_SRC
    E32   (0);							// CB_CLRCMP_SRC
    E32   (0);							// CB_CLRCMP_DST
    E32   (0);							// CB_CLRCMP_MSK

    if (adapt->chipset <= CHIPSET_RV670) {
	CMD_BUFFER_ALLOC (5);
	PACK0 (CB_CLEAR_RED, 4);
	EFLOAT(1.0);						/* WTF? */
	EFLOAT(0.0);
	EFLOAT(1.0);
	EFLOAT(1.0);
    }
    CMD_BUFFER_ALLOC (2);
    EREG  (CB_TARGET_MASK,                      (0x0f << TARGET0_ENABLE_shift));

    // SC
    set_generic_scissor(0, 0, 8192, 8192);
    set_screen_scissor (0, 0, 8192, 8192);
    CMD_BUFFER_ALLOC (2);
    EREG  (PA_SC_WINDOW_OFFSET,                 ((0 << WINDOW_X_OFFSET_shift) |
						 (0 << WINDOW_Y_OFFSET_shift)));
    set_window_scissor (0, 0, 8192, 8192);

    CMD_BUFFER_ALLOC (2);
    EREG  (PA_SC_CLIPRECT_RULE,                 CLIP_RULE_mask);

#if CLEAN_SETUP
    /* clip boolean is set to always visible -> doesn't matter */
    for (i = 0; i < PA_SC_CLIPRECT_0_TL_num; i++)
	set_clip_rect (i, 0, 0, 8192, 8192);
#endif

    CMD_BUFFER_ALLOC (2);
    if (adapt->chipset <= CHIPSET_RV670)
	EREG  (R7xx_PA_SC_EDGERULE,             0x00000000);
    else
	EREG  (R7xx_PA_SC_EDGERULE,             0xAAAAAAAA); /* ? */

    for (i = 0; i < PA_SC_VPORT_SCISSOR_0_TL_num; i++) {
	set_vport_scissor (i, 0, 0, 8192, 8192);
	CMD_BUFFER_ALLOC (3);
	PACK0 (PA_SC_VPORT_ZMIN_0 + i * PA_SC_VPORT_ZMIN_0_offset, 2);
	EFLOAT(0.0);
	EFLOAT(1.0);
    }

    CMD_BUFFER_ALLOC (2 + 3*2);

    if (adapt->chipset <= CHIPSET_RV670)
	EREG  (PA_SC_MODE_CNTL,                 (WALK_ORDER_ENABLE_bit | FORCE_EOV_CNTDWN_ENABLE_bit));
    else
	EREG  (PA_SC_MODE_CNTL,                 (FORCE_EOV_CNTDWN_ENABLE_bit | FORCE_EOV_REZ_ENABLE_bit |
						 0x00500000)); /* ? */

    EREG  (PA_SC_LINE_CNTL,                     0);
    EREG  (PA_SC_AA_CONFIG,                     0);
    EREG  (PA_SC_AA_MASK,                       0xFFFFFFFF);

    //XXX: double check this
    if (adapt->chipset > CHIPSET_R600) {
	CMD_BUFFER_ALLOC (2*2);
	EREG  (PA_SC_AA_SAMPLE_LOCS_MCTX,       0);
	EREG  (PA_SC_AA_SAMPLE_LOCS_8S_WD1_M,   0);
    }

    CMD_BUFFER_ALLOC (2*2 + 7 + 4*2 + 5);
    
    EREG  (PA_SC_LINE_STIPPLE,                  0);
    EREG  (PA_SC_MPASS_PS_CNTL,                 0);

    // CL
    PACK0  (PA_CL_VPORT_XSCALE_0, 6);
    EFLOAT (0.0f);						// PA_CL_VPORT_XSCALE
    EFLOAT (0.0f);						// PA_CL_VPORT_XOFFSET
    EFLOAT (0.0f);						// PA_CL_VPORT_YSCALE
    EFLOAT (0.0f);						// PA_CL_VPORT_YOFFSET
    EFLOAT (0.0f);						// PA_CL_VPORT_ZSCALE
    EFLOAT (0.0f);						// PA_CL_VPORT_ZOFFSET
    EREG   (PA_CL_CLIP_CNTL,                     (CLIP_DISABLE_bit | DX_CLIP_SPACE_DEF_bit));
    EREG   (PA_CL_VTE_CNTL,                      0);
    EREG   (PA_CL_VS_OUT_CNTL,                   0);
    EREG   (PA_CL_NANINF_CNTL,                   0);
    PACK0  (PA_CL_GB_VERT_CLIP_ADJ, 4);
    EFLOAT (1.0);						// PA_CL_GB_VERT_CLIP_ADJ
    EFLOAT (1.0);						// PA_CL_GB_VERT_DISC_ADJ
    EFLOAT (1.0);						// PA_CL_GB_HORZ_CLIP_ADJ
    EFLOAT (1.0);						// PA_CL_GB_HORZ_DISC_ADJ

#ifdef CLEAN_SETUP
    CMD_BUFFER_ALLOC (25);
    /* user clipping planes are disabled by default */
    PACK0 (PA_CL_UCP_0_X, 24);
    for (i = 0; i < 24; i++)
	EFLOAT (0.0);
#endif

    CMD_BUFFER_ALLOC (8*2 + 2*2 + 2 + 2 + 5*2 + SPI_VS_OUT_ID_0_num+1 + 2);

    // SU
    EREG  (PA_SU_SC_MODE_CNTL,                  FACE_bit);
    EREG  (PA_SU_POINT_SIZE,                    0);
    EREG  (PA_SU_POINT_MINMAX,                  0);
    EREG  (PA_SU_POLY_OFFSET_DB_FMT_CNTL,       0);
    EREG  (PA_SU_POLY_OFFSET_BACK_SCALE,        0);
    EREG  (PA_SU_POLY_OFFSET_FRONT_SCALE,       0);
    EREG  (PA_SU_POLY_OFFSET_BACK_OFFSET,       0);
    EREG  (PA_SU_POLY_OFFSET_FRONT_OFFSET,      0);

    EREG  (PA_SU_LINE_CNTL,                     (8 << PA_SU_LINE_CNTL__WIDTH_shift)); /* Line width 1 pixel */
    EREG  (PA_SU_VTX_CNTL,                      ((2 << PA_SU_VTX_CNTL__ROUND_MODE_shift) |
						 (5 << QUANT_MODE_shift))); /* Round to Even, fixed point 1/256 */
    EREG  (PA_SU_POLY_OFFSET_CLAMP,             0);

    // SPI
    if (adapt->chipset <= CHIPSET_RV670)
	EREG  (R7xx_SPI_THREAD_GROUPING,        0);
    else
	EREG  (R7xx_SPI_THREAD_GROUPING,        (1 << PS_GROUPING_shift));

    EREG  (SPI_INTERP_CONTROL_0,                ((2 << PNT_SPRITE_OVRD_X_shift)		|
						 (3 << PNT_SPRITE_OVRD_Y_shift)		|
						 (0 << PNT_SPRITE_OVRD_Z_shift)		|
						 (1 << PNT_SPRITE_OVRD_W_shift))); /* s,t,0,1 */
    EREG  (SPI_INPUT_Z,                         0);
    EREG  (SPI_FOG_CNTL,                        0);
    EREG  (SPI_FOG_FUNC_SCALE,                  0);
    EREG  (SPI_FOG_FUNC_BIAS,                   0);

    PACK0 (SPI_VS_OUT_ID_0, SPI_VS_OUT_ID_0_num);
    for (i = 0; i < SPI_VS_OUT_ID_0_num; i++)		/* identity mapping */
	E32 (0x03020100 + i*0x04040404);
    EREG  (SPI_VS_OUT_CONFIG,                   0);

    // clear FS
    fs_setup(adapt, &fs_conf);

    CMD_BUFFER_ALLOC (5*2 + 20*2);

    // VGT
    EREG  (VGT_MAX_VTX_INDX,                    0);
    EREG  (VGT_MIN_VTX_INDX,                    0);
    EREG  (VGT_INDX_OFFSET,                     0);
    EREG  (VGT_INSTANCE_STEP_RATE_0,            0);
    EREG  (VGT_INSTANCE_STEP_RATE_1,            0);

    EREG  (VGT_MULTI_PRIM_IB_RESET_INDX,        0);
    EREG  (VGT_OUTPUT_PATH_CNTL,                0);
    EREG  (VGT_GS_MODE,                         0);
    EREG  (VGT_HOS_CNTL,                        0);
    EREG  (VGT_HOS_MAX_TESS_LEVEL,              0);
    EREG  (VGT_HOS_MIN_TESS_LEVEL,              0);
    EREG  (VGT_HOS_REUSE_DEPTH,                 0);
    EREG  (VGT_GROUP_PRIM_TYPE,                 0);
    EREG  (VGT_GROUP_FIRST_DECR,                0);
    EREG  (VGT_GROUP_DECR,                      0);
    EREG  (VGT_GROUP_VECT_0_CNTL,               0);
    EREG  (VGT_GROUP_VECT_1_CNTL,               0);
    EREG  (VGT_GROUP_VECT_0_FMT_CNTL,           0);
    EREG  (VGT_GROUP_VECT_1_FMT_CNTL,           0);
    EREG  (VGT_PRIMITIVEID_EN,                  0);
    EREG  (VGT_MULTI_PRIM_IB_RESET_EN,          0);
    EREG  (VGT_STRMOUT_EN,                      0);
    EREG  (VGT_REUSE_OFF,                       0);
    EREG  (VGT_VTX_CNT_EN,                      0);
    EREG  (VGT_STRMOUT_BUFFER_EN,               0);

#ifdef CLEAN_SETUP
    // clear tex resources - PS
    for (i = 0; i < 16; i++) {
	tex_res.id = i;
	set_tex_resource(adapt, &tex_res);
    }

    // clear tex resources - VS
    for (i = 160; i < 164; i++) {
	tex_res.id = i;
	set_tex_resource(adapt, &tex_res);
    }

    // clear tex resources - FS
    for (i = 320; i < 335; i++) {
	tex_res.id = i;
	set_tex_resource(adapt, &tex_res);
    }
#endif

}


/*
 * Commands
 */

void draw_immd(adapter_t *adapt, draw_config_t *draw_conf, uint32_t *indices)
{
    uint32_t i, count;
    CMD_BUFFER_PREAMBLE (2 + 2 + 2);

    if (verbose)
	printf ("Drawing %d vertices\n", draw_conf->num_indices);

    EREG  (VGT_PRIMITIVE_TYPE, draw_conf->prim_type);
    PACK3 (IT_INDEX_TYPE, 1);
    E32   (draw_conf->index_type);
    PACK3 (IT_NUM_INSTANCES, 1);
    E32   (draw_conf->num_instances);

    // calculate num of packets
    count = 2;
    if (draw_conf->index_type == DI_INDEX_SIZE_16_BIT)
	count += (draw_conf->num_indices + 1) / 2;
    else
	count += draw_conf->num_indices;

    CMD_BUFFER_ALLOC (count +1);
    PACK3 (IT_DRAW_INDEX_IMMD, count);
    E32   (draw_conf->num_indices);
    E32   (draw_conf->vgt_draw_initiator);

    if (draw_conf->index_type == DI_INDEX_SIZE_16_BIT) {
	for (i = 0; i < draw_conf->num_indices; i += 2) {
	    if ((i + 1) == draw_conf->num_indices)
		E32   (indices[i]);
	    else
		E32   ((indices[i] | (indices[i + 1] << 16)));
	}
    } else {
	for (i = 0; i < draw_conf->num_indices; i++)
	    E32   (indices[i]);
    }

}

void draw_auto(adapter_t *adapt, draw_config_t *draw_conf)
{
    CMD_BUFFER_PREAMBLE (2 + 2 + 2 + 3);

    EREG  (VGT_PRIMITIVE_TYPE, draw_conf->prim_type);
    PACK3 (IT_INDEX_TYPE, 1);
    E32   (draw_conf->index_type);
    PACK3 (IT_NUM_INSTANCES, 1);
    E32   (draw_conf->num_instances);
    PACK3 (IT_DRAW_INDEX_AUTO, 2);
    E32   (draw_conf->num_indices);
    E32   (draw_conf->vgt_draw_initiator);
}

void set_viewport(adapter_t *adapt, uint32_t width, uint32_t height, uint32_t depth)
{
    float xscale, yscale, zscale;
    float xoffset, yoffset, zoffset;
    CMD_BUFFER_PREAMBLE (7);

    xscale  = 0.5f * (float)width;
    xoffset = 0.0f + xscale;
    yscale  = -0.5f * (float)height;
    yoffset = 0.0f - yscale;
    // fixme depth
    zscale  = 1.0f - 0.0f;
    zoffset = 0.0f;

    PACK0  (PA_CL_VPORT_XSCALE_0, 6);
    EFLOAT (xscale);
    EFLOAT (xoffset);
    EFLOAT (yscale);
    EFLOAT (yoffset);
    EFLOAT (zscale);
    EFLOAT (zoffset);
}

