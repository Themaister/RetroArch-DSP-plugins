#include "eq.h"

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __ALTIVEC__
#include <altivec.h>
typedef vector float vec4;
#endif

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdint.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define TAPS 32

#if defined(__ALTIVEC__)
#define FILT_OVERRUN 8
typedef union
{
   vec4 vec[TAPS / 4 + 2]; // Wraparound for unaligned reads
   float f[TAPS + 8];
} vec_filt;
#elif defined(__SSE3__)
#define FILT_OVERRUN 4
typedef union
{
   __m128 vec[TAPS / 4 + 1]; // Wraparound for unaligned reads
   float f[TAPS + 4];
} vec_filt;
#else
typedef float vec_filt[TAPS];
#define FILT_OVERRUN 0
#endif

static void generate_band_pass(float *fir_out, unsigned num_fir, float freq, float input_rate)
{
   float w_norm = 2.0 * M_PI * freq / input_rate;
   for (unsigned i = 0; i < num_fir; i++)
      fir_out[i] = cosf(i * w_norm);

   complex float amp = 0.0;
   for (unsigned i = 0; i < num_fir; i++)
      amp += cosf(i * w_norm) * cexp(-I * w_norm * i); 

   float abs = cabs(amp);
   for (unsigned i = 0; i < num_fir; i++)
      fir_out[i] /= abs;

   for (unsigned i = 0; i < FILT_OVERRUN; i++)
      fir_out[num_fir + i] = fir_out[i];
}

struct dsp_eq_state
{
   vec_filt buffer;

   vec_filt *bpf;
   vec_filt *bpf_coeff;
   unsigned num_filt;

   unsigned buf_ptr;
};

dsp_eq_state_t *dsp_eq_new(float input_rate, const float *bands, unsigned num_bands)
{
   dsp_eq_state_t *eq = calloc(1, sizeof(*eq));
   if (!eq)
      goto error;

   eq->num_filt = num_bands;
   eq->bpf_coeff = calloc(num_bands, sizeof(vec_filt));
   if (!eq->bpf_coeff)
      goto error;
   eq->bpf = calloc(num_bands, sizeof(vec_filt));
   if (!eq->bpf)
      goto error;

   for (unsigned i = 0; i < num_bands; i++)
   {
#if defined(__SSE__) || defined(__ALTIVEC__)
      generate_band_pass(eq->bpf_coeff[i].f, TAPS, bands[i], input_rate);
#else
      generate_band_pass(eq->bpf_coeff[i], TAPS, bands[i], input_rate);
#endif
      dsp_eq_set_gain(eq, i, 1.0);
   }

#if defined(__ALTIVEC__)
   fprintf(stderr, "[Equalizer]: AltiVec/VMX optimized!\n");
#elif defined(__SSE__)
   fprintf(stderr, "[Equalizer]: SSE optimized!\n");
#else
   fprintf(stderr, "[Equalizer]: Plain C99, how lame!\n");
#endif

   return eq;

error:
   dsp_eq_free(eq);
   return NULL;
}

