#ifndef YUYV_TO_RGB_H
#define YUYV_TO_RGB_H

#ifdef __ARM_NEON__
/*
 * Based on libyuv/source/row_neon.cc [https://code.google.com/p/libyuv]
 */

#define READYUYV                                                                                   \
	"vld2.u8    {d0, d2}, [%0]!                \n"                                             \
	"vmov.u8    d3, d2                         \n"                                             \
	"vuzp.u8    d2, d3                         \n"                                             \
	"vtrn.u32   d2, d3                         \n"

#define YUV422TORGB                                                                                \
	"veor.u8    d2, d26                        \n" /*subtract 128 from u and v*/               \
	"vmull.s8   q8, d2, d24                    \n" /*  u/v B/R component      */               \
	"vmull.s8   q9, d2, d25                    \n" /*  u/v G component        */               \
	"vmov.u8    d1, #0                         \n" /*  split odd/even y apart */               \
	"vtrn.u8    d0, d1                         \n"                                             \
	"vsub.s16   q0, q0, q15                    \n" /*  offset y               */               \
	"vmul.s16   q0, q0, q14                    \n"                                             \
	"vadd.s16   d18, d19                       \n"                                             \
	"vqadd.s16  d22, d0, d16                   \n" /* B */                                     \
	"vqadd.s16  d23, d1, d16                   \n"                                             \
	"vqadd.s16  d20, d0, d17                   \n" /* R */                                     \
	"vqadd.s16  d21, d1, d17                   \n"                                             \
	"vqadd.s16  d16, d0, d18                   \n" /* G */                                     \
	"vqadd.s16  d17, d1, d18                   \n"                                             \
	"vqshrun.s16 d0, q11, #6                   \n" /* B */                                     \
	"vqshrun.s16 d1, q10, #6                   \n" /* R */                                     \
	"vqshrun.s16 d2, q8, #6                    \n" /* G */                                     \
	"vmovl.u8   q11, d0                        \n" /*  set up for reinterleave*/               \
	"vmovl.u8   q10, d1                        \n"                                             \
	"vmovl.u8   q8, d2                         \n"                                             \
	"vtrn.u8    d22, d23                       \n"                                             \
	"vtrn.u8    d20, d21                       \n"                                             \
	"vtrn.u8    d16, d17                       \n"                                             \
	"vmov.u8    d21, d16                       \n"

static const int8_t __attribute__((vector_size(16))) kUVToRB = {
	127, 127, 127, 127, 102, 102, 102, 102, 0, 0, 0, 0, 0, 0, 0, 0};
static const int8_t __attribute__((vector_size(16))) kUVToG = {
	-25, -25, -25, -25, -52, -52, -52, -52, 0, 0, 0, 0, 0, 0, 0, 0};

static inline void yuyv_to_rgb(const uint8_t *src, uint8_t *dest, int length)
{
	asm volatile("vld1.u8    {d24}, [%3]                    \n"
		     "vld1.u8    {d25}, [%4]                    \n"
		     "vmov.u8    d26, #128                      \n"
		     "vmov.u16   q14, #74                       \n"
		     "vmov.u16   q15, #16                       \n"
		     ".p2align  2                               \n"
		     "1:                                          \n" READYUYV YUV422TORGB
		     "subs       %2, %2, #8                     \n"
		     "vst3.8     {d20, d21, d22}, [%1]!         \n"
		     "bgt        1b                             \n"
		     : "+r"(src),     /* %0 */
		       "+r"(dest),    /* %1 */
		       "+r"(length)   /* %2 */
		     : "r"(&kUVToRB), /* %3 */
		       "r"(&kUVToG)   /* %4 */
		     : "cc", "memory", "q0", "q1", "q2", "q3", "q8", "q9", "q10", "q11", "q12",
		       "q13", "q14", "q15");
}

#else /* __ARM_NEON__ */
/*
 * Based on libv4lconvert/rgbyuv.c [http://freecode.com/projects/libv4l]
 */

#define CLIP(color) (unsigned char)(((color) > 0xFF) ? 0xff : (((color) < 0) ? 0 : (color)))

static inline void yuyv_to_rgb(const uint8_t *src, uint8_t *dest, int length)
{
	int j;

	for (j = 0; j < length; j += 2) {
		int u = src[1];
		int v = src[3];
		int u1 = (((u - 128) << 7) + (u - 128)) >> 6;
		int rg = (((u - 128) << 1) + (u - 128) + ((v - 128) << 2) + ((v - 128) << 1)) >> 3;
		int v1 = (((v - 128) << 1) + (v - 128)) >> 1;

		*dest++ = CLIP(src[0] + v1);
		*dest++ = CLIP(src[0] - rg);
		*dest++ = CLIP(src[0] + u1);

		*dest++ = CLIP(src[2] + v1);
		*dest++ = CLIP(src[2] - rg);
		*dest++ = CLIP(src[2] + u1);
		src += 4;
	}
}

#endif /* __ARM_NEON__ */

#endif /* YUYV_TO_RGB_H */
