/*
 * RadeonHD R6xx, R7xx DRI driver
 *
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


#ifndef _R600_EMIT_H
#define _R600_EMIT_H

/* This contains various usability inline functions + macros.
 * All based on E32(), CMD_BUFFER_PREAMBLE(), CMD_BUFFER_DEBUG_AVAIL(),
 * which have to be defined externaly: */

#include "r600_hwapi.h"


/* R6xx knows 2 ways to set registers: Packet0, and a set of Packet3 commands,
 * that set certain parts of the register file.
 * Set this to 1 if you want to use the various Packet3 commands. */
//#define USE_SET_CPCMDS 1


#define CP_PACKET0(reg, n)	(RADEON_CP_PACKET0 | (((n)-1)<<16) | ((reg)>>2))
#define CP_PACKET3(cmd, n)	(RADEON_CP_PACKET3 | (((n)-1)<<16) | ((cmd)<<8))


/*
 * Emit macros
 */

/* Emit uint32_t */
/* #define E32(x) already defined outside this headerfile */


/* Emit float value */
#define EFLOAT(x)				\
    do {					\
	union { float f; uint32_t d; } _u;	\
	_u.f=(x);				\
	E32(_u.d);				\
    } while (0)


/* Emit list-of-register writes */
#ifdef USE_SET_CPCMDS
#  define PACK0(reg, num)	pack0 (reg, num)
#else
#  define PACK0(reg, num)			\
    do {					\
	CMD_BUFFER_DEBUG_AVAIL (num+1);		\
	DEBUG (if (num > 0x4000) {		\
		 fprintf (stderr, "Cannot emit packet0 0x%02x with %d dwords\n", reg, num); \
		 exit (1); }			\
	      );				\
	E32 (CP_PACKET0 (reg, num));		\
    } while (0)
#endif


/* Emit Packet3 commands */
#define PACK3(cmd, num)				\
    do {					\
	CMD_BUFFER_DEBUG_AVAIL (num+1);		\
	DEBUG (if (num > 0x4000) {		\
		 fprintf (stderr, "Cannot emit packet3 0x%02x with %d dwords\n", cmd, num); \
		 exit (1); }			\
	      );				\
	E32 (CP_PACKET3 (cmd, num));		\
    } while (0);


/* write a single register, uint32_t */
#define EREG(reg, val)				\
    do {					\
	CMD_BUFFER_DEBUG_AVAIL (2);		\
	PACK0 (reg, 1);				\
	E32   (val);				\
    } while (0)

/* write a single register, float */
#define EREGFLOAT(reg, val)			\
    do {					\
	CMD_BUFFER_DEBUG_AVAIL (2);		\
	PACK0  (reg, 1);			\
	EFLOAT (val);				\
    } while (0)


/*
 * Support macros for emitting standard command sequences
 */

#define EMIT_WAIT_3D_IDLE()							\
    EREG (WAIT_UNTIL, WAIT_3D_IDLE_bit);
#define ELEN_WAIT_3D_IDLE							2

#define EMIT_WAIT_3D_IDLE_CLEAN()						\
    PACK3 (IT_EVENT_WRITE, 1);		/* flush caches, no timestamp */       	\
    E32   (CACHE_FLUSH_AND_INV_EVENT);						\
    EREG  (WAIT_UNTIL, WAIT_3D_IDLECLEAN_bit);
#define ELEN_WAIT_3D_IDLE_CLEAN							4


/*
 * Function versions (old style, to be nuked)
 */

void static inline e32 (uint32_t dword)
{
    CMD_BUFFER_NOCHECK_PREAMBLE;
    E32 (dword);
}

void static inline efloat (float f)
{
    union {
	float f;
	uint32_t d;
    } _u;
    CMD_BUFFER_NOCHECK_PREAMBLE;
    _u.f = f;
    E32 (_u.d);
}

static void inline pack3 (int cmd, unsigned num)
{
    CMD_BUFFER_PREAMBLE (num+1);
    if (num > 0x4000) {
	fprintf (stderr, "Cannot emit packet3 0x%02x with %d dwords\n", cmd, num);
	exit (1);
    }
    E32 (CP_PACKET3 (cmd, num));
}

/* write num registers, start at reg */
/* If register falls in a special area, special commands are issued */
void static inline pack0 (uint32_t reg, int num)
{
#ifdef USE_SET_CPCMDS
    CMD_BUFFER_PREAMBLE (num+2);
    DEBUG (if(num > 0x3fff) {
	    fprintf (stderr, "Cannot emit packet0 emulation 0x%04x with %d dwords\n", reg, num);
	    exit (1); }	);
    if (reg >= SET_CONFIG_REG_offset && reg < SET_CONFIG_REG_end) {
	PACK3 (IT_SET_CONFIG_REG, num+1);
	E32   ((reg-SET_CONFIG_REG_offset) >> 2);
    } else if (reg >= SET_CONTEXT_REG_offset && reg < SET_CONTEXT_REG_end) {
	PACK3 (IT_SET_CONTEXT_REG, num+1);
	E32   ((reg-0x28000) >> 2);
    } else if (reg >= SET_ALU_CONST_offset && reg < SET_ALU_CONST_end) {
	PACK3 (IT_SET_ALU_CONST, num+1);
	E32   ((reg-SET_ALU_CONST_offset) >> 2);
    } else if (reg >= SET_RESOURCE_offset && reg < SET_RESOURCE_end) {
	PACK3 (IT_SET_RESOURCE, num+1);
	E32   ((reg-SET_RESOURCE_offset) >> 2);
    } else if (reg >= SET_SAMPLER_offset && reg < SET_SAMPLER_end) {
	PACK3 (IT_SET_SAMPLER, num+1);
	E32   ((reg-SET_SAMPLER_offset) >> 2);
    } else if (reg >= SET_CTL_CONST_offset && reg < SET_CTL_CONST_end) {
	PACK3 (IT_SET_CTL_CONST, num+1);
	E32   ((reg-SET_CTL_CONST_offset) >> 2);
    } else if (reg >= SET_LOOP_CONST_offset && reg < SET_LOOP_CONST_end) {
	PACK3 (IT_SET_LOOP_CONST, num+1);
	E32   ((reg-SET_LOOP_CONST_offset) >> 2);
    } else if (reg >= SET_BOOL_CONST_offset && reg < SET_BOOL_CONST_end) {
	PACK3 (IT_SET_BOOL_CONST, num+1);
	E32   ((reg-SET_BOOL_CONST_offset) >> 2);
    } else {
	E32 (CP_PACKET0 (reg, num));
    }
#else
    CMD_BUFFER_PREAMBLE (num+1);
    DEBUG (if(num > 0x4000) {
	    fprintf (stderr, "Cannot emit packet0 0x%04x with %d dwords\n", reg, num);
	    exit (1); }	);
    E32 (CP_PACKET0 (reg, num));
#endif
}

void static inline ereg (uint32_t reg, uint32_t val)
{
    pack0 (reg, 1);
    e32   (val);
}

static inline void flush_cache (void *start, void *end)
{
    volatile uint32_t *p;
    /* Read back each cache line from main memory to ensure write combined data is flushed */
    for (p = start; (void *) p < end; p += 16) /* a cache line is at least 64 bytes for all modern cpus */
	(void) *p;
}

static inline void flush_cache_indirect (void)
{
    flush_cache (indirect, (uint32_t *) indirect + indirect_end);
}

#endif	/* _R600_EMIT_H */
