
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

const char *rel[2] = {
	"ABSOLUTE",
	"RELATIVE"
};

const char *im[5] = {
	"SQ_INDEX_AR_X",
	"SQ_INDEX_AR_Y",
	"SQ_INDEX_AR_Z",
	"SQ_INDEX_AR_W",
	"SQ_INDEX_LOOP",
};

const char *ps[4] = {
	"SQ_PRED_SEL_OFF",
	"Reserved",
	"SQ_PRED_SEL_ZERO",
	"SQ_PRED_SEL_ONE"
};

const char *elem[4] = {
	"ELEM_X",
	"ELEM_Y",
	"ELEM_Z",
	"ELEM_W",
};

const char *omod[4] = {
	"SQ_ALU_OMOD_OFF",
	"SQ_ALU_OMOD_M2",
	"SQ_ALU_OMOD_M4",
	"SQ_ALU_OMOD_D2",
};

const char *bs[6] = {
	"SQ_ALU_VEC_012",
	"SQ_ALU_VEC_021",
	"SQ_ALU_VEC_120",
	"SQ_ALU_VEC_102",
	"SQ_ALU_VEC_201",
	"SQ_ALU_VEC_210",
};

const char *op2[122] = {
	"SQ_OP2_INST_ADD", //0
	"SQ_OP2_INST_MUL", //1
	"SQ_OP2_INST_MUL_IEEE", //2
	"SQ_OP2_INST_MAX", //3
	"SQ_OP2_INST_MIN", //4
	"SQ_OP2_INST_MAX_DX10", //5
	"SQ_OP2_INST_MIN_DX10", //6
	"reserved", //7
	"SQ_OP2_INST_SETE", //8
	"SQ_OP2_INST_SETGT", //9
	"SQ_OP2_INST_SETGE", //10
	"SQ_OP2_INST_SETNE", //11
	"SQ_OP2_INST_SETE_DX10", //12
	"SQ_OP2_INST_SETGT_DX10", //13
	"SQ_OP2_INST_SETGE_DX10", //14
	"SQ_OP2_INST_SETNE_DX10", //15
	"SQ_OP2_INST_FRACT", //16
	"SQ_OP2_INST_TRUNC", //17
	"SQ_OP2_INST_CEIL", //18
	"SQ_OP2_INST_RNDNE", //19
	"SQ_OP2_INST_FLOOR", //20
	"SQ_OP2_INST_MOVA", //21
	"SQ_OP2_INST_MOVA_FLOOR", //22
	"reserved", //23
	"SQ_OP2_INST_MOVA_INT", //24
	"SQ_OP2_INST_MOV", //25
	"SQ_OP2_INST_NOP", //26
	"reserved", //27
	"reserved", //28
	"reserved", //29
	"SQ_OP2_INST_PRED_SETGT_UINT", //30
	"SQ_OP2_INST_PRED_SETGE_UINT", //31
	"SQ_OP2_INST_PRED_SETE",  //32
	"SQ_OP2_INST_PRED_SETGT", //33
	"SQ_OP2_INST_PRED_SETGE", //34
	"SQ_OP2_INST_PRED_SETNE", //35
	"SQ_OP2_INST_PRED_SET_INV", //36
	"SQ_OP2_INST_PRED_SET_POP", //37
	"SQ_OP2_INST_PRED_SET_CLR", //38
	"SQ_OP2_INST_PRED_SET_RESTORE", //39
	"SQ_OP2_INST_PRED_SETE_PUSH", //40
	"SQ_OP2_INST_PRED_SETGT_PUSH", //41
	"SQ_OP2_INST_PRED_SETGE_PUSH", //42
	"SQ_OP2_INST_PRED_SETNE_PUSH", //43
	"SQ_OP2_INST_KILLE", //44
	"SQ_OP2_INST_KILLGT", //45
	"SQ_OP2_INST_KILLGE", //46
	"SQ_OP2_INST_KILLNE", //47
	"SQ_OP2_INST_AND_INT", //48
	"SQ_OP2_INST_OR_INT", //49
	"SQ_OP2_INST_XOR_INT", //50
	"SQ_OP2_INST_NOT_INT", //51
	"SQ_OP2_INST_ADD_INT", //52
	"SQ_OP2_INST_SUB_INT", //53
	"SQ_OP2_INST_MAX_INT", //54
	"SQ_OP2_INST_MIN_INT", //55
	"SQ_OP2_INST_MAX_UINT", //56
	"SQ_OP2_INST_MIN_UINT", //57
	"SQ_OP2_INST_SETE_INT", //58
	"SQ_OP2_INST_SETGT_INT", //59
	"SQ_OP2_INST_SETGE_INT", //60
	"SQ_OP2_INST_SETNE_INT", //61
	"SQ_OP2_INST_SETGT_UINT", //62
	"SQ_OP2_INST_SETGE_UINT", //63
	"SQ_OP2_INST_KILLGT_UINT", //64
	"SQ_OP2_INST_KILLGE_UINT", //65
	"SQ_OP2_INST_PRED_SETE_INT", //66
	"SQ_OP2_INST_PRED_SETGT_INT", //67
	"SQ_OP2_INST_PRED_SETGE_INT", //68
	"SQ_OP2_INST_PRED_SETNE_INT", //69
	"SQ_OP2_INST_PRED_SETLT_INT", //70
	"SQ_OP2_INST_PRED_SETLE_INT", //71
	"SQ_OP2_INST_KILLGE_INT", //72
	"SQ_OP2_INST_KILLNE_INT", //73
	"SQ_OP2_INST_PRED_SETE_PUSH_INT", //74
	"SQ_OP2_INST_PRED_SETGT_PUSH_INT", //75
	"SQ_OP2_INST_PRED_SETGE_PUSH_INT", //76
	"SQ_OP2_INST_PRED_SETNE_PUSH_INT", //77
	"SQ_OP2_INST_PRED_SETLT_PUSH_INT", //78
	"SQ_OP2_INST_PRED_SETLE_PUSH_INT", //79
	"SQ_OP2_INST_DOT4", //80
	"SQ_OP2_INST_DOT4_IEEE", //81
	"SQ_OP2_INST_CUBE", //82
	"SQ_OP2_INST_MAX4", //83
	"invalid", //84
	"invalid", //85
	"invalid", //86
	"invalid", //87
	"invalid", //88
	"invalid", //89
	"invalid", //90
	"invalid", //91
	"invalid", //92
	"invalid", //93
	"invalid", //94
	"invalid", //95
	"SQ_OP2_INST_MOVA_GPR_INT", //96
	"SQ_OP2_INST_EXP_IEEE", //97
	"SQ_OP2_INST_LOG_CLAMPED", //98
	"SQ_OP2_INST_LOG_IEEE", //99
	"SQ_OP2_INST_RECIP_CLAMPED", //100
	"SQ_OP2_INST_RECIP_FF", //101
	"SQ_OP2_INST_RECIP_IEEE", //102
	"SQ_OP2_INST_RECIPSQRT_CLAMPED", //103
	"SQ_OP2_INST_RECIPSQRT_FF", //104
	"SQ_OP2_INST_RECIPSQRT_IEEE", //105
	"SQ_OP2_INST_SQRT_IEEE", //106
	"SQ_OP2_INST_FLT_TO_INT", //107
	"SQ_OP2_INST_INT_TO_FLT", //108
	"SQ_OP2_INST_UINT_TO_FLT", //109
	"SQ_OP2_INST_SIN", //110
	"SQ_OP2_INST_COS", //111
	"SQ_OP2_INST_ASHR_INT", //112
	"SQ_OP2_INST_LSHR_INT", //113
	"SQ_OP2_INST_LSHL_INT", //114
	"SQ_OP2_INST_MULLO_INT", //115
	"SQ_OP2_INST_MULHI_INT", //116
	"SQ_OP2_INST_MULLO_UINT", //117
	"SQ_OP2_INST_MULHI_UINT", //118
	"SQ_OP2_INST_RECIP_INT", //119
	"SQ_OP2_INST_RECIP_UINT", //120
	"SQ_OP2_INST_FLT_TO_UINT", //121
};

