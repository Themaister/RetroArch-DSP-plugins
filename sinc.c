#include "ssnes_dsp.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>

#if __SSE__
#include <xmmintrin.h>
#endif

#if __SSE3__
#include <pmmintrin.h>
#endif

#define PHASE_BITS 8
#define SUBPHASE_BITS 16

#define PHASES (1 << PHASE_BITS)
#define PHASES_SHIFT (SUBPHASE_BITS)
#define SUBPHASES (1 << SUBPHASE_BITS)
#define SUBPHASES_SHIFT 0
#define SUBPHASES_MASK ((1 << SUBPHASE_BITS) - 1)
#define PHASES_WRAP (1 << (PHASE_BITS + SUBPHASE_BITS))

#define SIDELOBES 8

struct sinc_resampler
{
   float buffer[0x4000];
   float phase_table[PHASES + 1][SIDELOBES];
   float delta_table[PHASES + 1][SIDELOBES];
   float buffer_l[2 * SIDELOBES];
   float buffer_r[2 * SIDELOBES];

   uint32_t ratio;
   uint32_t time;
};

static inline double sinc(double val)
{
   if (fabs(val) < 0.00001)
      return 1.0;
   else
      return sin(val) / val;
}

static inline double blackman(double index)
{
   index *= 0.5;
   index += 0.5;

   double alpha = 0.16;
   double a0 = (1.0 - alpha) / 2.0;
   double a1 = 0.5;
   double a2 = alpha / 2.0;

   return a0 - a1 * cos(2.0 * M_PI * index) + a2 * cos(4.0 * M_PI * index);
}

static void init_sinc_table(struct sinc_resampler *resamp)
{
   for (unsigned i = 0; i <= PHASES; i++)
   {
      for (unsigned j = 0; j < SIDELOBES; j++)
      {
         double sinc_phase = M_PI * ((double)i / PHASES + (double)j);
         resamp->phase_table[i][j] = sinc(sinc_phase) * blackman(sinc_phase / SIDELOBES);
      }
   }

   // Optimize linear interpolation.
   for (unsigned i = 0; i < PHASES; i++)
      for (unsigned j = 0; j < SIDELOBES; j++)
         resamp->delta_table[i][j] = resamp->phase_table[i + 1][j] - resamp->phase_table[i][j];
}

