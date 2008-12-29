/*
 * RadeonHD R6xx, R7xx DRI driver
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

/*
 * PM4 tests, captured + cleaned up from test utilities at ATI
 * Base for all other cleaned up tests.
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
 * Simple triangle test
 */

// 4 DWs per const
static  float tri_vs_alu_consts[] = {
	0.5,                 0.0,                 2.0,                 4.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	0.0,                 0.0,                 0.0,                 0.0,
	2.0,                 0.0,                 0.0,                -1.0,
	0.0,                -2.0,                 0.0,                 1.0,
	0.0,                 0.0,                -1.0,                -0.0,
	0.0,                 0.0,                 0.0,                 1.0,
	0.9999999403953552,  0.0,                 0.0,                -0.0,
	0.0,                 0.9999998807907104,  0.0,                -0.0,
	0.0,                 0.0,                 0.9999999403953552, -0.0,
	0.0,                 0.0,                 0.0,                 1.0,
};

static  float tri_ps_alu_consts[] = {
	1.0,                 1.0,                 1.0,                 1.0,
};

void pm4play_tri_test_r6xx(adapter_t *adapt)
{
    uint32_t fs[] = {
	// CF INST 0
	CF_DWORD0(ADDR(2)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(2),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_VTX),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// CF INST 1
	CF_DWORD0(ADDR(0)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(0),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_RETURN),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// VTX clause INST 0
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_SEMANTIC),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(160),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(16)),
	VTX_DWORD1_SEM(SEMANTIC_ID(0),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_Z),
		       DST_SEL_W(SQ_SEL_1),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_32_32_32_FLOAT),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(0),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(1)),
	VTX_DWORD_PAD,
	// VTX clause INST 1
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_SEMANTIC),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(160),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(4)),
	VTX_DWORD1_SEM(SEMANTIC_ID(10),
		       DST_SEL_X(SQ_SEL_Z),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_X),
		       DST_SEL_W(SQ_SEL_W),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_8_8_8_8),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(12),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(0)),
	VTX_DWORD_PAD,
    };

    uint32_t vs[] = {
	// CF INST 0
	CF_ALU_DWORD0(ADDR(5),
		      KCACHE_BANK0(0),
		      KCACHE_BANK1(0),
		      KCACHE_MODE0(0)),
	CF_ALU_DWORD1(KCACHE_MODE1(0),
		      KCACHE_ADDR0(0),
		      KCACHE_ADDR1(0),
		      COUNT(8),
		      USES_WATERFALL(0),
		      CF_INST(SQ_CF_INST_ALU),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	// CF INST 1
	CF_DWORD0(ADDR(0)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(0),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_CALL_FS),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(0)),
	// CF INST 2
	CF_ALU_DWORD0(ADDR(13),
		      KCACHE_BANK0(0),
		      KCACHE_BANK1(0),
		      KCACHE_MODE0(0)),
	CF_ALU_DWORD1(KCACHE_MODE1(0),
		      KCACHE_ADDR0(0),
		      KCACHE_ADDR1(0),
		      COUNT(36),
		      USES_WATERFALL(0),
		      CF_INST(SQ_CF_INST_ALU),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	// CF INST 3
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				TYPE(SQ_EXPORT_POS),
				RW_GPR(1),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(0),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
	// CF INST 4
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				TYPE(SQ_EXPORT_PARAM),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(0)),
	// ALU clause INST 0
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU clause INST 1
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU clause INST 2
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU clause INST 3
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU clause INST 4
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(2),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU clause INST 5
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(2),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU clause INST 6
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(2),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU clause INST 7
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(2),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU clause INST 0
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(272),				//cfile
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(272),				//cfile
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(272),				//cfile
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(272),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(1)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)
	    ),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(1)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(1)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       
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
	    CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(1)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
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
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       CLAMP(0)),
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   INDEX_MODE(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,
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
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
    };

    uint32_t ps[] = {
	// CF INST 0
	CF_ALU_DWORD0(ADDR(2),
		      KCACHE_BANK0(0),
		      KCACHE_BANK1(0),
		      KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
		      KCACHE_ADDR0(0),
		      KCACHE_ADDR1(0),
		      COUNT(4),
		      USES_WATERFALL(0),
		      CF_INST(SQ_CF_INST_ALU),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	// CF INST 1
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
	// ALU clause INST 0
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(1)),
	// ALU clause INST 1
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(1)),
	// ALU clause INST 2
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(1)),
	// ALU clause INST 3
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_MOV),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(1)),
    };

    uint32_t vb[] = {
	0x3F000000,0x00000000,0x00000000,0xFFFF0000,		// X, Y, Z, ARGB
	0x00000000,0x3F800000,0x00000000,0xFF00FF00,		// X, Y, Z, ARGB
	0x3F800000,0x3F800000,0x00000000,0xFF0000FF,		// X, Y, Z, ARGB
    };


    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    int i;
    uint64_t vb_addr;
    uint64_t fs_addr, vs_addr, ps_addr;

    draw_config_t draw_conf;
    uint32_t indices[3] = { 0, 1, 2 };


    vtx_resource_t vtx_res;
    cb_config_t cb_conf;
    tex_resource_t tex_res;
    tex_sampler_t tex_samp;
    shader_config_t fs_conf, vs_conf, ps_conf;


    printf ("\n* PM4 Play Tri Test r6xx\n\n");

    memset(&fs_conf, 0, sizeof(shader_config_t));
    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&vtx_res, 0, sizeof(vtx_resource_t));
    memset(&tex_res, 0, sizeof(tex_resource_t));
    memset(&tex_samp, 0, sizeof(tex_sampler_t));

    draw_conf.prim_type = 0x4;					// 0x4 triangle list
    // 0x11 rect list
    draw_conf.vgt_draw_initiator = 1;
    draw_conf.num_instances = 1;
    draw_conf.num_indices = 3;
    draw_conf.index_type = 0;					// 0 = 16 bit, 1= 32 bit indicies

    fs_addr = upload (adapt, fs, sizeof(fs), 0);
    vs_addr = upload (adapt, vs, sizeof(vs), 4096);
    ps_addr = upload (adapt, ps, sizeof(ps), 8192);
    vb_addr = upload (adapt, vb, sizeof(vb), 12288);

    vtx_res.id = SQ_VTX_RESOURCE_fs;
    vtx_res.vtx_size_dw = 4;
    vtx_res.vtx_num_entries = 1024;
    vtx_res.mem_req_size = 1;
    vtx_res.vb_addr = vb_addr;

    printf("win tri r6xx\n");

    start_3d(adapt);

    set_default_state(adapt);

    // PS resources
    for (i = 0; i < 16; i++) {
	tex_res.id = i;
	tex_res.dst_sel_x = 4;
	tex_res.dst_sel_y = 4;
	tex_res.dst_sel_z = 4;
	tex_res.dst_sel_w = 5;
	set_tex_resource(adapt, &tex_res);
	tex_samp.id = i;
	set_tex_sampler (adapt, &tex_samp);
    }

    // VS resources
    for (i = 0; i < 4; i++) {
	tex_res.id = 160 + i;
	tex_res.dst_sel_x = 4;
	tex_res.dst_sel_y = 4;
	tex_res.dst_sel_z = 4;
	tex_res.dst_sel_w = 5;
	set_tex_resource(adapt, &tex_res);
	tex_samp.id = 18 + i;				/* ??? */
	set_tex_sampler (adapt, &tex_samp);
    }

    // PS alu constants
    set_alu_consts(0x3e0 >> 2, sizeof(tri_ps_alu_consts) / SQ_ALU_CONSTANT_offset, tri_ps_alu_consts);

    ereg  (CB_TARGET_MASK,                      0x0000FFFF);
    pack0 (CB_CLEAR_RED, 4);
    e32   (0x00000000);
    e32   (0x00000000);
    e32   (0x00000000);
    e32   (0x00000000);

    ereg  (SX_ALPHA_TEST_CONTROL,               0x00000007);
    ereg  (SX_ALPHA_REF,                        0x00000000);

    ereg  (DB_ALPHA_TO_MASK,                    0x0000AA00);
    ereg  (DB_DEPTH_CONTROL,                    0x00700700);
    ereg  (DB_STENCILREFMASK,                   0x00FFFF00);
    ereg  (DB_STENCILREFMASK_BF,                0x00FFFF00);
    ereg  (DB_DEPTH_CONTROL,                    0x00700700);
    ereg  (DB_DEPTH_CONTROL,                    0x00700704);
    ereg  (DB_DEPTH_CONTROL,                    0x00700774);

    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);
    ereg  (PA_SU_POLY_OFFSET_BACK_OFFSET,       0x00000000);
    ereg  (PA_SU_POLY_OFFSET_FRONT_OFFSET,      0x00000000);
    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);
    ereg  (PA_SU_POLY_OFFSET_BACK_SCALE,        0x00000000);
    ereg  (PA_SU_POLY_OFFSET_FRONT_SCALE,       0x00000000);

    set_screen_scissor(0, 0, 8192, 8192);
    set_vport_scissor(0, 0, 0, 8192, 8192);
    set_generic_scissor(0, 0, 256, 256);
    set_window_scissor(0, 0, 256, 256);

    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_MODE_CNTL,                     0x00004010);
    ereg  (PA_SC_AA_CONFIG,                     0x00000000);

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

    pack0  (PA_CL_GB_VERT_CLIP_ADJ, 4);
    efloat (3.0f);
    efloat (1.0f);
    efloat (3.0f);
    efloat (1.0f);
    ereg  (PA_CL_CLIP_CNTL,                     0x00000000);

    ereg  (SPI_FOG_CNTL,                        0x00000000);
    ereg  (SPI_INPUT_Z,                         0x00000000);

    pack0 (SPI_VS_OUT_ID_0, 10);
    e32   (0xFFFFFF0A);
    for (i = 1; i < 10; i++)
	e32   (0xFFFFFFFF);

    pack0 (CB_FOG_RED, 3);
    e32   (0x00000000);
    e32   (0x00000000);
    e32   (0x00000000);

    ereg  (SPI_FOG_CNTL,                        0x00000000);
    ereg  (SPI_INPUT_Z,                         0x00000000);

    ereg  (PA_SU_POINT_SIZE,                    0x00080008);
    ereg  (PA_SU_POINT_MINMAX,                  0x08000008);
    ereg  (PA_CL_CLIP_CNTL,                     0x00080000);

    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0x00000000);

    set_vtx_resource(adapt, &vtx_res);

    ereg  (VGT_OUTPUT_PATH_CNTL,                0x00000000);

    fs_conf.shader_addr = fs_addr;
    fs_setup(adapt, &fs_conf);

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0x00000000);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0x00000000);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 4;
    vs_conf.stack_size = 1;
    vs_setup(adapt, &vs_conf);

    ereg  (SQ_VTX_SEMANTIC_CLEAR,               0xFFFFFFFC);
    ereg  (SQ_VTX_SEMANTIC_0,                   0x00000000);
    ereg  (SQ_VTX_SEMANTIC_0 + (1 << 2),        0x0000000A);


    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         0x0000000E);
    ereg  (VGT_OUT_DEALLOC_CNTL,                0x00000010);

    ereg  (PA_CL_VS_OUT_CNTL,                   0x00000000);

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(tri_vs_alu_consts) / SQ_ALU_CONSTANT_offset, tri_vs_alu_consts);

    pack0 (SQ_BOOL_CONST_0, 2);
    e32   (0x00000001);
    e32   (0x00000000);

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.clamp_consts = 1;
    ps_conf.export_mode = 2;
    ps_setup(adapt, &ps_conf);

    ereg  (CB_SHADER_MASK,                      0x0000000F);

    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);
    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift) |
						 ((1 << BARYC_SAMPLE_CNTL_shift)) |
						 PERSP_GRADIENT_ENA_bit));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0,                 0x00000F0A);

    ereg  (CB_COLOR_CONTROL,                    0x00CC0000);

    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);
    ereg  (SPI_INTERP_CONTROL_0,                0x00000868);
    ereg  (PA_SC_LINE_CNTL,                     0x00000400);

    cb_conf.id = 0;
    cb_conf.w = surface_w;
    cb_conf.h = surface_h;
    cb_conf.base = adapt->display_gpu;
    cb_conf.format = 0x1a;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SC_GENERIC_SCISSOR_TL,            0x80000000);
    ereg  (PA_SC_GENERIC_SCISSOR_BR,            0x01000100);

    ereg  (DB_DEPTH_INFO,                       0x00000000);
    ereg  (DB_RENDER_OVERRIDE,                  0x00000040);
    ereg  (DB_SHADER_CONTROL,                   0x00000200);
    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);

    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);

    wait_3d_idle();

    ereg  (DB_SHADER_CONTROL,                   0x00000210);

    draw_immd(adapt, &draw_conf, indices);

    wait_3d_idle_clean();

}