#if defined(__ALTIVEC__)
#define TAPS_MASK (TAPS * 4 - 1)
static float calculate_fir(const float * restrict samples, const float * restrict coeffs,
      unsigned buf_ptr)
{
   unsigned fir_ptr = 4 * ((TAPS - buf_ptr) & (TAPS - 1));
   vec4 sum0 = vec_splats(0.0f);
   vec4 sum1 = vec_splats(0.0f);
   vec4 sum2 = vec_splats(0.0f);
   vec4 sum3 = vec_splats(0.0f);
   vector unsigned char mask = vec_lvsl(fir_ptr, coeffs);

   for (unsigned i = 0; i < TAPS * 4; i += 64, fir_ptr += 64)
   {
      vec4 samp0 = vec_ld(i +  0, samples);
      vec4 samp1 = vec_ld(i + 16, samples);
      vec4 samp2 = vec_ld(i + 32, samples);
      vec4 samp3 = vec_ld(i + 48, samples);

      vec4 v0 = vec_ld((fir_ptr +  0) & TAPS_MASK, coeffs);
      vec4 v1 = vec_ld((fir_ptr + 16) & TAPS_MASK, coeffs);
      vec4 v2 = vec_ld((fir_ptr + 32) & TAPS_MASK, coeffs);
      vec4 v3 = vec_ld((fir_ptr + 48) & TAPS_MASK, coeffs);
      vec4 v4 = vec_ld((fir_ptr + 64) & TAPS_MASK, coeffs);
      vec4 cof0 = vec_perm(v0, v1, mask);
      vec4 cof1 = vec_perm(v1, v2, mask);
      vec4 cof2 = vec_perm(v2, v3, mask);
      vec4 cof3 = vec_perm(v3, v4, mask);

      sum0 = vec_madd(samp0, cof0, sum0);
      sum1 = vec_madd(samp1, cof1, sum1);
      sum2 = vec_madd(samp2, cof2, sum2);
      sum3 = vec_madd(samp3, cof3, sum3);
   }

   vec4 sum = sum0 + sum1 + sum2 + sum3;

   union
   {
      vec4 vec;
      float f[4];
   } u;
   u.vec = sum;

   float final = 0.0f;
   for (unsigned i = 0; i < 4; i++)
      final += u.f[i];

   return final;
}
#elif defined(__SSE3__)
static float calculate_fir(const float * restrict samples, const float * restrict coeffs,
      unsigned buf_ptr)
{
   unsigned fir_ptr = (TAPS - buf_ptr) & (TAPS - 1);

   __m128 sum[4];
   for (unsigned i = 0; i < 4; i++)
      sum[i] = _mm_setzero_ps();

   for (unsigned i = 0; i < TAPS; i += 16, fir_ptr += 16)
   {
      __m128 samp[4];
      __m128 cof[4];
      __m128 res[4];

      for (unsigned j = 0; j < 4; j++)
         samp[j] = _mm_load_ps(samples + i + 4 * j);

      for (unsigned j = 0; j < 4; j++) // Needs unaligned loads.
         cof[j] = _mm_loadu_ps(coeffs + ((fir_ptr + 4 * j) & (TAPS - 1)));

      for (unsigned j = 0; j < 4; j++)
         res[j] = _mm_mul_ps(samp[j], cof[j]);

      for (unsigned j = 0; j < 4; j++)
         sum[j] = _mm_add_ps(sum[j], res[j]);
   }

   __m128 tmp = _mm_add_ps(_mm_add_ps(sum[0], sum[1]), _mm_add_ps(sum[2], sum[3]));

   union
   {
      __m128 vec;
      float f[4];
   } u;
   u.vec = tmp;

   float final = 0.0f;
   for (unsigned i = 0; i < 4; i++)
      final += u.f[i];

   return final;
}
#else
static float calculate_fir(const float * restrict buffer, const float * restrict coeff,
      unsigned buf_ptr)
{
   unsigned fir_ptr = (TAPS - buf_ptr) & (TAPS - 1);
   float sum = 0.0;

   for (unsigned i = 0; i < TAPS; i++)
      sum += buffer[i] * coeff[(fir_ptr + i) & (TAPS - 1)];

   return sum;
}
#endif

void dsp_eq_set_gain(dsp_eq_state_t *eq, unsigned band, float gain)
{
   for (unsigned i = 0; i < TAPS + FILT_OVERRUN; i++)
   {
#if defined(__SSE3__) || defined(__ALTIVEC__)
      eq->bpf[band].f[i] = gain * eq->bpf_coeff[band].f[i];
#else
      eq->bpf[band][i] = gain * eq->bpf_coeff[band][i];
#endif
   }
}

float dsp_eq_process(dsp_eq_state_t *eq, float sample)
{
#if defined(__SSE3__) || defined(__ALTIVEC__)
   eq->buffer.f[eq->buf_ptr] = sample;
#else
   eq->buffer[eq->buf_ptr] = sample;
#endif

   float sum = 0.0;
   for (unsigned i = 0; i < eq->num_filt; i++)
   {
#if defined(__SSE3__) || defined(__ALTIVEC__)
      sum += calculate_fir(eq->buffer.f, eq->bpf[i].f, eq->buf_ptr);
#else
      sum += calculate_fir(eq->buffer, eq->bpf[i], eq->buf_ptr);
#endif
   }

   eq->buf_ptr = (eq->buf_ptr - 1) & (TAPS - 1);

   return sum;
}

void dsp_eq_free(dsp_eq_state_t *eq)
{
   if (eq)
   {
      if (eq->bpf_coeff)
         free(eq->bpf_coeff);
      if (eq->bpf)
         free(eq->bpf);
      free(eq);
   }
}

