/*
 * RadeonHD R6xx, R7xx DRI driver
 *
 * Copyright (C) 2008-2009  Matthias Hopf
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

/*
 * Single triangle tests, derived from pm4 tests
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


/*
 * Simple triangle test, masively edited
 */

void tri_test_2d(adapter_t *adapt)
{
    draw_config_t   draw_conf;
    cb_config_t     cb_conf;
    vtx_resource_t  vtx_res;
    tex_resource_t  tex_res;
    tex_sampler_t   tex_samp;
    shader_config_t vs_conf, ps_conf;
    
    uint64_t vb_addr, vs_addr, ps_addr;
    int vs_len, vb_len;

    static int32_t vb_orig[][3] = {
	{ 128,   0, 0xFFFF0000 },
	{   0, 196, 0xFF00FF00 },
	{ 256, 320, 0xFF0000FF },
	{ 192, -32, 0xFF00FFFF },
	{ 128, 128, 0xFFFF00FF },
	{ 320, 192, 0xFFFFFF00 },
    } ;
    char vb[256];					/* char because of aliasing */

    uint32_t vs[40];
    static uint32_t ps[] = {
	// CF INST 0
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				TYPE(SQ_EXPORT_PIXEL),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(1)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(1),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
    } ;


    int i = 0;
    vs[i++] = CF_DWORD0(ADDR(6));			/* CF INST 0 */
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
    if ((vertex_alu_scale_override == 0 &&
	 (vertex_load_as_int > 0 && vertex_format > 0)) ||
	vertex_alu_scale_override == 1) {
	vs[i++] = CF_ALU_DWORD0(ADDR(4),		/* CF INST 1a */
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
    } else {
	vs[i++] = CF_ALU_DWORD0(0, 0, 0, 0);		/* CF INST 1b */
	vs[i++] = CF_ALU_DWORD1(0, 0, 0, 0, 0, CF_INST(SQ_CF_INST_NOP), 0, BARRIER(0));
    }
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0), /* CF INST 2 */
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
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),	/* CF INST 3 */
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
	// INT_TO_FLT is trans unit only, so both use individual slots
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
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),	/* VTX inst 0 */
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(12));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(1), DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_0),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(vertex_format == 0 ? FMT_32_32_FLOAT :
					 vertex_format == 1 ? FMT_32_32_FLOAT :
					 vertex_format == 2 ? FMT_16_16_FLOAT :
					 vertex_format == 3 ? FMT_32_32       :
					 vertex_format == 4 ? FMT_16_16       : -1),
			     NUM_FORMAT_ALL(vertex_load_as_int == 0 ? SQ_NUM_FORMAT_SCALED :
					    vertex_load_as_int == 1 ? SQ_NUM_FORMAT_INT    :
					    vertex_load_as_int == 2 ? SQ_NUM_FORMAT_NORM   : -1),
			     FORMAT_COMP_ALL(vertex_unsigned ? SQ_FORMAT_COMP_UNSIGNED :
					     SQ_FORMAT_COMP_SIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(0),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(1));
    vs[i++] = VTX_DWORD_PAD;
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),	/* VTX inst 1 */
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(4));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(2), DST_REL(0),
			     DST_SEL_X(SQ_SEL_Z),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_X),
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
    vs_len = i;

    vb_len = sizeof(vb_orig) / 12;
    for (i = 0; i < vb_len; i++) {
	int32_t *vbl = (int32_t *) &vb[i*12];
	if (vertex_format == 0) {
	    float   *vbf = (float   *) vbl;
	    *vbf++ = vb_orig[i][0];
	    *vbf++ = vb_orig[i][1];
	} else if (vertex_format == 1 || vertex_format == 3) {
	    *vbl++ = vb_orig[i][0];
	    *vbl++ = vb_orig[i][1];
	} else {
	    int16_t *vbs = (int16_t *) vbl;
	    *vbs++ = vb_orig[i][0];
	    *vbs++ = vb_orig[i][1];
	}
	vbl = (int32_t *) &vb[i*12 + 8];
	*vbl++ = vb_orig[i][2];
    }

    CLEAR (draw_conf);
    CLEAR (cb_conf);
    CLEAR (vtx_res);
    CLEAR (tex_res);
    CLEAR (tex_samp);
    CLEAR (vs_conf);
    CLEAR (ps_conf);


    printf ("\n* Tri Test 2d - Vertex format    %s %s %s\n\n",
	    vertex_format == 0 ? "float" :
	    vertex_format == 1 ? "int32 (FMT_32_32_FLOAT)" :
	    vertex_format == 2 ? "int16 (FMT_16_16_FLOAT)" :
	    vertex_format == 3 ? "int32 (FMT_32_32)" :
	    vertex_format == 4 ? "int16 (FMT_16_16)" : NULL,
	    vertex_load_as_int == 0 ? "_SCALED" : vertex_load_as_int == 1 ? "_INT" : "_NORM",
	    vertex_unsigned ? "unsigned" : "signed"
	);

    if (verbose) {
	dump_shader (adapt, vs, vs_len * 4, "vertex");
	dump_shader (adapt, ps, sizeof(ps), "pixel");
        printf ("\n");
    }


    /* Init */
    start_3d(adapt);
    set_default_state(adapt);


    /* Scissor / viewport */
    ereg  (PA_CL_VTE_CNTL,                      VTX_XY_FMT_bit);
    /* Not necessary due to PA_CL_VTE_CNTL */