void pm4play_tri_test_r7xx(adapter_t *adapt)
{

    uint32_t fs[] = {
	// CF INST 0
	CF_DWORD0(ADDR(2)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(2),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_VTX),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// CF INST 1
	CF_DWORD0(ADDR(0)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(1),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_RETURN),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// VTX INST 0
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_SEMANTIC),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(160),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(16)),
	VTX_DWORD1_SEM(SEMANTIC_ID(0),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_Z),
		       DST_SEL_W(SQ_SEL_1),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_32_32_32_FLOAT),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(0),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(1)),
	// VTX INST 1
	VTX_DWORD_PAD,
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_SEMANTIC),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(160),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(4)),
	VTX_DWORD1_SEM(SEMANTIC_ID(10),
		       DST_SEL_X(SQ_SEL_Z),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_X),
		       DST_SEL_W(SQ_SEL_W),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_8_8_8_8),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(12),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(0)),
	VTX_DWORD_PAD
    };
	
    uint32_t vs[] = {
	// CF INST 0
	CF_DWORD0(ADDR(0)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(1),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_CALL_FS),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(0)),
	// CF INST 1
	CF_ALU_DWORD0(ADDR(4),
		      KCACHE_BANK0(0),
		      KCACHE_BANK1(0),
		      KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
		      KCACHE_ADDR0(0),
		      KCACHE_ADDR1(0),
		      COUNT(36),
		      USES_WATERFALL(0),
		      CF_INST(SQ_CF_INST_ALU),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	// CF INST 2
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				TYPE(SQ_EXPORT_POS),
				RW_GPR(1),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(0),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
	// CF INST 3
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				TYPE(SQ_EXPORT_PARAM),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(0)),
	// ALU INST 0
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(272),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(127),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 1
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(272),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 2
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(272),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 3
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(272),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 4
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(1)),
	// ALU INST 5
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 6
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(127),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 7
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 8
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(273),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 9
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(1)),
	// ALU INST 10
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 11
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 12
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(127),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 13
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(274),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 14
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(1)),
	// ALU INST 15
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 16
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 17
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 18
	ALU_DWORD0(SRC0_SEL(1),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(275),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(127),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 19
	ALU_DWORD0(SRC0_SEL(2),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(1)),
	// ALU INST 20
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 21
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 22
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 23
	ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(268),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 24
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 25
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 26
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 27
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(269),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 28
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 29
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 30
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 31
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(270),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
	// ALU INST 32
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(0)),
	// ALU INST 33
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Y),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(0)),
	// ALU INST 34
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_Z),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(0),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(0)),
	// ALU INST 35
	ALU_DWORD0(SRC0_SEL(127),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(271),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_W),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_OFF),
		       ALU_INST(SQ_OP2_INST_PRED_SETE_PUSH),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(1),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(0)),
    };

    uint32_t ps[] = {
	// CF INST 0
	CF_ALU_DWORD0(ADDR(2),
		      KCACHE_BANK0(0),
		      KCACHE_BANK1(0),
		      KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
		      KCACHE_ADDR0(0),
		      KCACHE_ADDR1(0),
		      COUNT(4),
		      USES_WATERFALL(0),
		      CF_INST(SQ_CF_INST_ALU),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	// CF INST 1
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				TYPE(SQ_EXPORT_PIXEL),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
	// ALU INST 0
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_X),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_X),
		       CLAMP(1)),
	// ALU INST 1
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Y),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Y),
		       CLAMP(1)),
	// ALU INST 2
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_Z),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(0)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_Z),
		       CLAMP(1)),
	// ALU INST 3
	ALU_DWORD0(SRC0_SEL(0),
		   SRC0_REL(ABSOLUTE),
		   SRC0_ELEM(ELEM_W),
		   SRC0_NEG(0),
		   SRC1_SEL(SQ_ALU_SRC_0),
		   SRC1_REL(ABSOLUTE),
		   SRC1_ELEM(ELEM_X),
		   SRC1_NEG(0),
		   INDEX_MODE(SQ_INDEX_AR_X),
		   PRED_SEL(SQ_PRED_SEL_OFF),
		   LAST(1)),
	ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
		       SRC0_ABS(0),
		       SRC1_ABS(0),
		       UPDATE_EXECUTE_MASK(0),
		       UPDATE_PRED(0),
		       WRITE_MASK(1),
		       FOG_MERGE(0),
		       OMOD(SQ_ALU_OMOD_M4),
		       ALU_INST(SQ_OP2_INST_SETE_DX10),
		       BANK_SWIZZLE(SQ_ALU_VEC_012),
		       DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_ELEM(ELEM_W),
		       CLAMP(1)),
    };

    uint32_t vb[] = {
	0x3F000000,0x00000000,0x00000000,0xFFFF0000,		// X, Y, Z, ARGB
	0x00000000,0x3F800000,0x00000000,0xFF00FF00,		// X, Y, Z, ARGB
	0x3F800000,0x3F800000,0x00000000,0xFF0000FF,		// X, Y, Z, ARGB
    };

    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    int i;
    uint64_t vb_addr;
    uint64_t fs_addr, vs_addr, ps_addr;

    draw_config_t draw_conf;
    uint32_t indices[3] = { 0, 1, 2 };

    cb_config_t cb_conf;
    vtx_resource_t vtx_res;
    tex_resource_t tex_res;
    tex_sampler_t tex_samp;
    shader_config_t fs_conf, vs_conf, ps_conf;


    printf ("\n* PM4 Play Tri Test r7xx\n\n");

    memset(&fs_conf, 0, sizeof(shader_config_t));
    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&vtx_res, 0, sizeof(vtx_resource_t));
    memset(&tex_res, 0, sizeof(tex_resource_t));
    memset(&tex_samp, 0, sizeof(tex_sampler_t));

    draw_conf.prim_type = 0x4;					// 0x4 triangle list
                                                                // 0x11 rect list
    draw_conf.vgt_draw_initiator = 1;
    draw_conf.num_instances = 1;
    draw_conf.num_indices = 3;
    draw_conf.index_type = 0;					// 0 = 16 bit, 1= 32 bit indicies

    fs_addr = upload (adapt, fs, sizeof(fs), 0);
    vs_addr = upload (adapt, vs, sizeof(vs), 4096);
    ps_addr = upload (adapt, ps, sizeof(ps), 8192);
    vb_addr = upload (adapt, vb, sizeof(vb), 12288);

    vtx_res.id = SQ_VTX_RESOURCE_fs;
    vtx_res.vtx_size_dw = 4;
    vtx_res.vtx_num_entries = 1024;
    vtx_res.mem_req_size = 1;
    vtx_res.vb_addr = vb_addr;

    printf("win tri r7xx\n");


    start_3d(adapt);

    pack3 (IT_EVENT_WRITE, 1);
    e32   (0x0000002C);

    wait_3d_idle();

    pack3 (IT_CONTEXT_CONTROL, 2);
    e32   (0x80000000);
    e32   (0x80000000);

    wait_3d_idle();

    set_default_state(adapt);

    pack3 (IT_CONTEXT_CONTROL, 2);
    e32   (0x80000000);
    e32   (0x80000000);

    // PS resources
    for (i = 0; i < 16; i++) {
	tex_res.id = i;
	tex_res.dst_sel_x = 4;
	tex_res.dst_sel_y = 4;
	tex_res.dst_sel_z = 4;
	tex_res.dst_sel_w = 5;
	set_tex_resource(adapt, &tex_res);
	tex_samp.id = i;
	set_tex_sampler (adapt, &tex_samp);
    }

    // VS resources
    for (i = 0; i < 4; i++) {
	tex_res.id = 160 + i;
	tex_res.dst_sel_x = 4;
	tex_res.dst_sel_y = 4;
	tex_res.dst_sel_z = 4;
	tex_res.dst_sel_w = 5;
	set_tex_resource(adapt, &tex_res);
	tex_samp.id = 18 + i;
	set_tex_sampler (adapt, &tex_samp);
    }

    // PS alu constants
    set_alu_consts(0x3e0 >> 2, sizeof(tri_ps_alu_consts) / SQ_ALU_CONSTANT_offset, tri_ps_alu_consts);

    ereg  (CB_TARGET_MASK,                      0x0000FFFF);

    ereg  (SX_ALPHA_TEST_CONTROL,               0x00000007);
    ereg  (SX_ALPHA_REF,                        0x00000000);

    ereg  (DB_ALPHA_TO_MASK,                    0x0000AA00);
    ereg  (DB_DEPTH_CONTROL,                    0x00700700);
    ereg  (DB_STENCILREFMASK,                   0x00FFFF00);
    ereg  (DB_STENCILREFMASK_BF,                0x00FFFF00);
    ereg  (DB_DEPTH_CONTROL,                    0x00700700);
    ereg  (DB_DEPTH_CONTROL,                    0x00700704);
    ereg  (DB_DEPTH_CONTROL,                    0x00700774);

    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);
    ereg  (PA_SU_POLY_OFFSET_BACK_OFFSET,       0x00000000);
    ereg  (PA_SU_POLY_OFFSET_FRONT_OFFSET,      0x00000000);
    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);
    ereg  (PA_SU_POLY_OFFSET_BACK_SCALE,        0x00000000);
    ereg  (PA_SU_POLY_OFFSET_FRONT_SCALE,       0x00000000);

    set_screen_scissor(0, 0, 8192, 8192);
    set_vport_scissor(0, 0, 0, 8192, 8192);
    set_generic_scissor(0, 0, 256, 256);
    set_window_scissor(0, 0, 256, 256);

    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_MODE_CNTL,                     0x00514000);
    ereg  (PA_SC_AA_CONFIG,                     0x00000000);

    pack0 (SPI_VS_OUT_ID_0, 10);
    e32   (0xFFFFFF0A);
    for (i = 1; i < 10; i++)
	e32   (0xFFFFFFFF);

    ereg  (SPI_FOG_CNTL,                        0x00000000);
    ereg  (SPI_INPUT_Z,                         0x00000000);
    ereg  (SPI_FOG_CNTL,                        0x00000000);
    ereg  (SPI_INPUT_Z,                         0x00000000);

    ereg  (PA_SU_POINT_SIZE,                    0x00080008);
    ereg  (PA_SU_POINT_MINMAX,                  0x08000008);
    ereg  (PA_CL_CLIP_CNTL,                     0x00000000);

    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0x00000000);

    set_vtx_resource(adapt, &vtx_res);

    ereg  (VGT_OUTPUT_PATH_CNTL,                0x00000000);

    fs_conf.shader_addr = fs_addr;
    fs_setup(adapt, &fs_conf);

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0x00000000);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0x00000000);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 4;
    vs_conf.stack_size = 1;
    vs_setup(adapt, &vs_conf);

    ereg  (SQ_VTX_SEMANTIC_CLEAR,               0xFFFFFFFC);
    ereg  (SQ_VTX_SEMANTIC_0,                   0x00000000);
    ereg  (SQ_VTX_SEMANTIC_0 + (1 << 2),        0x0000000A);

    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         0x0000000E);
    ereg  (VGT_OUT_DEALLOC_CNTL,                0x00000010);

    ereg  (PA_CL_VS_OUT_CNTL,                   0x00000000);

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(tri_vs_alu_consts) / SQ_ALU_CONSTANT_offset, tri_vs_alu_consts);

    pack0 (SQ_BOOL_CONST_0, 2);
    e32   (0x00000001);
    e32   (0x00000000);

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

    pack0  (0x00028C0C, 4);
    efloat (62.9921875f);					// PA_CL_GB_VERT_CLIP_ADJ = 0x0000A303
    efloat (1.0f);						// PA_CL_GB_VERT_DISC_ADJ = 0x0000A304
    efloat (62.9921875f);					// PA_CL_GB_HORZ_CLIP_ADJ = 0x0000A305
    efloat (1.0f);						// PA_CL_GB_HORZ_DISC_ADJ = 0x0000A306
    ereg  (PA_CL_CLIP_CNTL,                     0x00080000);

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.clamp_consts = 1;
    ps_conf.export_mode = 2;
    ps_setup(adapt, &ps_conf);

    ereg  (CB_SHADER_MASK,                      0x0000000F);
    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);

    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift) |
						 ((1 << BARYC_SAMPLE_CNTL_shift)) |
						 PERSP_GRADIENT_ENA_bit));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0,                 0x00000F0A);
    ereg  (CB_COLOR_CONTROL,                    0x00CC0000);

    ereg  (PA_SU_SC_MODE_CNTL,                  0x00000244);

    ereg  (SPI_INTERP_CONTROL_0,                0x00000868);

    ereg  (PA_SC_LINE_CNTL,                     0x00000400);

    cb_conf.id = 0;
    cb_conf.w = surface_w;
    cb_conf.h = surface_h;
    cb_conf.base = adapt->display_gpu;
    cb_conf.format = 0x1a;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    set_generic_scissor(0, 0, 256, 256);

    ereg  (DB_DEPTH_INFO,                       0x00000000);
    ereg  (DB_RENDER_OVERRIDE,                  0x00000000);
    ereg  (DB_SHADER_CONTROL,                   0x00000210);

    draw_immd(adapt, &draw_conf, indices);

    wait_3d_idle_clean();

}