#if __SSE__
static void process_sinc(struct sinc_resampler *resamp, float * restrict out_buffer)
{
   __m128 sum_l = _mm_setzero_ps();
   __m128 sum_r = _mm_setzero_ps();

   const float *buffer_l = resamp->buffer_l;
   const float *buffer_r = resamp->buffer_r;

   unsigned phase = resamp->time >> PHASES_SHIFT;
   unsigned delta = (resamp->time >> SUBPHASES_SHIFT) & SUBPHASES_MASK;
   __m128 delta_f = _mm_set1_ps((float)delta / SUBPHASES);

   const float *phase_table = resamp->phase_table[phase];
   const float *delta_table = resamp->delta_table[phase];

   __m128 sinc_vals[SIDELOBES / 4];
   for (unsigned i = 0; i < SIDELOBES; i += 4)
   {
      __m128 phases    = _mm_load_ps(phase_table + i);
      __m128 deltas    = _mm_load_ps(delta_table + i);
      sinc_vals[i / 4] = _mm_add_ps(phases, _mm_mul_ps(deltas, delta_f));
   }

   // Older data.
   for (unsigned i = 0; i < SIDELOBES; i += 4)
   {
      __m128 buf_l = _mm_loadr_ps(buffer_l + SIDELOBES - 4 - i);
      sum_l        = _mm_add_ps(sum_l, _mm_mul_ps(buf_l, sinc_vals[i / 4]));

      __m128 buf_r = _mm_loadr_ps(buffer_r + SIDELOBES - 4 - i);
      sum_r        = _mm_add_ps(sum_r, _mm_mul_ps(buf_r, sinc_vals[i / 4]));
   }

   // Newer data.
   unsigned reverse_phase = PHASES_WRAP - resamp->time;
   phase   = reverse_phase >> PHASES_SHIFT;
   delta   = (reverse_phase >> SUBPHASES_SHIFT) & SUBPHASES_MASK;
   delta_f = _mm_set1_ps((float)delta / SUBPHASES);

   phase_table = resamp->phase_table[phase];
   delta_table = resamp->delta_table[phase];

   for (unsigned i = 0; i < SIDELOBES; i += 4)
   {
      __m128 phases    = _mm_load_ps(phase_table + i);
      __m128 deltas    = _mm_load_ps(delta_table + i);
      sinc_vals[i / 4] = _mm_add_ps(phases, _mm_mul_ps(deltas, delta_f));
   }

   for (unsigned i = 0; i < SIDELOBES; i += 4)
   {
      __m128 buf_l = _mm_load_ps(buffer_l + SIDELOBES + i);
      sum_l        = _mm_add_ps(sum_l, _mm_mul_ps(buf_l, sinc_vals[i / 4]));

      __m128 buf_r = _mm_load_ps(buffer_r + SIDELOBES + i);
      sum_r        = _mm_add_ps(sum_r, _mm_mul_ps(buf_r, sinc_vals[i / 4]));
   }

#if __SSE3__ // hadd! :D
   __m128 sum = _mm_hadd_ps(sum_l, sum_r);
   sum = _mm_hadd_ps(sum, sum);

   _mm_storeu_ps(out_buffer, sum);
#else // Slower shuffles :(
   __m128 sum_shuf_l = _mm_shuffle_ps(sum_l, sum_r, _MM_SHUFFLE(1, 0, 1, 0));
   __m128 sum_shuf_r = _mm_shuffle_ps(sum_l, sum_r, _MM_SHUFFLE(3, 2, 3, 2));
   __m128 sum        = _mm_add_ps(sum_shuf_l, sum_shuf_r);

   union
   {
      float f[4];
      __m128 v;
   } u;
   u.v = sum;

   out_buffer[0] = u.f[0] + u.f[1];
   out_buffer[1] = u.f[2] + u.f[3];
#endif
}
#else // Plain ol' C99
static void process_sinc(struct sinc_resampler *resamp, float * restrict out_buffer)
{
   float sum_l = 0.0f;
   float sum_r = 0.0f;
   const float *buffer_l = resamp->buffer_l;
   const float *buffer_r = resamp->buffer_r;

   unsigned phase = resamp->time >> PHASES_SHIFT;
   unsigned delta = (resamp->time >> SUBPHASES_SHIFT) & SUBPHASES_MASK;
   float delta_f = (float)delta / SUBPHASES;

   const float *phase_table = resamp->phase_table[phase];
   const float *delta_table = resamp->delta_table[phase];

   float sinc_vals[SIDELOBES];
   for (unsigned i = 0; i < SIDELOBES; i++)
      sinc_vals[i] = phase_table[i] + delta_f * delta_table[i];

   // Older data.
   for (unsigned i = 0; i < SIDELOBES; i++)
   {
      sum_l += buffer_l[SIDELOBES - 1 - i] * sinc_vals[i];
      sum_r += buffer_r[SIDELOBES - 1 - i] * sinc_vals[i];
   }

   // Newer data.
   unsigned reverse_phase = PHASES_WRAP - resamp->time;
   phase   = reverse_phase >> PHASES_SHIFT;
   delta   = (reverse_phase >> SUBPHASES_SHIFT) & SUBPHASES_MASK;
   delta_f = (float)delta / SUBPHASES;

   phase_table = resamp->phase_table[phase];
   delta_table = resamp->delta_table[phase];

   for (unsigned i = 0; i < SIDELOBES; i++)
      sinc_vals[i] = phase_table[i] + delta_f * delta_table[i];

   for (unsigned i = 0; i < SIDELOBES; i++)
   {
      sum_l += buffer_l[SIDELOBES + i] * sinc_vals[i];
      sum_r += buffer_r[SIDELOBES + i] * sinc_vals[i];
   }

   out_buffer[0] = sum_l;
   out_buffer[1] = sum_r;
}
#endif