//    pack0  (PA_CL_VPORT_XSCALE_0, 4);
//    efloat (1.0);
//    efloat (0.0);
//    efloat (1.0);
//    efloat (0.0);
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);

    
    /* Upload */
    vs_addr = upload (adapt, vs, vs_len * 4, 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);
    vb_addr = upload (adapt, vb, vb_len * 12, 8192);


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
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (R7xx_CB_SHADER_CONTROL,              (RT0_ENABLE_bit));
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); /* copy */

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    cb_conf.format = FMT_8_8_8_8;
    cb_conf.comp_swap = 1;
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


    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = 12 / 4;
    vtx_res.vtx_num_entries = vb_len * 12 / 4;		/* Can overcommit if necessary */
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    /* these are only used if USE_CONST_FIELDS is set in the shader vtx fetch instruction */
    vtx_res.format          = (vertex_format == 0 ? FMT_32_32_FLOAT :
			       vertex_format == 1 ? FMT_32_32_FLOAT :
			       vertex_format == 2 ? FMT_16_16_FLOAT :
			       vertex_format == 3 ? FMT_32_32       :
			       vertex_format == 4 ? FMT_16_16       : -1);
    vtx_res.num_format_all  = (vertex_load_as_int == 0 ? SQ_NUM_FORMAT_SCALED :
			       vertex_load_as_int == 1 ? SQ_NUM_FORMAT_INT    :
			       vertex_load_as_int == 2 ? SQ_NUM_FORMAT_NORM   : -1);
    vtx_res.format_comp_all = (vertex_unsigned ? SQ_FORMAT_COMP_UNSIGNED :
			       SQ_FORMAT_COMP_SIGNED);

    set_vtx_resource        (adapt, &vtx_res);


    /* Draw */
    draw_conf.prim_type          = DI_PT_TRILIST;
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