const char *op3[32] = {
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"SQ_OP3_INST_MUL_LIT",
	"SQ_OP3_INST_MUL_LIT_M2",
	"SQ_OP3_INST_MUL_LIT_M4",
	"SQ_OP3_INST_MUL_LIT_D2",
	"SQ_OP3_INST_MULADD",
	"SQ_OP3_INST_MULADD_M2",
	"SQ_OP3_INST_MULADD_M4",
	"SQ_OP3_INST_MULADD_D2",
	"SQ_OP3_INST_MULADD_IEEE",
	"SQ_OP3_INST_MULADD_IEEE_M2",
	"SQ_OP3_INST_MULADD_IEEE_M4",
	"SQ_OP3_INST_MULADD_IEEE_D2",
	"SQ_OP3_INST_CNDE",
	"SQ_OP3_INST_CNDGT",
	"SQ_OP3_INST_CNDGE",
	"reserved",
	"SQ_OP3_INST_CNDE_INT",
	"SQ_OP3_INST_CNDGT_INT",
	"SQ_OP3_INST_CNDGE_INT",
	"reserved",
};

const char *cond[4] = {
	"SQ_CF_COND_ACTIVE",
	"SQ_CF_COND_FALSE",
	"SQ_CF_COND_BOOL",
	"SQ_CF_COND_NOT_BOOL",
};

const char *cf_op[25] = {
	"SQ_CF_INST_NOP",
	"SQ_CF_INST_TEX",
	"SQ_CF_INST_VTX",
	"SQ_CF_INST_VTX_TC",
	"SQ_CF_INST_LOOP_START",
	"SQ_CF_INST_LOOP_END",
	"SQ_CF_INST_START_DX10",
	"SQ_CF_INST_START_NO_AL",
	"SQ_CF_INST_LOOP_CONTINUE",
	"SQ_CF_INST_LOOP_BREAK",
	"SQ_CF_INST_JUMP",
	"SQ_CF_INST_PUSH",
	"SQ_CF_INST_PUSH_ELSE",
	"SQ_CF_INST_ELSE",
	"SQ_CF_INST_POP",
	"SQ_CF_INST_POP_JUMP",
	"SQ_CF_INST_POP_PUSH",
	"SQ_CF_INST_POP_PUSH_ELSE",
	"SQ_CF_INST_CALL",
	"SQ_CF_CALL_FS",
	"SQ_CF_INST_RETURN",
	"SQ_CF_INST_EMIT_VERTEX",
	"SQ_CF_INST_EMIT_CUT_VERTEX",
	"SQ_CF_INST_CUT_VERTEX",
	"SQ_CF_INST_KILL",
};


const char *cf_alu_op[16] = {
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"SQ_CF_INST_ALU",
	"SQ_CF_INST_ALU_PUSH_BEFORE",
	"SQ_CF_INST_ALU_POP_AFTER",
	"SQ_CF_INST_ALU_POP2_AFTER",
	"reserved",
	"SQ_CF_INST_ALU_CONTINUE",
	"SQ_CF_INST_ALU_BREAK",
	"SQ_CF_INST_ALU_ELSE_AFTER",
};

const char *cf_exp_op[41] = {
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"SQ_CF_INST_MEM_STREAM0",
	"SQ_CF_INST_MEM_STREAM1",
	"SQ_CF_INST_MEM_STREAM2",
	"SQ_CF_INST_MEM_STREAM3",
	"SQ_CF_INST_MEM_SCRATCH",
	"SQ_CF_INST_MEM_REDUCTION",
	"SQ_CF_INST_MEM_RING",
	"SQ_CF_INST_EXPORT",
	"SQ_CF_INST_EXPORT_DONE",
};

const char *km[4] = {
	"SQ_CF_KCACHE_NOP",
	"SQ_CF_KCACHE_LOCK_1",
	"SQ_CF_KCACHE_LOCK_2",
	"SQ_CF_KCACHE_LOCK_LOOP_INDEX",
};

const char *sel[8] = {
	"SQ_SEL_X",
	"SQ_SEL_Y",
	"SQ_SEL_Z",
	"SQ_SEL_W",
	"SQ_SEL_0",
	"SQ_SEL_1",
	"reserved",
	"SQ_SEL_MASK",
};

const char *cf_exp_type[8] = {
	"SQ_EXPORT_PIXEL",
	"SQ_EXPORT_POS",
	"SQ_EXPORT_PARAM",
	"unused",
};

const char *cf_mem_type[8] = {
	"SQ_EXPORT_WRITE",
	"SQ_EXPORT_WRITE_IND",
	"SQ_IMPORT_READ",
	"SQ_IMPORT_READ_IND",
};

const char *ab_pixel[62] = {
	"CF_PIXEL_MRT0",
	"CF_PIXEL_MRT1",
	"CF_PIXEL_MRT2",
	"CF_PIXEL_MRT3",
	"CF_PIXEL_MRT4",
	"CF_PIXEL_MRT5",
	"CF_PIXEL_MRT6",
	"CF_PIXEL_MRT7",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"CF_PIXEL_MRT0_FOG",
	"CF_PIXEL_MRT1_FOG",
	"CF_PIXEL_MRT2_FOG",
	"CF_PIXEL_MRT3_FOG",
	"CF_PIXEL_MRT4_FOG",
	"CF_PIXEL_MRT5_FOG",
	"CF_PIXEL_MRT6_FOG",
	"CF_PIXEL_MRT7_FOG",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"CF_PIXEL_Z",
};

