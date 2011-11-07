#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssnes_dsp.h"
#include <pmmintrin.h>

#define SIDELOBES 4 // 8-tap SINC.
#define PHASES 0x4000

#define ALIGNED __attribute__((aligned(16)))

struct sinc_resampler
{
   float buffer[0x4000] ALIGNED;
   float buffer_l[2 * SIDELOBES] ALIGNED;
   float buffer_r[2 * SIDELOBES] ALIGNED;
   float sinc_table[(2 * PHASES + 1) * (2 * SIDELOBES)] ALIGNED;

   double index;
   double ratio_r;
};

static inline float sinc(float val)
{
   if (fabs(val) < 0.0001)
      return 1.0;
   else
      return sinf(val) / val;
}

static void init_sinc_table(struct sinc_resampler *resamp)
{
   for (unsigned i = 0; i <= 2 * PHASES; i++)
   {
      float index = (float)i / PHASES - 1.5f;
      for (unsigned j = 0; j < 8; j++)
      {
         float phase = M_PI * (SIDELOBES - j + index);
         resamp->sinc_table[i * 8 + j] = sinc(phase / SIDELOBES) * sinc(phase);
      }
   }
}

static unsigned resample_sse3(struct sinc_resampler *resamp, const float * restrict input, unsigned frames)
{
   frames &= ~1;

   float *output = resamp->buffer;
   unsigned frames_out = 0;

   __m128 buffer_l[2] = { _mm_load_ps(resamp->buffer_l + 0), _mm_load_ps(resamp->buffer_l + 4) };
   __m128 buffer_r[2] = { _mm_load_ps(resamp->buffer_r + 0), _mm_load_ps(resamp->buffer_r + 4) };

   for (unsigned i = 0; i < frames; i += 2)
   {
      do
      {
         unsigned base_ptr = round((resamp->index + 1.5) * PHASES);

         __m128 sincs[2] = {
            _mm_load_ps(resamp->sinc_table + base_ptr * 8 + 0),
            _mm_load_ps(resamp->sinc_table + base_ptr * 8 + 4),
         };

         // Do teh math.
         __m128 sum_left  = _mm_add_ps(_mm_mul_ps(sincs[0], buffer_l[0]), _mm_mul_ps(sincs[1], buffer_l[1]));
         __m128 sum_right = _mm_add_ps(_mm_mul_ps(sincs[0], buffer_r[0]), _mm_mul_ps(sincs[1], buffer_r[1]));

         // Pack together so result of L/R ends up as packed.x, packed.y.
         __m128 packed = _mm_hadd_ps(sum_left, sum_right);
         packed = _mm_hadd_ps(packed, packed);

         _mm_storeu_ps(output, packed); // Save the whole vector. It will be overwritten anyways.

         output += 2;
         frames_out++;

         resamp->index += resamp->ratio_r;
      } while (resamp->index < 0.5);

      resamp->index -= 2.0;

      __m128 input_ =_mm_loadu_ps(input);
      input += 4;

      // Shuffle in new bytes in buffer.
      buffer_l[0] = _mm_shuffle_ps(buffer_l[0], buffer_l[1], _MM_SHUFFLE(1, 0, 3, 2));
      buffer_l[1] = _mm_shuffle_ps(buffer_l[1], input_, _MM_SHUFFLE(2, 0, 3, 2));
      buffer_r[0] = _mm_shuffle_ps(buffer_r[0], buffer_r[1], _MM_SHUFFLE(1, 0, 3, 2));
      buffer_r[1] = _mm_shuffle_ps(buffer_r[1], input_, _MM_SHUFFLE(3, 1, 3, 2));
   }

   _mm_store_ps(resamp->buffer_l + 0, buffer_l[0]);
   _mm_store_ps(resamp->buffer_l + 4, buffer_l[1]);
   _mm_store_ps(resamp->buffer_r + 0, buffer_r[0]);
   _mm_store_ps(resamp->buffer_r + 4, buffer_r[1]);

   return frames_out;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   struct sinc_resampler *resamp = data;
   output->samples = resamp->buffer;
   output->frames = resample_sse3(resamp, input->samples, input->frames);
   output->should_resample = SSNES_FALSE;
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

   struct sinc_resampler *resamp = calloc(1, sizeof(*resamp));
   if (!resamp)
      return NULL;

   resamp->ratio_r = (double)info->input_rate / (double)info->output_rate;
   resamp->index = -0.5;

   init_sinc_table(resamp);

   fprintf(stderr, "[SINC resampler (SSE3)] loaded!\n");

   return resamp;
}

static const ssnes_dsp_plugin_t dsp_plug = {
   .init = dsp_init,
   .process = dsp_process,
   .free = dsp_free,
   .api_version = SSNES_DSP_API_VERSION,
   .ident = "Simple Sinc (SSE3)"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE
   ssnes_dsp_plugin_init(void) { return &dsp_plug; }

#ifdef DSP_TEST
#include <stdio.h>
#include "utils.h"
int main(void)
{
   const ssnes_dsp_plugin_t *plug = ssnes_dsp_plugin_init();
   void *handle = plug->init(&(const ssnes_dsp_info_t) { .input_rate = 44100, .output_rate = 96000 });

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

      fwrite(out_buf, sizeof(int16_t), output.frames * 2, stdout);
   }

   plug->free(handle);
}
#endif