void tri_test_3d(adapter_t *adapt)
{
    // 4 DWs per const
    static  float vs_alu_consts_12[] = {
	2.0,                 0.0,                 0.0,                -1.0,
	0.0,                -2.0,                 0.0,                 1.0,
	0.0,                 0.0,                -1.0,                -0.0,
	0.0,                 0.0,                 0.0,                 1.0,
	0.9999999403953552,  0.0,                 0.0,                -0.0,
	0.0,                 0.9999998807907104,  0.0,                -0.0,
	0.0,                 0.0,                 0.9999999403953552, -0.0,
	0.0,                 0.0,                 0.0,                 1.0,
    };

    uint32_t vs[100];

    static uint32_t trivial_ps[] = {
	// CF INST 0
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				TYPE(SQ_EXPORT_PIXEL),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(1)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(1),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
    } ;

    typedef struct {
	float x, y, z;
	uint32_t argb;
    } vertex_3d_t;
    static vertex_3d_t vb[] = {
	{   .5,     0, 0, 0xFFFF0000 },
	{    0,   .75, 0, 0xFF00FF00 },
	{    1,  1.25, 0, 0xFF0000FF },
	{  .75, -.125, 0, 0xFF00FFFF },
	{   .5,    .5, 0, 0xFFFF00FF },
	{ 1.25,   .75, 0, 0xFFFFFF00 },
    };

    static uint32_t indices[] = { 0, 1, 2, 3, 4, 5 };
    int vs_len;

    int i = 0;
    // CF INST 0
    vs[i++] = CF_DWORD0(ADDR(28));
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
    // CF INST 1
    vs[i++] = CF_ALU_DWORD0(ADDR(4),
			    KCACHE_BANK0(0),
			    KCACHE_BANK1(0),
			    KCACHE_MODE0(0));
    vs[i++] = CF_ALU_DWORD1(KCACHE_MODE1(0),
			    KCACHE_ADDR0(0),
			    KCACHE_ADDR1(0),
			    COUNT(24),
			    USES_WATERFALL(0),
			    CF_INST(SQ_CF_INST_ALU),
			    WHOLE_QUAD_MODE(0),
			    BARRIER(1));
    // CF INST 2
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
    // CF INST 3
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
					   BARRIER(1));
    // ALU clause INST 0
    // ALU 0: R1 dot C16 -> R127.X
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(272),				//cfile
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(127),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 1
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(272),				//cfile
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 2
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(272),				//cfile
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Z),
			     CLAMP(0));
    // 3
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_W),
			 SRC0_NEG(0),
			 SRC1_SEL(272),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 PRED_SEL(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // ALU 4: R1 dot C17 -> R127.Y
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(273),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 5
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(273),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(127),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 6
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(273),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Z),
			     CLAMP(0));
    // 7
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_W),
			 SRC0_NEG(0),
			 SRC1_SEL(273),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // ALU 8: R1 dot C18 -> R127.Z
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(274),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 9
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(274),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 10
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(274),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(127),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Z),
			     CLAMP(0));
    // 11
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_W),
			 SRC0_NEG(0),
			 SRC1_SEL(274),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // ALU 12: R1 dot C19 -> R127.W
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(275),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 13
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(275),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 14
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(275),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Z),
			     CLAMP(0));
    // 15
    vs[i++] = ALU_DWORD0(SRC0_SEL(1),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_W),
			 SRC0_NEG(0),
			 SRC1_SEL(275),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(127),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // ALU 16: R127 dot C12 -> R1.X
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(268),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 17
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(268),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 18
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(268),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0)
	),
	vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
				 SRC0_ABS(0),
				 SRC1_ABS(0),
				 UPDATE_EXECUTE_MASK(0),
				 UPDATE_PRED(0),
				 WRITE_MASK(0),
				 FOG_MERGE(0),
				 OMOD(SQ_ALU_OMOD_OFF),
				 ALU_INST(SQ_OP2_INST_DOT4),
				 BANK_SWIZZLE(SQ_ALU_VEC_012),
				 DST_GPR(0),
				 DST_REL(ABSOLUTE),
				 DST_ELEM(ELEM_Z),
				 CLAMP(0));
    // 19
    vs[i++] = ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(268),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // ALU 20: R127 dot C13 -> R1.Y
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_X),
			 SRC0_NEG(0),
			 SRC1_SEL(269),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_X),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_X),
			     CLAMP(0));
    // 21
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Y),
			 SRC0_NEG(0),
			 SRC1_SEL(269),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Y),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(1),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(1),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Y),
			     CLAMP(0));
    // 22
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_Z),
			 SRC0_NEG(0),
			 SRC1_SEL(269),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_Z),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(0));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_Z),
			     CLAMP(0));
    // 23
    vs[i++] = ALU_DWORD0(SRC0_SEL(127),
			 SRC0_REL(ABSOLUTE),
			 SRC0_ELEM(ELEM_W),
			 SRC0_NEG(0),
			 SRC1_SEL(269),
			 SRC1_REL(ABSOLUTE),
			 SRC1_ELEM(ELEM_W),
			 SRC1_NEG(0),
			 INDEX_MODE(SQ_INDEX_AR_X),
			 INDEX_MODE(SQ_PRED_SEL_OFF),
			 LAST(1));
    vs[i++] = ALU_DWORD1_OP2(adapt->chipset,
			     SRC0_ABS(0),
			     SRC1_ABS(0),
			     UPDATE_EXECUTE_MASK(0),
			     UPDATE_PRED(0),
			     WRITE_MASK(0),
			     FOG_MERGE(0),
			     OMOD(SQ_ALU_OMOD_OFF),
			     ALU_INST(SQ_OP2_INST_DOT4),
			     BANK_SWIZZLE(SQ_ALU_VEC_012),
			     DST_GPR(0),
			     DST_REL(ABSOLUTE),
			     DST_ELEM(ELEM_W),
			     CLAMP(0));
    // VTX clause INST 0
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(16));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(1), DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_Z),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_32_32_32_FLOAT),
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(0),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(1));
    vs[i++] = VTX_DWORD_PAD,
	// VTX clause INST 1
	vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			     FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			     FETCH_WHOLE_QUAD(0),
			     BUFFER_ID(0),
			     SRC_GPR(0),
			     SRC_REL(ABSOLUTE),
			     SRC_SEL_X(SQ_SEL_X),
			     MEGA_FETCH_COUNT(4));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(2), DST_REL(0),
			     DST_SEL_X(SQ_SEL_Z),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_X),
			     DST_SEL_W(SQ_SEL_W),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_8_8_8_8),
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(12),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(0));
    vs[i++] = VTX_DWORD_PAD;
    vs_len = i;
    
    draw_config_t   draw_conf;
    cb_config_t     cb_conf;
    vtx_resource_t  vtx_res;
    tex_resource_t  tex_res;
    tex_sampler_t   tex_samp;
    shader_config_t vs_conf, ps_conf;
    
    uint64_t vb_addr, vs_addr, ps_addr;

    CLEAR (draw_conf);
    CLEAR (cb_conf);
    CLEAR (vtx_res);
    CLEAR (tex_res);
    CLEAR (tex_samp);
    CLEAR (vs_conf);
    CLEAR (ps_conf);


    printf ("\n* Tri Test 3d\n\n");

    if (verbose) {
	dump_shader (adapt, vs, vs_len * 4, "vertex");
	dump_shader (adapt, trivial_ps, sizeof(trivial_ps), "pixel");
        printf ("\n");
    }


    /* Init */
    start_3d(adapt);
    set_default_state(adapt);


    /* Scissor / viewport */
    set_generic_scissor(0, 0, 256, 256);
    set_window_scissor(0, 0, 256, 256);
    ereg  (PA_CL_VTE_CNTL,                      (VPORT_X_SCALE_ENA_bit |
						 VPORT_X_OFFSET_ENA_bit |
						 VPORT_Y_SCALE_ENA_bit |
						 VPORT_Y_OFFSET_ENA_bit |
						 VPORT_Z_SCALE_ENA_bit |
						 VPORT_Z_OFFSET_ENA_bit |
						 //VTX_XY_FMT_bit |
						 //VTX_Z_FMT_bit |
						 VTX_W0_FMT_bit |
						 0));
    set_viewport(adapt, 256, 256, 0);
    ereg  (PA_CL_CLIP_CNTL,                     0x00000000);

    
    /* Upload */
    vs_addr = upload (adapt, vs, vs_len * 4, 0);
    ps_addr = upload (adapt, trivial_ps, sizeof(trivial_ps), 4096);
    vb_addr = upload (adapt, vb, sizeof(vb), 8192);


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


    /* Constants */
    set_alu_consts(SQ_ALU_CONSTANT_vs + 12,
		   sizeof(vs_alu_consts_12) / SQ_ALU_CONSTANT_offset, vs_alu_consts_12);


    /* Render setup */
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (0x287A0,                             0x00000001);
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); /* copy */

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    cb_conf.format = FMT_8_8_8_8;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SU_SC_MODE_CNTL,                  (FACE_bit			|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_FRONT_PTYPE_shift)	|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_BACK_PTYPE_shift)));
    ereg  (DB_SHADER_CONTROL,                   ((1 << Z_ORDER_shift)		| /* EARLY_Z_THEN_LATE_Z */
						 DUAL_EXPORT_ENABLE_bit)); /* Only useful if no depth export */


    /* Interpolator setup */
    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift)	|
						 (1 << BARYC_SAMPLE_CNTL_shift)	|
						 PERSP_GRADIENT_ENA_bit));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0 + (0 <<2),       ((0    << SEMANTIC_shift)	|
						 (0x03 << DEFAULT_VAL_shift)	|
						 SEL_CENTROID_bit));
    ereg  (SPI_INTERP_CONTROL_0,                ((2 << PNT_SPRITE_OVRD_X_shift)	|
						 (3 << PNT_SPRITE_OVRD_Y_shift)	|
						 (0 << PNT_SPRITE_OVRD_Z_shift)	|
						 (1 << PNT_SPRITE_OVRD_W_shift)));


    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = sizeof(vertex_3d_t) / 4;
    vtx_res.vtx_num_entries = sizeof(vb) / 4;		/* Can overcommit if necessary */
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    set_vtx_resource        (adapt, &vtx_res);


    /* Draw */
    draw_conf.prim_type          = DI_PT_TRILIST;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_IMMEDIATE;
    draw_conf.num_instances      = 1;
    draw_conf.num_indices        = vtx_res.vtx_num_entries / vtx_res.vtx_size_dw;
    draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);	/* ? */
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);

    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);

    draw_immd(adapt, &draw_conf, indices);

    wait_3d_idle_clean();

}