const char *ab_pos[64] = {
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"invalid",
	"CF_POS0",
	"CF_POS1",
	"CF_POS2",
	"CF_POS3",
};

const char *tex_op[32] = {
	"SQ_TEX_INST_VTX_FETCH",
	"SQ_TEX_INST_VTX_SEMANTIC",
	"reserved",
	"SQ_TEX_INST_LD",
	"SQ_TEX_INST_GET_TEXTURE_RESINFO",
	"SQ_TEX_INST_GET_NUMBER_OF_SAMPLES",
	"SQ_TEX_INST_GET_LOD",
	"SQ_TEX_INST_GET_GRADIENTS_H",
	"SQ_TEX_INST_GET_GRADIENTS_V",
	"SQ_TEX_INST_GET_LERP",
	"reserved",
	"SQ_TEX_INST_SET_GRADIENTS_H",
	"SQ_TEX_INST_SET_GRADIENTS_V",
	"SQ_TEX_INST_PASS",
	"Z set index for array of cubemaps",
	"reserved",
	"SQ_TEX_INST_SAMPLE",
	"SQ_TEX_INST_SAMPLE_L",
	"SQ_TEX_INST_SAMPLE_LB",
	"SQ_TEX_INST_SAMPLE_LZ",
	"SQ_TEX_INST_SAMPLE_G",
	"SQ_TEX_INST_SAMPLE_G_L",
	"SQ_TEX_INST_SAMPLE_G_LB",
	"SQ_TEX_INST_SAMPLE_G_LZ",
	"SQ_TEX_INST_SAMPLE_C",
	"SQ_TEX_INST_SAMPLE_C_L",
	"SQ_TEX_INST_SAMPLE_C_LB",
	"SQ_TEX_INST_SAMPLE_C_LZ",
	"SQ_TEX_INST_SAMPLE_C_G",
	"SQ_TEX_INST_SAMPLE_C_G_L",
	"SQ_TEX_INST_SAMPLE_C_G_LB",
	"SQ_TEX_INST_SAMPLE_C_G_LZ",
};

const char *coord_type[2] = {
	"TEX_UNNORMALIZED",
	"TEX_NORMALIZED",
};

const char *vtx_op[2] = {
	"SQ_VTX_INST_FETCH",
	"SQ_VTX_INST_SEMANTIC",
};

const char *fetch_type[3] = {
	"SQ_VTX_FETCH_VERTEX_DATA",
	"SQ_VTX_FETCH_INSTANCE_DATA",
	"SQ_VTX_FETCH_NO_INDEX_OFFSET",
};

const char *num_format_all[3] = {
	"SQ_NUM_FORMAT_NORM",
	"SQ_NUM_FORMAT_INT",
	"SQ_NUM_FORMAT_SCALED",
};

const char *format_comp_all[2] = {
	"SQ_FORMAT_COMP_UNSIGNED",
	"SQ_FORMAT_COMP_SIGNED",
};

const char *srf_mode_all[2] = {
	"SRF_MODE_ZERO_CLAMP_MINUS_ONE",
	"SRF_MODE_NO_ZERO",
};

const char *endian_swap[3] = {
	"SQ_ENDIAN_NONE",
	"SQ_ENDIAN_8IN16",
	"SQ_ENDIAN_8IN32",
};

void alu_dword1_op3(uint32_t dword1, int is_r700)
{
	int src2_sel, src2_rel;
	int src2_elem, src2_neg;
	int alu_inst;
	int bank_swizzle, dst_gpr, dst_rel;
	int dst_elem, clamp;

	src2_sel = (dword1 >> 0) & 0x1ff;
	src2_rel = (dword1 >> 9) & 0x1;
	src2_elem = (dword1 >> 10) & 0x3;
	src2_neg = (dword1 >> 12) & 0x1;

	printf("ALU_DWORD1_OP3(");
	if (is_r700) {
		if (src2_sel == 248)
			printf("SRC2_SEL(SQ_ALU_SRC_0),\n");
		else if (src2_sel == 249)
			printf("SRC2_SEL(SQ_ALU_SRC_1),\n");
		else if (src2_sel == 250)
			printf("SRC2_SEL(SQ_ALU_SRC_I_INT),\n");
		else if (src2_sel == 251)
			printf("SRC2_SEL(SQ_ALU_SRC_M_1_INT),\n");
		else if (src2_sel == 252)
			printf("SRC2_SEL(SQ_ALU_SRC_0_5),\n");
		else if (src2_sel == 253)
			printf("SRC2_SEL(SQ_ALU_SRC_LITERAL),\n");
		else if (src2_sel == 254)
			printf("SRC2_SEL(SQ_ALU_SRC_PV),\n");
		else if (src2_sel == 255)
			printf("SRC2_SEL(SQ_ALU_SRC_PS),\n");
		else
			printf("SRC2_SEL(%d),\n", src2_sel);
	} else {
		if (src2_sel == 248)
			printf("SRC2_SEL(ALU_SRC_0),\n");
		else if (src2_sel == 249)
			printf("SRC2_SEL(ALU_SRC_1),\n");
		else if (src2_sel == 250)
			printf("SRC2_SEL(ALU_SRC_I_INT),\n");
		else if (src2_sel == 251)
			printf("SRC2_SEL(ALU_SRC_M_1_INT),\n");
		else if (src2_sel == 252)
			printf("SRC2_SEL(ALU_SRC_0_5),\n");
		else if (src2_sel == 253)
			printf("SRC2_SEL(ALU_SRC_LITERAL),\n");
		else if (src2_sel == 254)
			printf("SRC2_SEL(ALU_SRC_PV),\n");
		else if (src2_sel == 255)
			printf("SRC2_SEL(ALU_SRC_PS),\n");
		else
			printf("SRC2_SEL(%d),\n", src2_sel);
	}
	printf("\tSRC2_REL(%s),\n", rel[src2_rel]);
	printf("\tSRC2_ELEM(%s),\n", elem[src2_elem]);
	printf("\tSRC2_NEG(%d),\n", src2_neg);

	alu_inst = (dword1 >> 13) & 0x1f;
	printf("\tALU_INST(%s),\n", op3[alu_inst]);
	bank_swizzle = (dword1 >> 18) & 0x7;
	printf("\tBANK_SWIZZLE(%s),\n", bs[bank_swizzle]);
	dst_gpr = (dword1 >> 21) & 0x7f;
	printf("\tDST_GPR(%d),\n", dst_gpr);
	dst_rel = (dword1 >> 28) & 0x1;
	printf("\tDST_REL(%s),\n", rel[dst_rel]);
	dst_elem = (dword1 >> 29) & 0x3;
	printf("\tDST_ELEM(%s),\n", elem[dst_elem]);
	clamp = (dword1 >> 31) & 0x1;
	printf("\tCLAMP(%d)", clamp);
	printf("),\n");
}

