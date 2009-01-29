/*
 * EXA acceleration functions
 *
 * Copyright (C) 2008-2009  Alexander Deucher
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"
#include "r600_state.h"
#include "r600_init.h"
#include "r600_shader.h"

struct r6xx_solid_vertex {
    int32_t x;
    int32_t y;
    uint32_t color;
};

struct r6xx_copy_vertex {
    int32_t x;
    int32_t y;
    int32_t s;
    int32_t t;
};

static void
R600PrepareSolid(adapter_t *adapt, int alu, uint32_t pm, uint32_t fg)
{
    cb_config_t     cb_conf;
    shader_config_t vs_conf, ps_conf;
    uint64_t vs_addr, ps_addr;
    int i = 0;
    uint32_t vs[20];
    uint32_t ps[2];
    uint32_t blendcntl;

    //0
    vs[i++] = CF_DWORD0(ADDR(6));
    vs[i++] = CF_DWORD1(POP_COUNT(0),
			CF_CONST(0),
			COND(SQ_CF_COND_ACTIVE),
			COUNT(2),
			CALL_COUNT(0),
			END_OF_PROGRAM(0),
			VALID_PIXEL_MODE(0),
			CF_INST(SQ_CF_INST_VTX),
			WHOLE_QUAD_MODE(0),
			BARRIER(1));
    //1
    vs[i++] = CF_ALU_DWORD0(ADDR(4),
			    KCACHE_BANK0(0),
			    KCACHE_BANK1(0),
			    KCACHE_MODE0(0));
    vs[i++] = CF_ALU_DWORD1(KCACHE_MODE1(0),
			    KCACHE_ADDR0(0),
			    KCACHE_ADDR1(0),
			    COUNT(2),
			    USES_WATERFALL(0),
			    CF_INST(SQ_CF_INST_ALU),
			    WHOLE_QUAD_MODE(0),
			    BARRIER(1));
    //2
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				      TYPE(SQ_EXPORT_POS),
				      RW_GPR(1),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(0),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));
    //3
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				      TYPE(SQ_EXPORT_PARAM),
				      RW_GPR(2),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(1),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(0));
    //4
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),			/* ALU inst 0 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    //5
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),			/* ALU inst 1 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    //6/7
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(12));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(1),
			     DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_0),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_32_32), //xxx
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_INT), //xxx
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED), //xxx
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(0),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(1));
    vs[i++] = VTX_DWORD_PAD;
    //8/9
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(4));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(2),
			     DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_Z),
			     DST_SEL_W(SQ_SEL_W),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_8_8_8_8),
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(8),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(0));
	    vs[i++] = VTX_DWORD_PAD;

    i = 0;
    ps[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				      TYPE(SQ_EXPORT_PIXEL),
				      RW_GPR(0),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(1));
    ps[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(1),
					   END_OF_PROGRAM(1),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));

    CLEAR (cb_conf);
    CLEAR (vs_conf);
    CLEAR (ps_conf);

    if (verbose) {
	dump_shader (adapt, vs, sizeof(vs), "vertex");
	dump_shader (adapt, ps, sizeof(ps), "pixel");
        printf ("\n");
    }

    /* Init */
    start_3d(adapt);
    set_default_state(adapt);

    /* Scissor / viewport */
    ereg  (PA_CL_VTE_CNTL,                      VTX_XY_FMT_bit);
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);

    // fixme use gart buffer directly
    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    /* Shader */
    vs_conf.shader_addr         = vs_addr;
    vs_conf.num_gprs            = 4;
    vs_conf.stack_size          = 1;
    vs_setup                    (adapt, &vs_conf);

    ps_conf.shader_addr         = ps_addr;
    ps_conf.num_gprs            = 1;
    ps_conf.stack_size          = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.clamp_consts        = 1;
    ps_conf.export_mode         = 2;
    ps_setup                    (adapt, &ps_conf);

    /* Render setup */
    // XXX fix me planemask
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (R7xx_CB_SHADER_CONTROL,              (RT0_ENABLE_bit));
    /* Clear */
    //blendcntl = (BLEND_ZERO << COLOR_SRCBLEND_shift) | (BLEND_ZERO << COLOR_DESTBLEND_shift);
    /* Src */
    //blendcntl = (BLEND_ONE << COLOR_SRCBLEND_shift) | (BLEND_ZERO << COLOR_DESTBLEND_shift);
    /* Dst */
    //blendcntl = (BLEND_ZERO << COLOR_SRCBLEND_shift) | (BLEND_ONE << COLOR_DESTBLEND_shift);
    /* Over */
    blendcntl = (BLEND_ONE << COLOR_SRCBLEND_shift) | (BLEND_ONE_MINUS_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* OverReverse */
    //blendcntl = (BLEND_ONE_MINUS_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_ONE << COLOR_DESTBLEND_shift);
    /* In */
    //blendcntl = (BLEND_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_ZERO << COLOR_DESTBLEND_shift);
    /* InReverse */
    //blendcntl = (BLEND_ZERO << COLOR_SRCBLEND_shift) | (BLEND_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* Out */
    //blendcntl = (BLEND_ONE_MINUS_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_ZERO << COLOR_DESTBLEND_shift);
    /* OutReverse */
    //blendcntl = (BLEND_ZERO << COLOR_SRCBLEND_shift) | (BLEND_ONE_MINUS_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* Atop */
    //blendcntl = (BLEND_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_ONE_MINUS_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* AtopReverse */
    //blendcntl = (BLEND_ONE_MINUS_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* Xor */
    //blendcntl = (BLEND_ONE_MINUS_DST_ALPHA << COLOR_SRCBLEND_shift) | (BLEND_ONE_MINUS_SRC_ALPHA << COLOR_DESTBLEND_shift);
    /* Add */
    //blendcntl = (BLEND_ONE << COLOR_SRCBLEND_shift) | (BLEND_ONE << COLOR_DESTBLEND_shift);

    // XXX fix me rops
    if (adapt->chipset == CHIPSET_R600) {
	    ereg  (CB_COLOR_CONTROL,                    ((0xcc << ROP3_shift) | /* copy */
							 (1 << TARGET_BLEND_ENABLE_shift)));
	    ereg  (CB_BLEND_CONTROL,                    blendcntl);
    } else {
	    ereg  (CB_COLOR_CONTROL,                    ((0xcc << ROP3_shift) | /* copy */
							 (1 << TARGET_BLEND_ENABLE_shift) |
							 PER_MRT_BLEND_bit));
	    ereg  (CB_BLEND0_CONTROL,                   blendcntl);
    }

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    // XXX fix me depth 16
    cb_conf.format = COLOR_8_8_8_8;
    cb_conf.comp_swap = 0;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SU_SC_MODE_CNTL,                  (FACE_bit			|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_FRONT_PTYPE_shift)	|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_BACK_PTYPE_shift)));
    ereg  (DB_SHADER_CONTROL,                   ((1 << Z_ORDER_shift)		| /* EARLY_Z_THEN_LATE_Z */
						 DUAL_EXPORT_ENABLE_bit)); /* Only useful if no depth export */

    /* Interpolator setup */
    /* Enabling flat shading needs both FLAT_SHADE_bit in SPI_PS_INPUT_CNTL_x
     * *and* FLAT_SHADE_ENA_bit in SPI_INTERP_CONTROL_0 */
    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift)));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0 + (0 <<2),       ((0    << SEMANTIC_shift)	|
						 (0x03 << DEFAULT_VAL_shift)	|
						 FLAT_SHADE_bit			|
						 SEL_CENTROID_bit));
    ereg  (SPI_INTERP_CONTROL_0,                /* FLAT_SHADE_ENA_bit | */ 0);

}