void pm4play_tri_test(adapter_t *adapt)
{
    if (adapt->chipset <= CHIPSET_RV670)
	pm4play_tri_test_r6xx(adapt);
    else
	pm4play_tri_test_r7xx(adapt);
}


/*
 * Clear test
 */

void pm4play_clear_test_r6xx(adapter_t *adapt)
{
    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    uint64_t vs_addr, ps_addr;
    draw_config_t draw_conf;
    cb_config_t cb_conf;
    db_config_t db_conf;
    shader_config_t vs_conf, ps_conf;

    // 4 DWs per const
    float vs_alu_consts[] = {
	      0.0,   0.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   0.0, 0.0, 0.0,
	      0.0,   0.0, 0.0, 0.0,
	    256.0,   0.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   0.0, 0.0, 0.0,
	      0.0,   0.0, 0.0, 0.0,
	    256.0, 256.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   0.0, 0.0, 0.0,
	      0.0,   0.0, 0.0, 0.0,
	      0.0, 256.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   0.0, 0.0, 0.0,
	      0.0,   0.0, 0.0, 0.0,
    };

    uint32_t vs[] = {
	    CF_ALU_DWORD0(ADDR(3),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(8),
			  USES_WATERFALL(1),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				    TYPE(SQ_EXPORT_POS),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(0),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				    TYPE(SQ_EXPORT_PARAM),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_MASK),
					 SRC_SEL_Y(SQ_SEL_MASK),
					 SRC_SEL_Z(SQ_SEL_MASK),
					 SRC_SEL_W(SQ_SEL_MASK),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    // (float) R0.X -> PS
	    ALU_DWORD0(SRC0_SEL(0),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MAD PS SRC_LITERAL.X PS -> R127.X, PV
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PS),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_LITERAL),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP3(SRC2_SEL(SQ_ALU_SRC_PS),
			   SRC2_REL(ABSOLUTE),
			   SRC2_ELEM(ELEM_X),
			   SRC2_NEG(0),
			   ALU_INST(SQ_OP3_INST_MULADD),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(127),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // SRC_LITERAL.X (2.0)
	    0x40000000,
	    // SRC_LITERAL.Y (0.0)
	    0x00000000,
	    // MOVA_FLOOR PV.X -> AR
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOVA_FLOOR),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV C[0+AR].X -> R0.X
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV C[0+AR].Y -> R0.Y
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    // MOV C[0+AR].Z -> R0.Z
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    // MOV C[0+AR].W -> R0.W
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       INDEX_MODE(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)
		    ),
    };

    uint32_t ps[] = {
	    CF_ALU_DWORD0(ADDR(2),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(4),
			  USES_WATERFALL(0),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				    TYPE(SQ_EXPORT_PIXEL),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    // MOV C[0].X -> R0.X
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV C[0].Y -> R0.Y
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    // MOV C[0].Z -> R0.Z
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    // MOV C[0].W -> R0.W
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
    };

    printf ("\n* PM4 Play Clear Test r6xx\n\n");

    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&db_conf, 0, sizeof(db_config_t));

    draw_conf.prim_type = DI_PT_RECTLIST;
    draw_conf.num_indices = 4;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.index_type = DI_INDEX_SIZE_32_BIT;
    draw_conf.num_instances = 1;

    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    printf("win clear r6xx: requires tiled framebuffer!!!\n");

    start_3d(adapt);

    wait_3d_idle_clean();

    cp_set_surface_sync();

    set_default_state(adapt);

    db_conf.base = adapt->display_gpu;
    db_conf.w = surface_w;
    db_conf.h = surface_h;
    db_conf.format = 6;
    db_conf.array_mode = 4;					//2;
    set_depth_target(adapt, &db_conf);

    ereg  (DB_PREFETCH_LIMIT,                   ((surface_h / 8) - 1));
    ereg  (DB_HTILE_DATA_BASE,                  0);
    ereg  (DB_HTILE_SURFACE,                    0);
    ereg  (DB_PRELOAD_CONTROL,                  0);

    set_generic_scissor(0, 0, 8192, 8192);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 1;
    vs_conf.stack_size = 0;
    vs_setup(adapt, &vs_conf);

    ereg  (SPI_VS_OUT_CONFIG,                   (VS_PER_COMPONENT_bit |
						 ((1 - 1) << VS_EXPORT_COUNT_shift)));

    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         (14 << VTX_REUSE_DEPTH_shift));
    ereg  (VGT_OUT_DEALLOC_CNTL,                (16 << DEALLOC_DIST_shift));

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.uncached_first_inst = 1;
    ps_setup(adapt, &ps_conf);

    ereg  (SPI_PS_IN_CONTROL_0,                 (((1 - 1) << NUM_INTERP_shift) |
						 (1 << BARYC_SAMPLE_CNTL_shift)));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);

    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);
    ereg  (DB_SHADER_CONTROL,                   (1 << Z_ORDER_shift)); /* EARLY_Z_THEN_LATE_Z */

    wait_3d_idle();

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);

    ereg  (DB_DEBUG,                            PREZ_MUST_WAIT_FOR_POSTZ_DONE);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);

    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);

    ereg  (VGT_OUTPUT_PATH_CNTL,                (0 << PATH_SELECT_shift)); // VGT_OUTPATH_VTX_REUSE
    ereg  (VGT_PRIMITIVEID_EN,                  0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_EN,          0); // IB-based prims disabled
    ereg  (VGT_STRMOUT_EN,                      0); // strmout off
    ereg  (VGT_REUSE_OFF,                       0); // reuse on
    ereg  (VGT_VTX_CNT_EN,                      0); // auto index gen off
    ereg  (VGT_STRMOUT_BUFFER_EN,               0); // all strmout buffers disabled
    ereg  (VGT_GS_MODE,                         0); // GS off
    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);
    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_INDX,        0);

    set_screen_scissor(0, 0, 256, 256);
    ereg  (PA_SC_WINDOW_OFFSET,                 0);
    set_window_scissor(0, 0, 8192, 8192);

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    ereg  (PA_SC_AA_CONFIG,                     0);
    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_CLIPRECT_RULE,                 0x0000FFFF);

    set_vport_scissor(0, 0, 0, 8192, 8192);

    ereg  (PA_SC_VPORT_ZMIN_0,                  0x00000000); // 0.0
    ereg  (PA_SC_VPORT_ZMAX_0,                  0x3F800000); // 1.0
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);
    ereg  (PA_SU_SC_MODE_CNTL,                  FACE_bit);
    ereg  (PA_CL_VTE_CNTL,                      (VTX_XY_FMT_bit |
                                                 VTX_Z_FMT_bit));

    ereg  (PA_SU_POINT_SIZE,                    0);
    ereg  (PA_SU_POINT_MINMAX,                  0);
    pack0 (0x00028C0C, 4);
    efloat(1.0f);						// PA_CL_GB_VERT_CLIP_ADJ = 0x0000A303
    efloat(1.0f);						// PA_CL_GB_VERT_DISC_ADJ = 0x0000A304
    efloat(1.0f);						// PA_CL_GB_HORZ_CLIP_ADJ = 0x0000A305
    efloat(1.0f);						// PA_CL_GB_HORZ_DISC_ADJ = 0x0000A306
    ereg  (PA_SC_LINE_STIPPLE,                  0);
    ereg  (PA_SC_MPASS_PS_CNTL,                 0);
    ereg  (PA_SU_LINE_CNTL,                     (8 << PA_SU_LINE_CNTL__WIDTH_shift)); // 16.0 fixed
    ereg  (PA_SU_VTX_CNTL,                      ((PIX_CENTER_bit) |
						 (2 << PA_SU_VTX_CNTL__ROUND_MODE_shift)));
    ereg  (PA_SU_POLY_OFFSET_CLAMP,             0);
    ereg  (PA_CL_VS_OUT_CNTL,                   0);

    ereg  (DB_ALPHA_TO_MASK,                    0);
    ereg  (DB_DEPTH_CONTROL,                    ((Z_ENABLE_bit) |
						 (Z_WRITE_ENABLE_bit) |
						 (7 << ZFUNC_shift))); // FRAG_ALWAYS
    ereg  (DB_RENDER_OVERRIDE,                  DISABLE_VIEWPORT_CLAMP_bit);
    ereg  (DB_SHADER_CONTROL,                   DUAL_EXPORT_ENABLE_bit);
    ereg  (DB_STENCILREFMASK,                   0);
    ereg  (DB_STENCILREFMASK_BF,                0);
    ereg  (DB_DEPTH_CLEAR,                      0); // float 0.0
    ereg  (DB_STENCIL_CLEAR,                    0);
    ereg  (DB_RENDER_CONTROL,                   ZPASS_INCREMENT_DISABLE_bit);
    ereg  (DB_SRESULTS_COMPARE_STATE0,          0);
    ereg  (DB_SRESULTS_COMPARE_STATE1,          0);

    ereg  (CB_TARGET_MASK,                      0);
    ereg  (CB_SHADER_MASK,                      0);
    ereg  (CB_CLRCMP_CONTROL,                   ((0 << CLRCMP_FCN_SRC_shift) |
						 (0 << CLRCMP_FCN_DST_shift) |
						 (2 << CLRCMP_FCN_SEL_shift)));
    ereg  (CB_CLRCMP_DST,                       0x00000000);
    ereg  (CB_CLRCMP_MSK,                       0xFFFFFFFF);
    pack0 (CB_BLEND_RED, 4);
    efloat(0.0f);						// CB_BLEND_RED = 0x0000A105
    efloat(0.0f);						// CB_BLEND_GREEN = 0x0000A106
    efloat(0.0f);						// CB_BLEND_BLUE = 0x0000A107
    efloat(0.0f);						// CB_BLEND_ALPHA = 0x0000A108

    ereg  (SPI_FOG_FUNC_BIAS + (39 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x27
    ereg  (SPI_FOG_FUNC_BIAS + (40 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x28
    ereg  (SPI_FOG_FUNC_BIAS + (41 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x29
    ereg  (SPI_FOG_FUNC_BIAS + (42 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2A
    ereg  (SPI_FOG_FUNC_BIAS + (43 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2B
    ereg  (SPI_FOG_FUNC_BIAS + (44 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2C
    ereg  (SPI_FOG_FUNC_BIAS + (45 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2D
    ereg  (SPI_FOG_FUNC_BIAS + (46 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2E

    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); // copy

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(vs_alu_consts) / SQ_ALU_CONSTANT_offset, vs_alu_consts);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void pm4play_clear_test_r7xx(adapter_t *adapt)
{
    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    uint64_t vs_addr, ps_addr;
    draw_config_t draw_conf;
    cb_config_t cb_conf;
    db_config_t db_conf;
    shader_config_t vs_conf, ps_conf;
    tex_sampler_t tex_samp;
    int i;

    // 4 DWs per const
    float ps_alu_consts[] = {
	    0.6509804129600525, 0.7921569347381592, 0.9411765336990356, 0.0,
    };

    float vs_alu_consts[] = {
	      0.0,                 0.0,                0.0,                 1.0,
	      0.03921568766236305, 0.1411764770746231, 0.41568630933761597, 0.0,
	      0.0,                 0.0,                0.0,                 0.0,
	    256.0,                 0.0,                0.0,                 1.0,
	      0.6509804129600525,  0.7921569347381592, 0.9411765336990356,  0.0,
	      0.0,                 0.0,                0.0,                 0.0,
	    256.0,               256.0,                0.0,                 1.0,
	      0.6509804129600525,  0.7921569347381592, 0.9411765336990356,  0.0,
	      0.0,                 0.0,                0.0,                 0.0,
	      0.0,               256.0,                0.0,                 1.0,
	      0.03921568766236305, 0.1411764770746231, 0.41568630933761597, 0.0,
	      0.0,                 0.0,                0.0,                 0.0,
    };

    uint32_t vs[] = {
	    CF_ALU_DWORD0(ADDR(3),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(8),
			  USES_WATERFALL(0),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				    TYPE(SQ_EXPORT_POS),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(0),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				    TYPE(SQ_EXPORT_PARAM),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_MASK),
					 SRC_SEL_Y(SQ_SEL_MASK),
					 SRC_SEL_Z(SQ_SEL_MASK),
					 SRC_SEL_W(SQ_SEL_MASK),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    ALU_DWORD0(SRC0_SEL(0),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MAX_INT),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PS),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_LITERAL),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP3(SRC2_SEL(SQ_ALU_SRC_PS),
			   SRC2_REL(ABSOLUTE),
			   SRC2_ELEM(ELEM_X),
			   SRC2_NEG(0),
			   ALU_INST(SQ_OP3_INST_MULADD),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(127),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    0x40000000,
	    0x00000000,
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_SETNE),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
    };

    uint32_t ps[] = {
	    CF_ALU_DWORD0(ADDR(2),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(4),
			  USES_WATERFALL(0),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				    TYPE(SQ_EXPORT_PIXEL),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
    };

    printf ("\n* PM4 Play Clear Test r7xx\n\n");

    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&db_conf, 0, sizeof(db_config_t));
    memset(&tex_samp, 0, sizeof(tex_sampler_t));

    draw_conf.prim_type = DI_PT_RECTLIST;
    draw_conf.num_indices = 4;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.index_type = DI_INDEX_SIZE_32_BIT;
    draw_conf.num_instances = 1;

    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    printf("win clear r7xx: requires tiled framebuffer!!!\n");

    start_3d(adapt);

    wait_3d_idle_clean();

    cp_set_surface_sync();

    set_default_state(adapt);

    db_conf.base = adapt->display_gpu;
    db_conf.w = surface_w;
    db_conf.h = surface_h;
    db_conf.format = 6;
    db_conf.array_mode = 4;					//2;
    set_depth_target(adapt, &db_conf);

    ereg  (DB_PREFETCH_LIMIT,                   ((surface_h / 8) - 1));
    ereg  (DB_HTILE_DATA_BASE,                  0);
    ereg  (DB_HTILE_SURFACE,                    0);
    ereg  (DB_PRELOAD_CONTROL,                  0);

    set_generic_scissor(0, 0, 8192, 8192);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 1;
    vs_conf.stack_size = 0;
    vs_setup(adapt, &vs_conf);

    ereg  (SPI_VS_OUT_CONFIG,                   (VS_PER_COMPONENT_bit |
						 ((1 - 1) << VS_EXPORT_COUNT_shift)));

    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         (14 << VTX_REUSE_DEPTH_shift));
    ereg  (VGT_OUT_DEALLOC_CNTL,                (16 << DEALLOC_DIST_shift));

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.export_mode = 2;
    ps_setup(adapt, &ps_conf);

    ereg  (SPI_PS_IN_CONTROL_0,                 (((1 - 1) << NUM_INTERP_shift) |
						 ((1 << BARYC_SAMPLE_CNTL_shift))));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);

    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);
    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);

    ereg  (DB_SHADER_CONTROL,                   (1 << Z_ORDER_shift)); /* EARLY_Z_THEN_LATE_Z */

    wait_3d_idle();

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (DB_DEBUG,                            0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);

    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);

    ereg  (R7xx_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ,   VS_PC_LIMIT_ENABLE_bit);

    ereg  (VGT_OUTPUT_PATH_CNTL,                (0 << PATH_SELECT_shift)); // VGT_OUTPATH_VTX_REUSE
    ereg  (VGT_PRIMITIVEID_EN,                  0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_EN,          0); // IB-based prims disabled
    ereg  (VGT_STRMOUT_EN,                      0); // strmout off
    ereg  (VGT_REUSE_OFF,                       0); // reuse on
    ereg  (VGT_VTX_CNT_EN,                      0); // auto index gen off
    ereg  (VGT_STRMOUT_BUFFER_EN,               0); // all strmout buffers disabled
    ereg  (VGT_GS_MODE,                         0); // GS off
    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);
    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_INDX,        0);

    set_screen_scissor(0, 0, 256, 256);
    ereg  (PA_SC_WINDOW_OFFSET,                 0);
    set_window_scissor(0, 0, 8192, 8192);

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    ereg  (PA_SC_AA_CONFIG,                     0);
    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_CLIPRECT_RULE,                 0x0000FFFF);

    set_vport_scissor(0, 0, 0, 8192, 8192);

    ereg  (PA_SC_VPORT_ZMIN_0,                  0x00000000); // 0.0
    ereg  (PA_SC_VPORT_ZMAX_0,                  0x3F800000); // 1.0
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);
    ereg  (PA_SU_SC_MODE_CNTL,                  FACE_bit);
    ereg  (PA_CL_VTE_CNTL,                      (VTX_XY_FMT_bit |
                                                 VTX_Z_FMT_bit));

    ereg  (PA_SU_POINT_SIZE,                    0);
    ereg  (PA_SU_POINT_MINMAX,                  0);
    pack0 (0x00028C0C, 4);
    efloat(1.0f);						// PA_CL_GB_VERT_CLIP_ADJ = 0x0000A303
    efloat(1.0f);						// PA_CL_GB_VERT_DISC_ADJ = 0x0000A304
    efloat(1.0f);						// PA_CL_GB_HORZ_CLIP_ADJ = 0x0000A305
    efloat(1.0f);						// PA_CL_GB_HORZ_DISC_ADJ = 0x0000A306
    ereg  (PA_SC_LINE_STIPPLE,                  0);
    ereg  (PA_SC_MPASS_PS_CNTL,                 0);
    ereg  (PA_SU_LINE_CNTL,                     (8 << PA_SU_LINE_CNTL__WIDTH_shift)); // 16.0 fixed
    ereg  (PA_SU_VTX_CNTL,                      ((PIX_CENTER_bit) |
						 (2 << PA_SU_VTX_CNTL__ROUND_MODE_shift)));
    ereg  (PA_SU_POLY_OFFSET_CLAMP,             0);
    ereg  (PA_CL_VS_OUT_CNTL,                   0);

    ereg  (SPI_INTERP_CONTROL_0,                0);
    ereg  (SPI_INPUT_Z,                         0);
    ereg  (SPI_FOG_CNTL,                        0);
    ereg  (SPI_FOG_FUNC_SCALE,                  0x00000000);
    ereg  (SPI_FOG_FUNC_BIAS,                   0x00000000);

    ereg  (SX_ALPHA_TEST_CONTROL,               0);
    ereg  (SX_ALPHA_REF,                        0x00000000); // 0.0

    ereg  (DB_ALPHA_TO_MASK,                    0);
    ereg  (DB_DEPTH_CONTROL,                    ((Z_ENABLE_bit) |
						 (Z_WRITE_ENABLE_bit) |
						 (7 << ZFUNC_shift))); // FRAG_ALWAYS
    ereg  (DB_RENDER_OVERRIDE,                  DISABLE_VIEWPORT_CLAMP_bit);
    ereg  (DB_SHADER_CONTROL,                   DUAL_EXPORT_ENABLE_bit);
    ereg  (DB_STENCILREFMASK,                   0);
    ereg  (DB_STENCILREFMASK_BF,                0);
    ereg  (DB_DEPTH_CLEAR,                      0); // float 0.0
    ereg  (DB_STENCIL_CLEAR,                    0);
    ereg  (DB_RENDER_CONTROL,                   ZPASS_INCREMENT_DISABLE_bit);
    ereg  (DB_SRESULTS_COMPARE_STATE0,          0);
    ereg  (DB_SRESULTS_COMPARE_STATE1,          0);

    ereg  (CB_TARGET_MASK,                      0);
    ereg  (CB_SHADER_MASK,                      0);
    ereg  (CB_CLRCMP_CONTROL,                   ((0 << CLRCMP_FCN_SRC_shift) |
						 (0 << CLRCMP_FCN_DST_shift) |
						 (2 << CLRCMP_FCN_SEL_shift)));
    ereg  (CB_CLRCMP_DST,                       0x00000000);
    ereg  (CB_CLRCMP_MSK,                       0xFFFFFFFF);
    pack0 (0x00028414, 4);
    efloat(0.0f);						// CB_BLEND_RED = 0x0000A105
    efloat(0.0f);						// CB_BLEND_GREEN = 0x0000A106
    efloat(0.0f);						// CB_BLEND_BLUE = 0x0000A107
    efloat(0.0f);						// CB_BLEND_ALPHA = 0x0000A108

    for (i = 0; i < CB_BLEND0_CONTROL_num; i++)
	    ereg  (CB_BLEND0_CONTROL + (i << 2),        (1 << COLOR_SRCBLEND_shift)); // BLEND_ONE
    ereg  (CB_COLOR_CONTROL,                    ((1 << SPECIAL_OP_shift) | // SPECIAL_DISABLE
						 (0xcc << ROP3_shift)));   // copy

    ereg  (R7xx_PA_SC_EDGERULE,                 0xAAAAAAAA);
    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);

    ereg  (R7xx_SPI_THREAD_GROUPING,            (1 << PS_GROUPING_shift));

    tex_samp.id = 0;
    set_tex_sampler (adapt, &tex_samp);

    // PS alu constants
    set_alu_consts(0x0 >> 2, sizeof(ps_alu_consts) / SQ_ALU_CONSTANT_offset, ps_alu_consts);

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(vs_alu_consts) / SQ_ALU_CONSTANT_offset, vs_alu_consts);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void pm4play_clear_test(adapter_t *adapt)
{
    if (adapt->chipset <= CHIPSET_RV670)
	pm4play_clear_test_r6xx(adapt);
    else
	pm4play_clear_test_r7xx(adapt);
}

/*
 * Blit test
 */

void pm4play_blit_test_r6xx(adapter_t *adapt)
{
    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    int tex_w = TEX_WIDTH;
    int tex_h = TEX_HEIGHT;
    int tex_p = TEX_PITCH;
    uint32_t *tex;
    uint64_t vs_addr, ps_addr, tex_addr;
    draw_config_t draw_conf;
    cb_config_t cb_conf;
    tex_resource_t tex_res;
    tex_sampler_t tex_samp;
    shader_config_t vs_conf, ps_conf;

    // 4 DWs per const
    float vs_alu_consts[] = {
	      3.0,  22.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   0.0, 0.0, 0.0, // S, T
	      0.0,   0.0, 0.0, 0.0,
	    259.0,  22.0, 0.0, 1.0, // X, Y, Z, W
	      1.0,   0.0, 0.0, 0.0, // S, T
	      0.0,   0.0, 0.0, 0.0,
	    259.0, 278.0, 0.0, 1.0, // X, Y, Z, W
	      1.0,   1.0, 0.0, 0.0, // S, T
	      0.0,   0.0, 0.0, 0.0,
	      3.0, 278.0, 0.0, 1.0, // X, Y, Z, W
	      0.0,   1.0, 0.0, 0.0, // S, T
	      0.0,   0.0, 0.0, 0.0,
    };

    uint32_t vs[] = {
	    CF_ALU_DWORD0(ADDR(3),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(12),
			  USES_WATERFALL(1),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				    TYPE(SQ_EXPORT_POS),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(0),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				    TYPE(SQ_EXPORT_PARAM),
				    RW_GPR(1),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(0)),
	    // (float) R0.X -> PS
	    ALU_DWORD0(SRC0_SEL(0),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_INT_TO_FLT),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MAD PS SRC_LITERAL.X PS -> R127.X, PV
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PS),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_LITERAL),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP3(SRC2_SEL(SQ_ALU_SRC_PS),
			   SRC2_REL(ABSOLUTE),
			   SRC2_ELEM(ELEM_X),
			   SRC2_NEG(0),
			   ALU_INST(SQ_OP3_INST_MULADD),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(127),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // SRC_LITERAL.X (2.0)
	    0x40000000,
	    // SRC_LITERAL.Y (0.0)
	    0x00000000,
	    // MOVA_FLOOR PV.X -> AR
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOVA_FLOOR),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV (C[0+AR].X) -> R0.X
	    ALU_DWORD0(SRC0_SEL(256), // cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV (C[0+AR].Y) -> R0.Y
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    // MOV (C[0+AR].Z) -> R0.Z
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    // MOV (C[0+AR].W) -> R0.W
	    ALU_DWORD0(SRC0_SEL(256), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
	    // MOV (C[1+AR].X) -> R1.X
	    ALU_DWORD0(SRC0_SEL(257), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    // MOV (C[1+AR].Y) -> R1.Y
	    ALU_DWORD0(SRC0_SEL(257), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    // MOV (C[1+AR].Z) -> R1.Z
	    ALU_DWORD0(SRC0_SEL(257), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    // MOV (C[1+AR].W) -> R1.W
	    ALU_DWORD0(SRC0_SEL(257), //cfile
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MOV),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
    };

    uint32_t ps[] = {
	    CF_DWORD0(ADDR(2)),
	    CF_DWORD1(POP_COUNT(0),
		      CF_CONST(0),
		      COND(SQ_CF_COND_ACTIVE),
		      COUNT(1),
		      CALL_COUNT(0),
		      END_OF_PROGRAM(0),
		      VALID_PIXEL_MODE(1),
		      CF_INST(SQ_CF_INST_TEX),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				    TYPE(SQ_EXPORT_PIXEL),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
		       BC_FRAC_MODE(0),
		       FETCH_WHOLE_QUAD(0),
		       RESOURCE_ID(0),
		       SRC_GPR(0),
		       SRC_REL(ABSOLUTE),
		       R7xx_ALT_CONST(0)),
	    TEX_DWORD1(DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_Z),
		       DST_SEL_W(SQ_SEL_W),
		       LOD_BIAS(0),
		       COORD_TYPE_X(TEX_NORMALIZED),
		       COORD_TYPE_Y(TEX_NORMALIZED),
		       COORD_TYPE_Z(TEX_NORMALIZED),
		       COORD_TYPE_W(TEX_NORMALIZED)),
	    TEX_DWORD2(OFFSET_X(0),
		       OFFSET_Y(0),
		       OFFSET_Z(0),
		       SAMPLER_ID(0),
		       SRC_SEL_X(SQ_SEL_X),
		       SRC_SEL_Y(SQ_SEL_Y),
		       SRC_SEL_Z(SQ_SEL_X),
		       SRC_SEL_W(SQ_SEL_X)),
	    TEX_DWORD_PAD,
    };

    printf ("\n* PM4 Play Blit Test r6xx\n\n");

    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&tex_res, 0, sizeof(tex_resource_t));
    memset(&tex_samp, 0, sizeof(tex_sampler_t));

    draw_conf.prim_type = DI_PT_RECTLIST;
    draw_conf.num_indices = 4;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.index_type = DI_INDEX_SIZE_32_BIT;
    draw_conf.num_instances = 1;

    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    tex      = create_sample_texture (tex_w, tex_h, tex_p);
    tex_addr = upload (adapt, tex, tex_p * tex_h * sizeof(uint32_t), 8192);
    free (tex);

    printf("win blit r6xx\n");

    start_3d(adapt);

    wait_3d_idle_clean();

    cp_set_surface_sync();

    set_default_state(adapt);

    ereg  (CB_TARGET_MASK,                      TARGET0_ENABLE_mask);
    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);


    cb_conf.id = 0;
    cb_conf.w = surface_w;
    cb_conf.h = surface_h;
    cb_conf.base = adapt->display_gpu;
    cb_conf.format = 0x1a;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (DB_DEPTH_INFO,                       0);

    tex_res.id = 0;
    tex_res.w = tex_w;
    tex_res.h = tex_h;
    tex_res.pitch = tex_p;
    tex_res.depth = 0;
    tex_res.dim = 1;						//2D
    tex_res.base = tex_addr;
    tex_res.mip_base = tex_addr;
    tex_res.format = FMT_8_8_8_8;
    tex_res.request_size = 2;
    tex_res.dst_sel_x = 0;
    tex_res.dst_sel_y = 1;
    tex_res.dst_sel_z = 2;
    tex_res.dst_sel_w = 3;
    tex_res.last_level = 1;
    tex_res.perf_modulation = 1;
    set_tex_resource(adapt, &tex_res);

    set_generic_scissor(0, 0, 8192, 8192);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 2;
    vs_conf.stack_size = 0;
    vs_setup(adapt, &vs_conf);

    ereg  (SPI_VS_OUT_CONFIG,                   (VS_PER_COMPONENT_bit |
						 ((1 - 1) << VS_EXPORT_COUNT_shift)));

    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         (14 << VTX_REUSE_DEPTH_shift));
    ereg  (VGT_OUT_DEALLOC_CNTL,                (16 << DEALLOC_DIST_shift));

    ereg  (SPI_VS_OUT_ID_0,                     0);

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.export_mode = 2;
    ps_setup(adapt, &ps_conf);

    ereg  (SPI_PS_IN_CONTROL_0,                 (((2 - 1) << NUM_INTERP_shift) |
						 ((1 << BARYC_SAMPLE_CNTL_shift)) |
						 PERSP_GRADIENT_ENA_bit));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);

    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);

    ereg  (DB_SHADER_CONTROL,                   (1 << Z_ORDER_shift)); /* EARLY_Z_THEN_LATE_Z */

    ereg  (SPI_PS_INPUT_CNTL_0,                 (1 << DEFAULT_VAL_shift));

    tex_samp.id = 0;
    set_tex_sampler (adapt, &tex_samp);

    wait_3d_idle();

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (DB_DEBUG,                            PREZ_MUST_WAIT_FOR_POSTZ_DONE);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);

    ereg  (VGT_OUTPUT_PATH_CNTL,                (0 << PATH_SELECT_shift)); // VGT_OUTPATH_VTX_REUSE
    ereg  (VGT_PRIMITIVEID_EN,                  0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_EN,          0); // IB-based prims disabled
    ereg  (VGT_STRMOUT_EN,                      0); // strmout off
    ereg  (VGT_REUSE_OFF,                       0); // reuse on
    ereg  (VGT_VTX_CNT_EN,                      0); // auto index gen off
    ereg  (VGT_STRMOUT_BUFFER_EN,               0); // all strmout buffers disabled
    ereg  (VGT_GS_MODE,                         0); // GS off
    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);
    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_INDX,        0);

    set_screen_scissor(0, 0, 8192, 8192);
    ereg  (PA_SC_WINDOW_OFFSET,                 0);
    set_window_scissor(0, 0, 8192, 8192);

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MODE_CNTL,                     FORCE_EOV_CNTDWN_ENABLE_bit);
    ereg  (PA_SC_AA_CONFIG,                     0);
    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_CLIPRECT_RULE,                 0x0000FFFF);

    set_vport_scissor(0, 0, 0, 8192, 8192);

    ereg  (PA_SC_VPORT_ZMIN_0,                  0x00000000); // 0.0
    ereg  (PA_SC_VPORT_ZMAX_0,                  0x3F800000); // 1.0
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);
    ereg  (PA_SU_SC_MODE_CNTL,                  FACE_bit);
    ereg  (PA_CL_VTE_CNTL,                      (VTX_XY_FMT_bit |
                                                 VTX_Z_FMT_bit));

    ereg  (PA_SU_POINT_SIZE,                    0);
    ereg  (PA_SU_POINT_MINMAX,                  0);
    pack0 (0x00028C0C, 4);
    efloat(1.0f);						// PA_CL_GB_VERT_CLIP_ADJ = 0x0000A303
    efloat(1.0f);						// PA_CL_GB_VERT_DISC_ADJ = 0x0000A304
    efloat(1.0f);						// PA_CL_GB_HORZ_CLIP_ADJ = 0x0000A305
    efloat(1.0f);						// PA_CL_GB_HORZ_DISC_ADJ = 0x0000A306
    ereg  (PA_SC_LINE_STIPPLE,                  0);
    ereg  (PA_SC_MPASS_PS_CNTL,                 0);
    ereg  (PA_SU_LINE_CNTL,                     (8 << PA_SU_LINE_CNTL__WIDTH_shift)); // 16.0 fixed
    ereg  (PA_SU_VTX_CNTL,                      ((PIX_CENTER_bit) |
						 (2 << PA_SU_VTX_CNTL__ROUND_MODE_shift)));
    ereg  (PA_SU_POLY_OFFSET_CLAMP,             0);
    ereg  (PA_CL_VS_OUT_CNTL,                   0);

    ereg  (SPI_INTERP_CONTROL_0,                0);

    ereg  (DB_ALPHA_TO_MASK,                    0);
    ereg  (DB_DEPTH_CONTROL,                    BACKFACE_ENABLE_bit);
    ereg  (DB_RENDER_OVERRIDE,                  0);
    ereg  (DB_SHADER_CONTROL,                   DUAL_EXPORT_ENABLE_bit);
    ereg  (DB_STENCILREFMASK,                   0);
    ereg  (DB_STENCILREFMASK_BF,                0);
    ereg  (DB_DEPTH_CLEAR,                      0); // float 0.0
    ereg  (DB_STENCIL_CLEAR,                    0);
    ereg  (DB_RENDER_CONTROL,                   ZPASS_INCREMENT_DISABLE_bit);
    ereg  (DB_SRESULTS_COMPARE_STATE0,          0);
    ereg  (DB_SRESULTS_COMPARE_STATE1,          0);

    ereg  (CB_TARGET_MASK,                      TARGET0_ENABLE_mask);
    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);
    ereg  (CB_CLRCMP_CONTROL,                   ((0 << CLRCMP_FCN_SRC_shift) |
						 (0 << CLRCMP_FCN_DST_shift) |
						 (2 << CLRCMP_FCN_SEL_shift)));
    ereg  (CB_CLRCMP_DST,                       0x00000000);
    ereg  (CB_CLRCMP_MSK,                       0xFFFFFFFF);

    ereg  (SPI_FOG_FUNC_BIAS + (39 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x27
    ereg  (SPI_FOG_FUNC_BIAS + (40 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x28
    ereg  (SPI_FOG_FUNC_BIAS + (41 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x29
    ereg  (SPI_FOG_FUNC_BIAS + (42 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2A
    ereg  (SPI_FOG_FUNC_BIAS + (43 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2B
    ereg  (SPI_FOG_FUNC_BIAS + (44 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2C
    ereg  (SPI_FOG_FUNC_BIAS + (45 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2D
    ereg  (SPI_FOG_FUNC_BIAS + (46 << 2),       0x00000001);	// SPI_FOG_FUNC_BIAS + 0x2E

    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); // copy

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(vs_alu_consts) / SQ_ALU_CONSTANT_offset, vs_alu_consts);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void pm4play_blit_test_r7xx(adapter_t *adapt)
{
    int surface_w = adapt->color_pitch;
    int surface_h = adapt->color_height;
    int tex_w = TEX_WIDTH;
    int tex_h = TEX_HEIGHT;
    int tex_p = TEX_PITCH;
    uint32_t *tex;
    uint64_t vs_addr, ps_addr, tex_addr;
    draw_config_t draw_conf;
    cb_config_t cb_conf;
    tex_resource_t tex_res;
    tex_sampler_t tex_samp;
    shader_config_t vs_conf, ps_conf;
    int i;

    // 4 DWs per const
    float ps_alu_consts[] = {
	    0.6509804129600525, 0.7921569347381592, 0.9411765336990356, 0.0,
    };

    float vs_alu_consts[] = {
	      3.0,  22.0, 0.0,                 1.0,
	      0.0,   0.0, 0.41568630933761597, 0.0,
	      0.0,   0.0, 0.0,                 0.0,
	    259.0,  22.0, 0.0,                 1.0,
	      1.0,   0.0, 0.9411765336990356,  0.0,
	      0.0,   0.0, 0.0,                 0.0,
	    259.0, 278.0, 0.0,                 1.0,
	      1.0,   1.0, 0.9411765336990356,  0.0,
	      0.0,   0.0, 0.0,                 0.0,
	      3.0, 278.0, 0.0,                 1.0,
	      0.0,   1.0, 0.41568630933761597, 0.0,
	      0.0,   0.0, 0.0,                 0.0,
    };

    uint32_t vs[] = {
	    CF_ALU_DWORD0(ADDR(3),
			  KCACHE_BANK0(0),
			  KCACHE_BANK1(0),
			  KCACHE_MODE0(SQ_CF_KCACHE_NOP)),
	    CF_ALU_DWORD1(KCACHE_MODE1(SQ_CF_KCACHE_NOP),
			  KCACHE_ADDR0(0),
			  KCACHE_ADDR1(0),
			  COUNT(12),
			  USES_WATERFALL(0),
			  CF_INST(SQ_CF_INST_ALU),
			  WHOLE_QUAD_MODE(0),
			  BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				    TYPE(SQ_EXPORT_POS),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(0),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				    TYPE(SQ_EXPORT_PARAM),
				    RW_GPR(1),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(0)),
	    ALU_DWORD0(SRC0_SEL(0),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_MAX_INT),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PS),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_LITERAL),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP3(SRC2_SEL(SQ_ALU_SRC_PS),
			   SRC2_REL(ABSOLUTE),
			   SRC2_ELEM(ELEM_X),
			   SRC2_NEG(0),
			   ALU_INST(SQ_OP3_INST_MULADD),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(127),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    0x40000000,
	    0x00000000,
	    ALU_DWORD0(SRC0_SEL(SQ_ALU_SRC_PV),
		       SRC0_REL(ABSOLUTE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(0),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_OFF),
			   ALU_INST(SQ_OP2_INST_SETNE),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(256),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(0),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(257),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_X),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_X),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(257),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Y),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Y),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(257),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_Z),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(0)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_Z),
			   CLAMP(0)),
	    ALU_DWORD0(SRC0_SEL(257),
		       SRC0_REL(RELATIVE),
		       SRC0_ELEM(ELEM_W),
		       SRC0_NEG(0),
		       SRC1_SEL(SQ_ALU_SRC_0),
		       SRC1_REL(ABSOLUTE),
		       SRC1_ELEM(ELEM_X),
		       SRC1_NEG(0),
		       INDEX_MODE(SQ_INDEX_AR_X),
		       PRED_SEL(SQ_PRED_SEL_OFF),
		       LAST(1)),
	    ALU_DWORD1_OP2(CHIPSET_R600,  /* FIXME: Macro arguments are wrong for CHIPSET_RV770 */
			   SRC0_ABS(0),
			   SRC1_ABS(0),
			   UPDATE_EXECUTE_MASK(0),
			   UPDATE_PRED(0),
			   WRITE_MASK(1),
			   FOG_MERGE(0),
			   OMOD(SQ_ALU_OMOD_M4),
			   ALU_INST(SQ_OP2_INST_SETE_DX10),
			   BANK_SWIZZLE(SQ_ALU_VEC_012),
			   DST_GPR(1),
			   DST_REL(ABSOLUTE),
			   DST_ELEM(ELEM_W),
			   CLAMP(0)),
    };

    uint32_t ps[] = {
	    CF_DWORD0(ADDR(2)),
	    CF_DWORD1(POP_COUNT(0),
		      CF_CONST(0),
		      COND(SQ_CF_COND_ACTIVE),
		      COUNT(1),
		      CALL_COUNT(0),
		      END_OF_PROGRAM(0),
		      VALID_PIXEL_MODE(1),
		      CF_INST(SQ_CF_INST_TEX),
		      WHOLE_QUAD_MODE(0),
		      BARRIER(1)),
	    CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				    TYPE(SQ_EXPORT_PIXEL),
				    RW_GPR(0),
				    RW_REL(ABSOLUTE),
				    INDEX_GPR(0),
				    ELEM_SIZE(0)),
	    CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					 SRC_SEL_Y(SQ_SEL_Y),
					 SRC_SEL_Z(SQ_SEL_Z),
					 SRC_SEL_W(SQ_SEL_W),
					 R6xx_ELEM_LOOP(0),
					 BURST_COUNT(0),
					 END_OF_PROGRAM(1),
					 VALID_PIXEL_MODE(0),
					 CF_INST(SQ_CF_INST_EXPORT_DONE),
					 WHOLE_QUAD_MODE(0),
					 BARRIER(1)),
	    TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
		       BC_FRAC_MODE(0),
		       FETCH_WHOLE_QUAD(0),
		       RESOURCE_ID(0),
		       SRC_GPR(0),
		       SRC_REL(ABSOLUTE),
		       R7xx_ALT_CONST(0)),
	    TEX_DWORD1(DST_GPR(0),
		       DST_REL(ABSOLUTE),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_Z),
		       DST_SEL_W(SQ_SEL_W),
		       LOD_BIAS(0),
		       COORD_TYPE_X(TEX_NORMALIZED),
		       COORD_TYPE_Y(TEX_NORMALIZED),
		       COORD_TYPE_Z(TEX_NORMALIZED),
		       COORD_TYPE_W(TEX_NORMALIZED)),
	    TEX_DWORD2(OFFSET_X(0),
		       OFFSET_Y(0),
		       OFFSET_Z(0),
		       SAMPLER_ID(0),
		       SRC_SEL_X(SQ_SEL_X),
		       SRC_SEL_Y(SQ_SEL_Y),
		       SRC_SEL_Z(SQ_SEL_X),
		       SRC_SEL_W(SQ_SEL_X)),
	    TEX_DWORD_PAD,
    };

    printf ("\n* PM4 Play Blit Test r7xx\n\n");

    memset(&vs_conf, 0, sizeof(shader_config_t));
    memset(&ps_conf, 0, sizeof(shader_config_t));
    memset(&cb_conf, 0, sizeof(cb_config_t));
    memset(&tex_res, 0, sizeof(tex_resource_t));
    memset(&tex_samp, 0, sizeof(tex_sampler_t));

    draw_conf.prim_type = DI_PT_RECTLIST;
    draw_conf.num_indices = 4;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.index_type = DI_INDEX_SIZE_32_BIT;
    draw_conf.num_instances = 1;

    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);

    tex      = create_sample_texture (tex_w, tex_h, tex_p);
    tex_addr = upload (adapt, tex, tex_p * tex_h * sizeof(uint32_t), 8192);
    free (tex);

    printf("win blit r7xx\n");

    start_3d(adapt);

    wait_3d_idle_clean();

    cp_set_surface_sync();

    set_default_state(adapt);

    ereg  (CB_TARGET_MASK,                      TARGET0_ENABLE_mask);
    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);

    cb_conf.id = 0;
    cb_conf.w = surface_w;
    cb_conf.h = surface_h;
    cb_conf.base = adapt->display_gpu;
    cb_conf.format = 0x1a;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (DB_DEPTH_INFO,                       0);

    tex_res.id = 0;
    tex_res.w = tex_w;
    tex_res.h = tex_h;
    tex_res.pitch = tex_p;
    tex_res.depth = 0;
    tex_res.dim = 1;						//2D
    tex_res.base = tex_addr;
    tex_res.mip_base = tex_addr;
    tex_res.format = FMT_8_8_8_8;
    tex_res.request_size = 2;
    tex_res.dst_sel_x = 0;
    tex_res.dst_sel_y = 1;
    tex_res.dst_sel_z = 2;
    tex_res.dst_sel_w = 3;
    tex_res.last_level = 1;
    tex_res.perf_modulation = 1;
    set_tex_resource(adapt, &tex_res);

    set_generic_scissor(0, 0, 8192, 8192);

    vs_conf.shader_addr = vs_addr;
    vs_conf.num_gprs = 2;
    vs_conf.stack_size = 0;
    vs_setup(adapt, &vs_conf);

    ereg  (SPI_VS_OUT_CONFIG,                   (VS_PER_COMPONENT_bit |
						 ((1 - 1) << VS_EXPORT_COUNT_shift)));

    ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL,         (14 << VTX_REUSE_DEPTH_shift));
    ereg  (VGT_OUT_DEALLOC_CNTL,                (16 << DEALLOC_DIST_shift));

    ereg  (SPI_VS_OUT_ID_0,                     0x00000000);

    ps_conf.shader_addr = ps_addr;
    ps_conf.num_gprs = 1;
    ps_conf.stack_size = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.export_mode = 2;
    ps_setup(adapt, &ps_conf);

    ereg  (SPI_PS_IN_CONTROL_0,                 (((1 - 1) << NUM_INTERP_shift) |
						 (1 << BARYC_SAMPLE_CNTL_shift)));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);

    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);
    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);

    ereg  (DB_SHADER_CONTROL,                   (1 << Z_ORDER_shift)); /* EARLY_Z_THEN_LATE_Z */

    ereg  (SPI_PS_INPUT_CNTL_0,                 (1 << DEFAULT_VAL_shift));

    tex_samp.id = 0;
    set_tex_sampler (adapt, &tex_samp);

    wait_3d_idle();

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);

    ereg  (DB_DEBUG,                            0);

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);

    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MULTI_CHIP_CNTL,               0);


    ereg  (R7xx_SQ_DYN_GPR_CNTL_PS_FLUSH_REQ,   VS_PC_LIMIT_ENABLE_bit);

    ereg  (VGT_OUTPUT_PATH_CNTL,                (0 << PATH_SELECT_shift)); // VGT_OUTPATH_VTX_REUSE
    ereg  (VGT_PRIMITIVEID_EN,                  0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_EN,          0); // IB-based prims disabled
    ereg  (VGT_STRMOUT_EN,                      0); // strmout off
    ereg  (VGT_REUSE_OFF,                       0); // reuse on
    ereg  (VGT_VTX_CNT_EN,                      0); // auto index gen off
    ereg  (VGT_STRMOUT_BUFFER_EN,               0); // all strmout buffers disabled
    ereg  (VGT_GS_MODE,                         0); // GS off
    ereg  (VGT_MAX_VTX_INDX,                    draw_conf.num_indices);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);
    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);
    ereg  (VGT_MULTI_PRIM_IB_RESET_INDX,        0);

    set_screen_scissor(0, 0, 8192, 8192);
    ereg  (PA_SC_WINDOW_OFFSET,                 0);
    set_window_scissor(0, 0, 8192, 8192);

    pack3 (IT_PRED_EXEC, 1);
    e32   (0x01000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x02000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x04000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    pack3 (IT_PRED_EXEC, 1);
    e32   (0x08000003);
    ereg  (PA_SC_MODE_CNTL,                     (FORCE_EOV_CNTDWN_ENABLE_bit |
						 FORCE_EOV_REZ_ENABLE_bit));
    ereg  (PA_SC_AA_CONFIG,                     0);
    ereg  (PA_SC_AA_MASK,                       0xFFFFFFFF);
    ereg  (PA_SC_CLIPRECT_RULE,                 0x0000FFFF);

    set_vport_scissor(0, 0, 0, 8192, 8192);

    ereg  (PA_SC_VPORT_ZMIN_0,                  0x00000000); // 0.0
    ereg  (PA_SC_VPORT_ZMAX_0,                  0x3F800000); // 1.0
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);
    ereg  (PA_SU_SC_MODE_CNTL,                  FACE_bit);
    ereg  (PA_CL_VTE_CNTL,                      (VTX_XY_FMT_bit |
                                                 VTX_Z_FMT_bit));

    ereg  (PA_SU_POINT_SIZE,                    0);
    ereg  (PA_SU_POINT_MINMAX,                  0);
    pack0 (0x00028C0C, 4);
    efloat(1.0f);						// PA_CL_GB_VERT_CLIP_ADJ = 0x0000A303
    efloat(1.0f);						// PA_CL_GB_VERT_DISC_ADJ = 0x0000A304
    efloat(1.0f);						// PA_CL_GB_HORZ_CLIP_ADJ = 0x0000A305
    efloat(1.0f);						// PA_CL_GB_HORZ_DISC_ADJ = 0x0000A306
    ereg  (PA_SC_LINE_STIPPLE,                  0);
    ereg  (PA_SC_MPASS_PS_CNTL,                 0);
    ereg  (PA_SU_LINE_CNTL,                     (8 << PA_SU_LINE_CNTL__WIDTH_shift)); // 16.0 fixed
    ereg  (PA_SU_VTX_CNTL,                      ((PIX_CENTER_bit) |
						 (2 << PA_SU_VTX_CNTL__ROUND_MODE_shift)));
    ereg  (PA_SU_POLY_OFFSET_CLAMP,             0);
    ereg  (PA_CL_VS_OUT_CNTL,                   0);

    ereg  (SPI_INTERP_CONTROL_0,                0);
    ereg  (SPI_INPUT_Z,                         0);
    ereg  (SPI_FOG_CNTL,                        0);
    ereg  (SPI_FOG_FUNC_SCALE,                  0x00000000);
    ereg  (SPI_FOG_FUNC_BIAS,                   0x00000000);

    ereg  (SX_ALPHA_TEST_CONTROL,               0);
    ereg  (SX_ALPHA_REF,                        0x00000000); // 0.0

    ereg  (DB_ALPHA_TO_MASK,                    0);
    ereg  (DB_DEPTH_CONTROL,                    BACKFACE_ENABLE_bit);
    ereg  (DB_RENDER_OVERRIDE,                  0);
    ereg  (DB_SHADER_CONTROL,                   DUAL_EXPORT_ENABLE_bit);
    ereg  (DB_STENCILREFMASK,                   0);
    ereg  (DB_STENCILREFMASK_BF,                0);
    ereg  (DB_DEPTH_CLEAR,                      0); // float 0.0
    ereg  (DB_STENCIL_CLEAR,                    0);
    ereg  (DB_RENDER_CONTROL,                   ZPASS_INCREMENT_DISABLE_bit);
    ereg  (DB_SRESULTS_COMPARE_STATE0,          0);
    ereg  (DB_SRESULTS_COMPARE_STATE1,          0);

    ereg  (CB_TARGET_MASK,                      TARGET0_ENABLE_mask);
    ereg  (CB_SHADER_MASK,                      OUTPUT0_ENABLE_mask);
    ereg  (CB_CLRCMP_CONTROL,                   ((0 << CLRCMP_FCN_SRC_shift) |
						 (0 << CLRCMP_FCN_DST_shift) |
						 (2 << CLRCMP_FCN_SEL_shift)));
    ereg  (CB_CLRCMP_DST,                       0x00000000);
    ereg  (CB_CLRCMP_MSK,                       0xFFFFFFFF);
    pack0 (0x00028414, 4);
    efloat(0.0f);						// CB_BLEND_RED = 0x0000A105
    efloat(0.0f);						// CB_BLEND_GREEN = 0x0000A106
    efloat(0.0f);						// CB_BLEND_BLUE = 0x0000A107
    efloat(0.0f);						// CB_BLEND_ALPHA = 0x0000A108

    for (i = 0; i < CB_BLEND0_CONTROL_num; i++)
	    ereg  (CB_BLEND0_CONTROL + (i << 2),        (1 << COLOR_SRCBLEND_shift)); // BLEND_ONE
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); // copy

    ereg  (R7xx_PA_SC_EDGERULE,                 0xAAAAAAAA);
    ereg  (R7xx_CB_SHADER_CONTROL,              RT0_ENABLE_bit);

    ereg  (R7xx_SPI_THREAD_GROUPING,            (1 << PS_GROUPING_shift));

    // PS alu constants
    set_alu_consts(0x0 >> 2, sizeof(ps_alu_consts) / SQ_ALU_CONSTANT_offset, ps_alu_consts);

    // VS alu constants
    set_alu_consts(0x400 >> 2, sizeof(vs_alu_consts) / SQ_ALU_CONSTANT_offset, vs_alu_consts);

    draw_auto(adapt, &draw_conf);

    wait_3d_idle_clean();

}

void pm4play_blit_test(adapter_t *adapt)
{
    if (adapt->chipset <= CHIPSET_RV670)
	pm4play_blit_test_r6xx(adapt);
    else
	pm4play_blit_test_r7xx(adapt);
}