void alu_dword1_op2(uint32_t dword1, int is_r700)
{
	int src0_abs, src1_abs;
	int update_execute_mask, update_pred;
	int write_mask, fog_merge;
	int outmod, alu_inst;
	int bank_swizzle, dst_gpr, dst_rel;
	int dst_elem, clamp;

	printf("ALU_DWORD1_OP2(");
	printf("adapt->chipset,\n");
	src0_abs = (dword1 >> 0) & 0x1;
	printf("\tSRC0_ABS(%d),\n", src0_abs);
	src1_abs = (dword1 >> 1) & 0x1;
	printf("\tSRC1_ABS(%d),\n", src1_abs);
	update_execute_mask = (dword1 >> 2) & 0x1;
	printf("\tUPDATE_EXECUTE_MASK(%d),\n", update_execute_mask);
	update_pred = (dword1 >> 3) & 0x1;
	printf("\tUPDATE_PRED(%d),\n", update_pred);
	write_mask = (dword1 >> 4) & 0x1;
	printf("\tWRITE_MASK(%d),\n", write_mask);
	if (is_r700) {
		printf("\tFOG_MERGE(0),\n");
		outmod = (dword1 >> 5) & 0x3;
		printf("\tOMOD(%s),\n", omod[outmod]);
		alu_inst = (dword1 >> 7) & 0x7ff;
		printf("\tALU_INST(%s),\n", op2[alu_inst]);
	} else {
		fog_merge = (dword1 >> 5) & 0x1;
		printf("\tFOG_MERGE(%d),\n", fog_merge);
		outmod = (dword1 >> 6) & 0x3;
		printf("\tOMOD(%s),\n", omod[outmod]);
		alu_inst = (dword1 >> 8) & 0x3ff;
		printf("\tALU_INST(%s),\n", op2[alu_inst]);
	}
	bank_swizzle = (dword1 >> 18) & 0x7;
	printf("\tBANK_SWIZZLE(%s),\n", bs[bank_swizzle]);
	dst_gpr = (dword1 >> 21) & 0x7f;
	printf("\tDST_GPR(%d),\n", dst_gpr);
	dst_rel = (dword1 >> 28) & 0x1;
	printf("\tDST_REL(%s),\n", rel[dst_rel]);
	dst_elem = (dword1 >> 29) & 0x3;
	printf("\tDST_ELEM(%s),\n", elem[dst_elem]);
	clamp = (dword1 >> 31) & 0x1;
	printf("\tCLAMP(%d)", clamp);
	printf("),\n");
}

void alu_dword0(uint32_t dword0, int is_r700)
{
	int src0_sel, src1_sel;
	int src0_rel, src1_rel;
	int src0_elem, src1_elem;
	int src0_neg, src1_neg;
	int index_mode, pred_sel, last;

	src0_sel = (dword0 >> 0) & 0x1ff;
	src1_sel = (dword0 >> 13) & 0x1ff;

	src0_rel = (dword0 >> 9) & 0x1;
	src1_rel = (dword0 >> 22) & 0x1;

	src0_elem = (dword0 >> 10) & 0x3;
	src1_elem = (dword0 >> 23) & 0x3;

	src0_neg = (dword0 >> 12) & 0x1;
	src1_neg = (dword0 >> 25) & 0x1;

	index_mode = (dword0 >> 26) & 0x7;
	pred_sel = (dword0 >> 29) & 0x3;
	last = (dword0 >> 31) & 0x1;

	printf("ALU_DWORD0(");
	if (is_r700) {
		if (src0_sel == 248)
			printf("SRC0_SEL(SQ_ALU_SRC_0),\n");
		else if (src0_sel == 249)
			printf("SRC0_SEL(SQ_ALU_SRC_1),\n");
		else if (src0_sel == 250)
			printf("SRC0_SEL(SQ_ALU_SRC_I_INT),\n");
		else if (src0_sel == 251)
			printf("SRC0_SEL(SQ_ALU_SRC_M_1_INT),\n");
		else if (src0_sel == 252)
			printf("SRC0_SEL(SQ_ALU_SRC_0_5),\n");
		else if (src0_sel == 253)
			printf("SRC0_SEL(SQ_ALU_SRC_LITERAL),\n");
		else if (src0_sel == 254)
			printf("SRC0_SEL(SQ_ALU_SRC_PV),\n");
		else if (src0_sel == 255)
			printf("SRC0_SEL(SQ_ALU_SRC_PS),\n");
		else
			printf("SRC0_SEL(%d),\n", src0_sel);
	} else {
		if (src0_sel == 248)
			printf("SRC0_SEL(SQ_ALU_SRC_0),\n");
		else if (src0_sel == 249)
			printf("SRC0_SEL(SQ_ALU_SRC_1),\n");
		else if (src0_sel == 250)
			printf("SRC0_SEL(SQ_ALU_SRC_I_INT),\n");
		else if (src0_sel == 251)
			printf("SRC0_SEL(SQ_ALU_SRC_M_1_INT),\n");
		else if (src0_sel == 252)
			printf("SRC0_SEL(SQ_ALU_SRC_0_5),\n");
		else if (src0_sel == 253)
			printf("SRC0_SEL(SQ_ALU_SRC_LITERAL),\n");
		else if (src0_sel == 254)
			printf("SRC0_SEL(SQ_ALU_SRC_PV),\n");
		else if (src0_sel == 255)
			printf("SRC0_SEL(SQ_ALU_SRC_PS),\n");
		else
			printf("SRC0_SEL(%d),\n", src0_sel);
	}
	printf("\tSRC0_REL(%s),\n", rel[src0_rel]);
	printf("\tSRC0_ELEM(%s),\n", elem[src0_elem]);
	printf("\tSRC0_NEG(%d),\n", src0_neg);

	if (is_r700) {
		if (src1_sel == 248)
			printf("\tSRC1_SEL(SQ_ALU_SRC_0),\n");
		else if (src1_sel == 249)
			printf("\tSRC1_SEL(SQ_ALU_SRC_1),\n");
		else if (src1_sel == 250)
			printf("\tSRC1_SEL(SQ_ALU_SRC_I_INT),\n");
		else if (src1_sel == 251)
			printf("\tSRC1_SEL(SQ_ALU_SRC_M_1_INT),\n");
		else if (src1_sel == 252)
			printf("\tSRC0_SEL(SQ_ALU_SRC_0_5),\n");
		else if (src1_sel == 253)
			printf("\tSRC1_SEL(SQ_ALU_SRC_LITERAL),\n");
		else if (src1_sel == 254)
			printf("\tSRC1_SEL(SQ_ALU_SRC_PV),\n");
		else if (src1_sel == 255)
			printf("\tSRC1_SEL(SQ_ALU_SRC_PS),\n");
		else
			printf("\tSRC1_SEL(%d),\n", src1_sel);
	} else {
		if (src1_sel == 248)
			printf("\tSRC1_SEL(SQ_ALU_SRC_0),\n");
		else if (src1_sel == 249)
			printf("\tSRC1_SEL(SQ_ALU_SRC_1),\n");
		else if (src1_sel == 250)
			printf("\tSRC1_SEL(SQ_ALU_SRC_I_INT),\n");
		else if (src1_sel == 251)
			printf("\tSRC1_SEL(SQ_ALU_SRC_M_1_INT),\n");
		else if (src1_sel == 252)
			printf("\tSRC0_SEL(SQ_ALU_SRC_0_5),\n");
		else if (src1_sel == 253)
			printf("\tSRC1_SEL(SQ_ALU_SRC_LITERAL),\n");
		else if (src1_sel == 254)
			printf("\tSRC1_SEL(SQ_ALU_SRC_PV),\n");
		else if (src1_sel == 255)
			printf("\tSRC1_SEL(SQ_ALU_SRC_PS),\n");
		else
			printf("\tSRC1_SEL(%d),\n", src1_sel);
	}

	printf("\tSRC1_REL(%s),\n", rel[src1_rel]);
	printf("\tSRC1_ELEM(%s),\n", elem[src1_elem]);
	printf("\tSRC1_NEG(%d),\n", src1_neg);
	printf("\tINDEX_MODE(%s),\n", im[index_mode]);
	printf("\tPRED_SEL(%s),\n", ps[pred_sel]);
	printf("\tLAST(%d)", last);
	printf("),\n");
}