static unsigned resample(struct sinc_resampler *resamp, const float * restrict input, unsigned frames)
{
   float *out_ptr = resamp->buffer;
   unsigned frames_out = 0;

   while (frames)
   {
      process_sinc(resamp, out_ptr);
      out_ptr += 2;
      frames_out++;

      // Shuffle in new data.
      resamp->time += resamp->ratio;
      if (resamp->time >= PHASES_WRAP)
      {
         memmove(resamp->buffer_l, resamp->buffer_l + 1,
               sizeof(resamp->buffer_l) - sizeof(float));
         memmove(resamp->buffer_r, resamp->buffer_r + 1,
               sizeof(resamp->buffer_r) - sizeof(float));

         resamp->buffer_l[2 * SIDELOBES - 1] = *input++;
         resamp->buffer_r[2 * SIDELOBES - 1] = *input++;
         frames--;

         resamp->time -= PHASES_WRAP;
      }
   }

   return frames_out;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   struct sinc_resampler *resamp = data;
   output->samples               = resamp->buffer;
   output->frames                = resample(resamp, input->samples, input->frames);
   output->should_resample       = SSNES_FALSE;
}

static void dsp_free(void *data)
{
   free(data);
}

static void *dsp_init(const ssnes_dsp_info_t *info)
{
   // Can only upsample.
   if (info->output_rate < info->input_rate)
      return NULL;

   struct sinc_resampler *resamp = memalign(16, sizeof(*resamp));
   if (!resamp)
      return NULL;

   memset(resamp, 0, sizeof(*resamp));

   resamp->ratio = ((uint64_t)PHASES_WRAP * info->input_rate) / info->output_rate;
   resamp->time  = 0;

   init_sinc_table(resamp);

   fprintf(stderr, "[SINC resampler] loaded!\n");

   return resamp;
}

static const ssnes_dsp_plugin_t dsp_plug = {
   .init        = dsp_init,
   .process     = dsp_process,
   .free        = dsp_free,
   .api_version = SSNES_DSP_API_VERSION,
#if __SSE3__
   .ident       = "Simple Sinc (SSE3)",
#elif __SSE__
   .ident       = "Simple Sinc (SSE)",
#else
   .ident       = "Simple Sinc",
#endif
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE
   ssnes_dsp_plugin_init(void) { return &dsp_plug; }

#ifdef DSP_TEST
#include <stdio.h>
#include "utils.h"
#define ALIGNED __attribute__((aligned(16)))
int main(void)
{
   const ssnes_dsp_plugin_t *plug = ssnes_dsp_plugin_init();
   void *handle = plug->init(&(const ssnes_dsp_info_t) { .input_rate = 44100, .output_rate = 48000 });

   int16_t int_buf[2048] ALIGNED;
   float float_buf[2048] ALIGNED;
   int16_t out_buf[2048 * 4] ALIGNED;

   for (;;)
   {
      if (fread(int_buf, 1, sizeof(int_buf), stdin) < sizeof(int_buf))
         break;

      audio_convert_s16_to_float(float_buf, int_buf, 2048);
      ssnes_dsp_output_t output;
      ssnes_dsp_input_t input = {
         .samples = float_buf,
         .frames = 1024,
      };

      plug->process(handle, &output, &input);

      audio_convert_float_to_s16(out_buf, output.samples, output.frames * 2);

      if (fwrite(out_buf, sizeof(int16_t), output.frames * 2, stdout) < output.frames * 2)
         break;
   }

   plug->free(handle);
}
#endif

