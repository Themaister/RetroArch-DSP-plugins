#include "utils.h"

#if __SSE2__
#include <emmintrin.h>
#elif __ALTIVEC__
#include <altivec.h>
#endif

void audio_convert_float_to_s16_C(int16_t *out,
      const float *in, size_t samples)
{
   for (size_t i = 0; i < samples; i++)
   {
      int32_t val = (int32_t)(in[i] * 0x8000);
      out[i] = (val > 0x7FFF) ? 0x7FFF : (val < -0x8000 ? -0x8000 : (int16_t)val);
   }
}

#if __SSE2__
void audio_convert_float_to_s16_SSE2(int16_t *out,
      const float *in, size_t samples)
{
   __m128 factor = _mm_set1_ps((float)0x7fff);
   size_t i;
   for (i = 0; i + 8 <= samples; i += 8, in += 8, out += 8)
   {
      __m128 input[2] = { _mm_loadu_ps(in + 0), _mm_loadu_ps(in + 4) };
      __m128 res[2] = { _mm_mul_ps(input[0], factor), _mm_mul_ps(input[1], factor) };

      __m128i ints[2] = { _mm_cvtps_epi32(res[0]), _mm_cvtps_epi32(res[1]) };
      __m128i packed = _mm_packs_epi32(ints[0], ints[1]);

      _mm_storeu_si128((__m128i *)out, packed);
   }

   audio_convert_float_to_s16_C(out, in, samples - i);
}
#elif __ALTIVEC__
void audio_convert_float_to_s16_altivec(int16_t *out,
      const float *in, size_t samples)
{
   // Unaligned loads/store is a bit expensive, so we optimize for the good path (very likely).
   if (((uintptr_t)out & 15) + ((uintptr_t)in & 15) == 0)
   {
      size_t i;
      for (i = 0; i + 8 <= samples; i += 8, in += 8, out += 8)
      {
         vector float input0 = vec_ld( 0, in);
         vector float input1 = vec_ld(16, in);
         vector signed int result0 = vec_cts(input0, 15);
         vector signed int result1 = vec_cts(input1, 15);
         vec_st(vec_packs(result0, result1), 0, out);
      }

      audio_convert_float_to_s16_C(out, in, samples - i);
   }
   else
      audio_convert_float_to_s16_C(out, in, samples);
}

#endif