void tex_dword0(uint32_t dword0, int is_r700)
{
	int inst, bfm, fwq;
	int resource_id, src_gpr;
	int sr, ac;

	printf("TEX_DWORD0(");
	inst = (dword0 >> 0) & 0x1f;
	printf("TEX_INST(%s),\n", tex_op[inst]);
	bfm = (dword0 >> 5) & 0x1;
	printf("\tBC_FRAC_MODE(%d),\n", bfm);
	fwq = (dword0 >> 7) & 0x1;
	printf("\tFETCH_WHOLE_QUAD(%d),\n", fwq);
	resource_id = (dword0 >> 8) & 0xff;
	printf("\tRESOURCE_ID(%d),\n", resource_id);
	src_gpr = (dword0 >> 16) & 0x7f;
	printf("\tSRC_GPR(%d),\n", src_gpr);
	sr = (dword0 >> 23) & 0x1;
	printf("\tSRC_REL(%s),\n", rel[sr]);
	if (is_r700) {
		ac = (dword0 >> 24) & 0x1;
		printf("\tR7xx_ALT_CONST(%d)", ac);
	} else {
		printf("\tR7xx_ALT_CONST(0)");
	}
	printf("),\n");
}

void tex_dword1(uint32_t dword1, int is_r700)
{
	int dst_gpr, dr;
	int dsx, dsy, dsz, dsw;
	int lod_bias;
	int ctx, cty, ctz, ctw;

	printf("TEX_DWORD1(");
	dst_gpr = (dword1 >> 0) & 0x7f;
	printf("DST_GPR(%d),\n", dst_gpr);
	dr = (dword1 >> 7) & 0x1;
	printf("\tDST_REL(%s),\n", rel[dr]);
	dsx = (dword1 >> 9) & 0x7;
	printf("\tDST_SEL_X(%s),\n", sel[dsx]);
	dsy = (dword1 >> 12) & 0x7;
	printf("\tDST_SEL_Y(%s),\n", sel[dsy]);
	dsz = (dword1 >> 15) & 0x7;
	printf("\tDST_SEL_Z(%s),\n", sel[dsz]);
	dsw = (dword1 >> 18) & 0x7;
	printf("\tDST_SEL_W(%s),\n", sel[dsw]);
	lod_bias = (dword1 >> 21) & 0x7f;
	printf("\tLOD_BIAS(%d),\n", lod_bias);
	ctx = (dword1 >> 28) & 0x1;
	printf("\tCOORD_TYPE_X(%s),\n", coord_type[ctx]);
	cty = (dword1 >> 29) & 0x1;
	printf("\tCOORD_TYPE_Y(%s),\n", coord_type[cty]);
	ctz = (dword1 >> 30) & 0x1;
	printf("\tCOORD_TYPE_Z(%s),\n", coord_type[ctz]);
	ctw = (dword1 >> 31) & 0x1;
	printf("\tCOORD_TYPE_W(%s)", coord_type[ctw]);
	printf("),\n");
}

void tex_dword2(uint32_t dword2, int is_r700)
{
	int ox, oy, oz;
	int sampler_id;
	int ssx, ssy, ssz, ssw;

	printf("TEX_DWORD2(");
	ox = (dword2 >> 0) & 0x1f;
	printf("OFFSET_X(%d),\n", ox);
	oy = (dword2 >> 5) & 0x1f;
	printf("\tOFFSET_Y(%d),\n", oy);
	oz = (dword2 >> 10) & 0x1f;
	printf("\tOFFSET_Z(%d),\n", oz);
	sampler_id = (dword2 >> 15) & 0x1f;
	printf("\tSAMPLER_ID(%d),\n", sampler_id);
	ssx = (dword2 >> 20) & 0x7;
	printf("\tSRC_SEL_X(%s),\n", sel[ssx]);
	ssy = (dword2 >> 23) & 0x7;
	printf("\tSRC_SEL_Y(%s),\n", sel[ssy]);
	ssz = (dword2 >> 26) & 0x7;
	printf("\tSRC_SEL_Z(%s),\n", sel[ssz]);
	ssw = (dword2 >> 29) & 0x7;
	printf("\tSRC_SEL_W(%s)", sel[ssw]);
	printf("),\n");
}

void tex_dword_pad()
{
	printf("TEX_DWORD_PAD,\n");
}