static void
R600Solid(adapter_t *adapt, int x1, int y1, int x2, int y2, uint32_t fg,
	  struct r6xx_solid_vertex *vb, int *index)
{
    struct r6xx_solid_vertex vertex[3];

    vertex[0].x = x1;
    vertex[0].y = y1;
    vertex[0].color = fg;

    vertex[1].x = x1;
    vertex[1].y = y2;
    vertex[1].color = fg;

    vertex[2].x = x2;
    vertex[2].y = y2;
    vertex[2].color = fg;

    // append to vertex buffer
    vb[(*index)++] = vertex[0];
    vb[(*index)++] = vertex[1];
    vb[(*index)++] = vertex[2];
}

static void
R600DoneSolid(adapter_t *adapt, struct r6xx_solid_vertex *vb, int *index)
{
    uint64_t vb_addr;
    draw_config_t   draw_conf;
    vtx_resource_t  vtx_res;


    CLEAR (draw_conf);
    CLEAR (vtx_res);

    // don't draw if vb is empty
    if (*index == 0)
	    return;

    // fixme use gart buffer directly
    vb_addr = upload (adapt, vb, (*index) * 12, 8192);

    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = 12 / 4;
    vtx_res.vtx_num_entries = (*index) * 12 / 4;
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    set_vtx_resource        (adapt, &vtx_res);

    /* Draw */
    draw_conf.prim_type          = DI_PT_RECTLIST;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.num_instances      = 1;
    draw_conf.num_indices        = vtx_res.vtx_num_entries / vtx_res.vtx_size_dw;
    draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);	/* ? */
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);

    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void