void vtx_dword0(uint32_t dword0, int is_r700)
{
	int inst, ft, fwq;
	int buffer_id, src_gpr;
	int sr, ssx, mfc;

	printf("VTX_DWORD0(");
	inst = (dword0 >> 0) & 0x1f;
	printf("VTX_INST(%s),\n", vtx_op[inst]);
	ft = (dword0 >> 5) & 0x3;
	printf("\tFETCH_TYPE(%s),\n", fetch_type[ft]);
	fwq = (dword0 >> 7) & 0x1;
	printf("\tFETCH_WHOLE_QUAD(%d),\n", fwq);
	buffer_id = (dword0 >> 8) & 0xff;
	printf("\tBUFFER_ID(%d),\n", buffer_id);
	src_gpr = (dword0 >> 16) & 0x7f;
	printf("\tSRC_GPR(%d),\n", src_gpr);
	sr = (dword0 >> 23) & 0x1;
	printf("\tSRC_REL(%s),\n", rel[sr]);
	ssx = (dword0 >> 24) & 0x3;
	printf("\tSRC_SEL_X(%s),\n", sel[ssx]);
	mfc = (dword0 >> 26) & 0x3f;
	mfc++;
	printf("\tMEGA_FETCH_COUNT(%d)", mfc);
	printf("),\n");
}

void vtx_dword1_sem(uint32_t dword1, int is_r700)
{
	int semantic_id;
	int dsx, dsy, dsz, dsw;
	int ucf, df, nfa, fca, sma;

	printf("VTX_DWORD1_SEM(");
	semantic_id = (dword1 >> 0) & 0xff;
	printf("SEMANTIC_ID(%d),\n", semantic_id);
	dsx = (dword1 >> 9) & 0x7;
	printf("\tDST_SEL_X(%s),\n", sel[dsx]);
	dsy = (dword1 >> 12) & 0x7;
	printf("\tDST_SEL_Y(%s),\n", sel[dsy]);
	dsz = (dword1 >> 15) & 0x7;
	printf("\tDST_SEL_Z(%s),\n", sel[dsz]);
	dsw = (dword1 >> 18) & 0x7;
	printf("\tDST_SEL_W(%s),\n", sel[dsw]);
	ucf = (dword1 >> 21) & 0x1;
	printf("\tUSE_CONST_FIELDS(%d),\n", ucf);
	df = (dword1 >> 22) & 0x3f;
	printf("\tDATA_FORMAT(%d),\n", df);
	nfa = (dword1 >> 28) & 0x3;
	printf("\tNUM_FORMAT_ALL(%s),\n", num_format_all[nfa]);
	fca = (dword1 >> 30) & 0x1;
	printf("\tFORMAT_COMP_ALL(%s),\n", format_comp_all[fca]);
	sma = (dword1 >> 31) & 0x1;
	printf("\tSRF_MODE_ALL(%s)", srf_mode_all[sma]);
	printf("),\n");
}

void vtx_dword1_gpr(uint32_t dword1, int is_r700)
{
	int dst_gpr, dr;
	int dsx, dsy, dsz, dsw;
	int ucf, df, nfa, fca, sma;

	printf("VTX_DWORD1_GPR(");
	dst_gpr = (dword1 >> 0) & 0x7f;
	printf("DST_GPR(%d),\n", dst_gpr);
	dr = (dword1 >> 7) & 0x1;
	printf("\tDST_REL(%s),\n", rel[dr]);
	dsx = (dword1 >> 9) & 0x7;
	printf("\tDST_SEL_X(%s),\n", sel[dsx]);
	dsy = (dword1 >> 12) & 0x7;
	printf("\tDST_SEL_Y(%s),\n", sel[dsy]);
	dsz = (dword1 >> 15) & 0x7;
	printf("\tDST_SEL_Z(%s),\n", sel[dsz]);
	dsw = (dword1 >> 18) & 0x7;
	printf("\tDST_SEL_W(%s),\n", sel[dsw]);
	ucf = (dword1 >> 21) & 0x1;
	printf("\tUSE_CONST_FIELDS(%d),\n", ucf);
	df = (dword1 >> 22) & 0x3f;
	printf("\tDATA_FORMAT(%d),\n", df);
	nfa = (dword1 >> 28) & 0x3;
	printf("\tNUM_FORMAT_ALL(%s),\n", num_format_all[nfa]);
	fca = (dword1 >> 30) & 0x1;
	printf("\tFORMAT_COMP_ALL(%s),\n", format_comp_all[fca]);
	sma = (dword1 >> 31) & 0x1;
	printf("\tSRF_MODE_ALL(%s)", srf_mode_all[sma]);
	printf("),\n");
}

void vtx_dword2(uint32_t dword2, int is_r700)
{
	int offset, es;
	int cbns, mf;

	printf("VTX_DWORD2(");
	offset = (dword2 >> 0) & 0xffff;
	printf("OFFSET(%d),\n", offset);
	es = (dword2 >> 16) & 0x3;
	printf("\tENDIAN_SWAP(%s),\n", endian_swap[es]);
	cbns = (dword2 >> 18) & 0x1;
	printf("\tCONST_BUF_NO_STRIDE(%d),\n", cbns);
	mf = (dword2 >> 19) & 0x1;
	printf("\tMEGA_FETCH(%d),\n", mf);
	printf("),\n");
}

void vtx_dword_pad()
{
	printf("VTX_DWORD_PAD,\n");
}

void cf_dword0(uint32_t dword0, int is_r700)
{
	printf("CF_DWORD0(");
	printf("ADDR(%d)", dword0);
	printf("),\n");
}

void cf_dword1(uint32_t dword1, int is_r700)
{
	int pop_count, cf_const;
	int cf_cond, count, call_count;
	int eop, vpm, cf_inst, wqm;
	int barrier;

	printf("CF_DWORD1(");
	pop_count = (dword1 >> 0) & 0x7;
	printf("POP_COUNT(%d),\n", pop_count);
	cf_const = (dword1 >> 3) & 0x1f;
	printf("\tCF_CONST(%d),\n", cf_const);
	cf_cond = (dword1 >> 8) & 0x3;
	printf("\tCOND(%s),\n", cond[cf_cond]);
	count = (dword1 >> 10) & 0x7;
	count++;
	printf("\tCOUNT(%d),\n", count);
	call_count = (dword1 >> 13) & 0x3f;
	printf("\tCALL_COUNT(%d),\n", call_count);
	eop = (dword1 >> 21) & 0x1;
	printf("\tEND_OF_PROGRAM(%d),\n", eop);
	vpm = (dword1 >> 22) & 0x1;
	printf("\tVALID_PIXEL_MODE(%d),\n", vpm);
	cf_inst = (dword1 >> 23) & 0x7f;
	printf("\tCF_INST(%s),\n", cf_op[cf_inst]);
	wqm = (dword1 >> 30) & 0x1;
	printf("\tWHOLE_QUAD_MODE(%d),\n", wqm);
	barrier = (dword1 >> 31) & 0x1;
	printf("\tBARRIER(%d)", barrier);
	printf("),\n");
}

void cf_alu_dword0(uint32_t dword0, int is_r700)
{
	int kb0, kb1, km0;

	kb0 = (dword0 >> 22) & 0xf;
	kb1 = (dword0 >> 26) & 0xf;
	km0 = (dword0 >> 30) & 0x3;

	printf("CF_ALU_DWORD0(");
	printf("ADDR(%d),\n", dword0 & 0x3fffff);
	printf("\tKCACHE_BANK0(%d),\n", kb0);
	printf("\tKCACHE_BANK1(%d),\n", kb1);
	printf("\tKCACHE_MODE0(%s)", km[km0]);
	printf("),\n");
}

void cf_alu_dword1(uint32_t dword1, int is_r700)
{
	int km1, ka0, ka1, count, uw;
	int cf_inst, wqm, barrier;

	printf("CF_ALU_DWORD1(");
	km1 = (dword1 >> 0) & 0x3;
	printf("KCACHE_MODE1(%s),\n", km[km1]);
	ka0 = (dword1 >> 2) & 0xff;
	printf("\tKCACHE_ADDR0(%d),\n", ka0);
	ka1 = (dword1 >> 10) & 0xff;
	printf("\tKCACHE_ADDR1(%d),\n", ka1);
	count = (dword1 >> 18) & 0x7f;
	count++;
	printf("\tCOUNT(%d),\n", count);
	uw = (dword1 >> 25) & 0x1;
	printf("\tUSES_WATERFALL(%d),\n", uw);
	cf_inst = (dword1 >> 26) & 0xf;
	printf("\tCF_INST(%s),\n", cf_alu_op[cf_inst]);
	wqm = (dword1 >> 30) & 0x1;
	printf("\tWHOLE_QUAD_MODE(%d),\n", wqm);
	barrier = (dword1 >> 31) & 0x1;
	printf("\tBARRIER(%d)", barrier);
	printf("),\n");
}

void cf_alloc_imp_exp_dword0(uint32_t dword0, int is_mem, int is_r700)
{
	int array_base, type, rw_gpr;
	int rw_rel, index_gpr, elem_size;

	printf("CF_ALLOC_IMP_EXP_DWORD0(");
	array_base = (dword0 >> 0) & 0x1fff;
	type = (dword0 >> 13) & 0x3;
	if (is_mem) {
		printf("ARRAY_BASE(%d),\n", array_base);
		printf("\tTYPE(%s),\n", cf_mem_type[type]);
	} else {
		if (type == 0)
			printf("ARRAY_BASE(%s),\n", ab_pixel[array_base]);
		else if (type == 1)
			printf("ARRAY_BASE(%s),\n", ab_pos[array_base]);
		else
			printf("ARRAY_BASE(%d),\n", array_base);
		printf("\tTYPE(%s),\n", cf_exp_type[type]);
	}
	rw_gpr = (dword0 >> 15) & 0x7f;
	printf("\tRW_GPR(%d),\n", rw_gpr);
	rw_rel = (dword0 >> 22) & 0x1;
	printf("\tRW_REL(%s),\n", rel[rw_rel]);
	index_gpr = (dword0 >> 23) & 0x7f;
	printf("\tINDEX_GPR(%d),\n", index_gpr);
	elem_size = (dword0 >> 30) & 0x3;
	printf("\tELEM_SIZE(%d)", elem_size);
	printf("),\n");
}

void cf_alloc_imp_exp_dword1_buf(uint32_t dword1, int is_r700)
{
	int array_size, comp_mask, elem_loop;
	int bc, eop, vpm, cf_inst, wqm, barrier;

	printf("CF_ALLOC_IMP_EXP_DWORD1_BUF(");
	array_size = (dword1 >> 0) & 0xfff;
	printf("ARRAY_SIZE(%d),\n", array_size);
	comp_mask = (dword1 >> 12) & 0xf;
	printf("\tCOMP_MASK(%d),\n", comp_mask);
	if (is_r700) {
		printf("\tR6xx_ELEM_LOOP(0),\n");
	} else {
		elem_loop = (dword1 >> 16) & 0x1;
		printf("\tR6xx_ELEM_LOOP(%d),\n", elem_loop);
	}
	bc = (dword1 >> 17) & 0xf;
	printf("\tBURST_COUNT(%d),\n", bc);
	eop = (dword1 >> 21) & 0x1;
	printf("\tEND_OF_PROGRAM(%d),\n", eop);
	vpm = (dword1 >> 22) & 0x1;
	printf("\tVALID_PIXEL_MODE(%d),\n", vpm);
	cf_inst = (dword1 >> 23) & 0x7f;
	printf("\tCF_INST(%s),\n", cf_exp_op[cf_inst]);
	wqm = (dword1 >> 30) & 0x1;
	printf("\tWHOLE_QUAD_MODE(%d),\n", wqm);
	barrier = (dword1 >> 31) & 0x1;
	printf("\tBARRIER(%d)", barrier);
	printf("),\n");

}

void cf_alloc_imp_exp_dword1_swiz(uint32_t dword1, int is_r700)
{
	int sel_x, sel_y, sel_z, sel_w, elem_loop;
	int bc, eop, vpm, cf_inst, wqm, barrier;

	printf("CF_ALLOC_IMP_EXP_DWORD1_SWIZ(");
	sel_x = (dword1 >> 0) & 0x7;
	printf("SRC_SEL_X(%s),\n", sel[sel_x]);
	sel_y = (dword1 >> 3) & 0x7;
	printf("\tSRC_SEL_Y(%s),\n", sel[sel_y]);
	sel_z = (dword1 >> 6) & 0x7;
	printf("\tSRC_SEL_Z(%s),\n", sel[sel_z]);
	sel_w = (dword1 >> 9) & 0x7;
	printf("\tSRC_SEL_W(%s),\n", sel[sel_w]);
	elem_loop = (dword1 >> 16) & 0x1;
	printf("\tELEM_LOOP(%d),\n", elem_loop);
	bc = (dword1 >> 17) & 0xf;
	printf("\tBURST_COUNT(%d),\n", bc);
	eop = (dword1 >> 21) & 0x1;
	printf("\tEND_OF_PROGRAM(%d),\n", eop);
	vpm = (dword1 >> 22) & 0x1;
	printf("\tVALID_PIXEL_MODE(%d),\n", vpm);
	cf_inst = (dword1 >> 23) & 0x7f;
	printf("\tCF_INST(%s),\n", cf_exp_op[cf_inst]);
	wqm = (dword1 >> 30) & 0x1;
	printf("\tWHOLE_QUAD_MODE(%d),\n", wqm);
	barrier = (dword1 >> 31) & 0x1;
	printf("\tBARRIER(%d)", barrier);
	printf("),\n");

}