test_solid(adapter_t *adapt)
{
	struct r6xx_solid_vertex vb[256];
	int alu = 3;
	uint32_t pm = 0xffffffff;
	uint32_t fg = 0x80ff0000;
	int index = 0;

	R600PrepareSolid(adapt, alu, pm, fg);

	// build vertex buffer
	R600Solid(adapt,   0,   0, 200, 200, fg, vb, &index);
	R600Solid(adapt, 500, 500, 700, 700, fg, vb, &index);
	R600Solid(adapt,   0, 300, 300, 400, fg, vb, &index);

	R600DoneSolid(adapt, vb, &index);

}

static void
R600PrepareCopy(adapter_t *adapt,
		int xdir, int ydir,
		int rop,
		uint32_t planemask)
{
    int i = 0;
    uint32_t vs[24];
    uint32_t ps[8];

    //0
    vs[i++] = CF_DWORD0(ADDR(8));
    vs[i++] = CF_DWORD1(POP_COUNT(0),
			CF_CONST(0),
			COND(SQ_CF_COND_ACTIVE),
			COUNT(2),
			CALL_COUNT(0),
			END_OF_PROGRAM(0),
			VALID_PIXEL_MODE(0),
			CF_INST(SQ_CF_INST_VTX),
			WHOLE_QUAD_MODE(0),
			BARRIER(1));
    //1
    vs[i++] = CF_ALU_DWORD0(ADDR(4),
			    KCACHE_BANK0(0),
			    KCACHE_BANK1(0),
			    KCACHE_MODE0(0));
    vs[i++] = CF_ALU_DWORD1(KCACHE_MODE1(0),
			    KCACHE_ADDR0(0),
			    KCACHE_ADDR1(0),
			    COUNT(4),
			    USES_WATERFALL(0),
			    CF_INST(SQ_CF_INST_ALU),
			    WHOLE_QUAD_MODE(0),
			    BARRIER(1));
    //2
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				      TYPE(SQ_EXPORT_POS),
				      RW_GPR(1),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(0),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));
    //3
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				      TYPE(SQ_EXPORT_PARAM),
				      RW_GPR(0),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(1),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(0));
    //4
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),			/* ALU inst 0 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    //5
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),			/* ALU inst 1 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    //6
    vs[i++] = ALU_DWORD0(SRC0_SEL(0),			/* ALU inst 0 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    //7
    vs[i++] = ALU_DWORD0(SRC0_SEL(0),			/* ALU inst 1 */
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(0),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    //8/9
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(16));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(1),
			     DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_0),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_32_32), //xxx
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_INT), //xxx
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED), //xxx
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(0),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(1));
    vs[i++] = VTX_DWORD_PAD;
    //10/11
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(8));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(0),
			     DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_0),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_32_32), //xxx
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_INT), //xxx
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED), //xxx
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(8),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(0));
    vs[i++] = VTX_DWORD_PAD;

    i = 0;

    // CF INST 0
    ps[i++] = CF_DWORD0(ADDR(2));
    ps[i++] = CF_DWORD1(POP_COUNT(0),
			CF_CONST(0),
			COND(SQ_CF_COND_ACTIVE),
			COUNT(1),
			CALL_COUNT(0),
			END_OF_PROGRAM(0),
			VALID_PIXEL_MODE(0),
			CF_INST(SQ_CF_INST_TEX),
			WHOLE_QUAD_MODE(0),
			BARRIER(1));
    // CF INST 1
    ps[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				      TYPE(SQ_EXPORT_PIXEL),
				      RW_GPR(0),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(1));
    ps[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(1),
					   END_OF_PROGRAM(1),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));
    // TEX INST 0
    ps[i++] = TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
			 BC_FRAC_MODE(0),
			 FETCH_WHOLE_QUAD(0),
			 RESOURCE_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 R7xx_ALT_CONST(0));
    ps[i++] = TEX_DWORD1(DST_GPR(0),
			 DST_REL(ABSOLUTE),
			 DST_SEL_X(SQ_SEL_X),
			 DST_SEL_Y(SQ_SEL_Y),
			 DST_SEL_Z(SQ_SEL_Z),
			 DST_SEL_W(SQ_SEL_W),
			 LOD_BIAS(0),
			 COORD_TYPE_X(TEX_UNNORMALIZED),
			 COORD_TYPE_Y(TEX_UNNORMALIZED),
			 COORD_TYPE_Z(TEX_UNNORMALIZED),
			 COORD_TYPE_W(TEX_UNNORMALIZED));
    ps[i++] = TEX_DWORD2(OFFSET_X(0),
			 OFFSET_Y(0),
			 OFFSET_Z(0),
			 SAMPLER_ID(0),
			 SRC_SEL_X(SQ_SEL_X),
			 SRC_SEL_Y(SQ_SEL_Y),
			 SRC_SEL_Z(SQ_SEL_0),
			 SRC_SEL_W(SQ_SEL_1));

    cb_config_t     cb_conf;
    tex_resource_t  tex_res;
    tex_sampler_t   tex_samp;
    shader_config_t vs_conf, ps_conf;
    uint64_t vs_addr, ps_addr;


    CLEAR (cb_conf);
    CLEAR (tex_res);
    CLEAR (tex_samp);
    CLEAR (vs_conf);
    CLEAR (ps_conf);

    if (verbose) {
	dump_shader (adapt, vs, sizeof(vs), "vertex");
	dump_shader (adapt, ps, sizeof(ps), "pixel");
        printf ("\n");
    }

    /* Init */
    start_3d(adapt);
    set_default_state(adapt);

    /* Scissor / viewport */
    ereg  (PA_CL_VTE_CNTL,                      VTX_XY_FMT_bit);
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);

    // fixme use gart buffer directly
    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    /* Shader */
    vs_conf.shader_addr         = vs_addr;
    vs_conf.num_gprs            = 4;
    vs_conf.stack_size          = 1;
    vs_setup                    (adapt, &vs_conf);

    ps_conf.shader_addr         = ps_addr;
    ps_conf.num_gprs            = 1;
    ps_conf.stack_size          = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.clamp_consts        = 1;
    ps_conf.export_mode         = 2;
    ps_setup                    (adapt, &ps_conf);


    /* Texture */
    tex_res.id                  = 0;
    tex_res.w                   = adapt->color_pitch;
    tex_res.h                   = adapt->color_height;
    tex_res.pitch               = adapt->color_pitch; //xxx
    tex_res.depth               = 0;
    tex_res.dim                 = 1; //2D
    tex_res.base                = adapt->color_gpu;
    tex_res.mip_base            = adapt->color_gpu;
    // fix me depth 16, etc.
    tex_res.format              = FMT_8_8_8_8;
    tex_res.request_size        = 2;
    tex_res.dst_sel_x           = 0;
    tex_res.dst_sel_y           = 1;
    tex_res.dst_sel_z           = 2;
    tex_res.dst_sel_w           = 3;
    tex_res.base_level          = 1;
    tex_res.last_level          = 0;
    tex_res.perf_modulation     = 1;
    set_tex_resource            (adapt, &tex_res);

    tex_samp.id                 = 0;
    tex_samp.clamp_x            = 2;
    tex_samp.clamp_y            = 2;
    tex_samp.clamp_z            = 0;
    tex_samp.xy_mag_filter      = 0;			/* 0: point 1:bilinear 2:bicubic */
    tex_samp.xy_min_filter      = 0;			/* 0: point 1:bilinear 2:bicubic */
    tex_samp.z_filter           = 0;			/* 0: none 1: point 2: linear */
    tex_samp.mip_filter         = 0;			/* no mipmap */
    set_tex_sampler             (adapt, &tex_samp);


    /* Render setup */
    //fixme planemask
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (R7xx_CB_SHADER_CONTROL,              (RT0_ENABLE_bit));
    // fixme rops
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); /* copy */

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    // fix me depth 16, etc.
    cb_conf.format = COLOR_8_8_8_8;
    cb_conf.comp_swap = 0;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SU_SC_MODE_CNTL,                  (FACE_bit			|
	   		 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_FRONT_PTYPE_shift)	|
	   		 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_BACK_PTYPE_shift)));
    ereg  (DB_SHADER_CONTROL,                   ((1 << Z_ORDER_shift)		| /* EARLY_Z_THEN_LATE_Z */
						 DUAL_EXPORT_ENABLE_bit)); /* Only useful if no depth export */

    /* Interpolator setup */
    /* Enabling flat shading needs both FLAT_SHADE_bit in SPI_PS_INPUT_CNTL_x
     * *and* FLAT_SHADE_ENA_bit in SPI_INTERP_CONTROL_0 */
    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift)));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0 + (0 <<2),       ((0    << SEMANTIC_shift)	|
						 (0x03 << DEFAULT_VAL_shift)	|
						 //FLAT_SHADE_bit			|
						 SEL_CENTROID_bit));
    ereg  (SPI_INTERP_CONTROL_0,                /* FLAT_SHADE_ENA_bit | */ 0);

}