void do_alu_clause(uint32_t *inst, int offset, int count, int is_r700)
{
	uint32_t dword0, dword1;
	int op3;
	int src0, src1, src2;
	int elem0, elem1, elem2;
	int inline_const_count = 0;

	while (count) {
		dword0 = inst[offset];
		dword1 = inst[offset + 1];
		if (inline_const_count) {
			printf("0x%08x,\n",dword0);
			printf("0x%08x,\n",dword1);
			inline_const_count--;
		} else {
			src0 = (dword0 >> 0) & 0x1ff;
			src1 = (dword0 >> 13) & 0x1ff;
			elem0 = (dword0 >> 10) & 0x3;
			elem1 = (dword0 >> 23) & 0x3;
			alu_dword0(dword0, is_r700);
			// if high 3 bits of inst are 000b, OP2, else OP3
			op3 = (dword1 >> 15) & 0x7;
			if (op3) {
				alu_dword1_op3(dword1, is_r700);
				src2 = (dword1 >> 0) & 0x1ff;
				elem2 = (dword1 >> 10) & 0x3;
			} else {
				alu_dword1_op2(dword1, is_r700);
				src2 = 0;
				elem2 = 0;
			}

			// inline constants
			if ((src0 == 253) || (src1 == 253) || (src2 == 253)) {
				if ((elem0 > 1) || (elem1 > 1) || (elem2 > 1))
					inline_const_count = 2; // 3-4 constants
				else
					inline_const_count = 1; // 1-2 constants
			}
		}
		count--;
		offset += 2;
	}
}

void do_tex_clause(uint32_t *inst, int offset, int count, int is_r700)
{
	uint32_t dword0, dword1, dword2;

	while (count) {
		dword0 = inst[offset];
		dword1 = inst[offset + 1];
		dword2 = inst[offset + 2];
		tex_dword0(dword0, is_r700);
		tex_dword1(dword1, is_r700);
		tex_dword2(dword2, is_r700);
		tex_dword_pad();

		count--;
		offset += 4;
	}
}

void do_vtx_clause(uint32_t *inst, int offset, int count, int is_r700)
{
	uint32_t dword0, dword1, dword2;
	int op;

	while (count) {
		dword0 = inst[offset];
		dword1 = inst[offset + 1];
		dword2 = inst[offset + 2];
		vtx_dword0(dword0, is_r700);
		// if op = 1, sem, else gpr
		op = (dword0 >> 0) & 0x1f;
		if (op == 1)
			vtx_dword1_sem(dword1, is_r700);
		else
			vtx_dword1_gpr(dword1, is_r700);
		vtx_dword2(dword2, is_r700);
		vtx_dword_pad();

		count--;
		offset += 4;
	}
}

static int do_cf_inst(uint32_t *inst, uint32_t offset, int is_r700)
{
	uint32_t dword0, dword1;
	int encoding, addr, count, op;
	int eop = 0;

	dword0 = inst[offset];
	dword1 = inst[offset + 1];

	// if high 2 bits of inst are 10b or 11b, ALU,
	// else if 01b, imp/exp
	// else if 00b, cf
	encoding = (dword1 >> 28) & 0x3;
	if (encoding >= 2) {
		cf_alu_dword0(dword0, is_r700);
		cf_alu_dword1(dword1, is_r700);
		addr = (dword0 >> 0) & 0x3fffff;
		addr *= 2;
		count = (dword1 >> 18) & 0x7f;
		count++;
		do_alu_clause(inst, addr, count, is_r700);
	} else if (encoding == 1) {
		op = (dword1 >> 23) & 0x7f;
		if ((op == 39) || (op == 40)) {
			// export inst
			cf_alloc_imp_exp_dword0(dword0, 0, is_r700);
			cf_alloc_imp_exp_dword1_swiz(dword1, is_r700);
		} else {
			// mem inst
			cf_alloc_imp_exp_dword0(dword0, 1, is_r700);
			cf_alloc_imp_exp_dword1_buf(dword1, is_r700);
		}
		eop = (dword1 >> 21) & 0x1;
	} else {
		op = (dword1 >> 23) & 0x7f;
		addr = dword0;
		addr *= 2;
		count = (dword1 >> 10) & 0x7;
		if (is_r700)
			count += (dword1 >> 19) & 0x1;
		count++;
		cf_dword0(dword0, is_r700);
		cf_dword1(dword1, is_r700);
		if (op == 1)
			do_tex_clause(inst, addr, count, is_r700);
		else if ((op == 2) || (op == 3))
			do_vtx_clause(inst, addr, count, is_r700);
		eop = (dword1 >> 21) & 0x1;
	}
	return eop;
}

int main () {
	uint32_t sp[] = {
		0x00000014,
		0x00800400,
		0x00000003,
		0xA03C0000,
		0xC0018000,
		0x94200688,
		0x10200001,
		0x004C2810,
		0x10A00401,
		0x204C2800,
		0x11200801,
		0x404C2800,
		0x91A00C01,
		0x60442800,
		0x10202001,
		0x004C2800,
		0x10A02401,
		0x204C2810,
		0x11202801,
		0x404C2800,
		0x91A02C01,
		0x60442800,
		0x10204001,
		0x004C2800,
		0x10A04401,
		0x204C2800,
		0x11204801,
		0x404C2810,
		0x91A04C01,
		0x60442800,
		0x10000002,
		0x00740C90,
		0x10000402,
		0x20740C90,
		0x10000802,
		0x40740C90,
		0x90000C02,
		0x60600C90,
		0x00000000,
		0x00000000,
		0x00000010,
		0xF00FF001,
		0x68800000,
		0xDEADDEAD,
		0x00000110,
		0xF01D1E01,
		0x68808000,
		0xDEADDEAD,
	};
	int is_r700 = 1; //1
	uint32_t dword0 = sp[0];
	uint32_t dword1 = sp[1];
	uint32_t offset = 0;
	int eop = 0;

	while (offset < (sizeof(sp) / 8)) {
		eop = do_cf_inst(sp, offset, is_r700);

		if (eop)
			break;
		offset += 2;
	}

	return 0;
}