static void
R600Copy(adapter_t *adapt,
	 int srcX, int srcY,
	 int dstX, int dstY,
	 int w,    int h,
	 struct r6xx_copy_vertex *vb, int *index)
{
    struct r6xx_copy_vertex vertex[3];

    vertex[0].x = dstX;
    vertex[0].y = dstY;
    vertex[0].s = srcX;
    vertex[0].t = srcY;

    vertex[1].x = dstX;
    vertex[1].y = dstY + h;
    vertex[1].s = srcX;
    vertex[1].t = srcY + h;

    vertex[2].x = dstX + w;
    vertex[2].y = dstY + h;
    vertex[2].s = srcX + w;
    vertex[2].t = srcY + h;

    // append to vertex buffer
    vb[(*index)++] = vertex[0];
    vb[(*index)++] = vertex[1];
    vb[(*index)++] = vertex[2];

}

static void
R600DoneCopy(adapter_t *adapt, struct r6xx_copy_vertex *vb, int *index)
{
    draw_config_t   draw_conf;
    vtx_resource_t  vtx_res;
    uint64_t vb_addr;

    CLEAR (draw_conf);
    CLEAR (vtx_res);

    // don't draw if vb is empty
    if (*index == 0)
	    return;

    // fixme use gart buffer directly
    vb_addr = upload (adapt, vb, (*index) * 16, 8192);

    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = 16 / 4;
    vtx_res.vtx_num_entries = (*index) * 16 / 4;
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    set_vtx_resource        (adapt, &vtx_res);

    draw_conf.prim_type          = DI_PT_RECTLIST;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.num_instances      = 1;
    draw_conf.num_indices        = vtx_res.vtx_num_entries / vtx_res.vtx_size_dw;
    draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);	/* ? */
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);

    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void
test_copy(adapter_t *adapt)
{
	struct r6xx_copy_vertex vb[256];
	int rop = 3;
	uint32_t pm = 0xffffffff;
	int index = 0;
	int xdir = 0;
	int ydir = 0;

	R600PrepareCopy(adapt, xdir, ydir, rop, pm);

	// build vertex buffer
	R600Copy(adapt, 0, 0, 500, 500, 200, 200, vb, &index);
	R600Copy(adapt, 0, 0, 500,   0, 200, 200, vb, &index);
	R600Copy(adapt, 0, 0,   0, 500, 200, 200, vb, &index);

	R600DoneCopy(adapt, vb, &index);

}

