#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ssnes_dsp.h"
#include <emmintrin.h>
#include <algorithm>
#include "abstract_plugin.hpp"

#define ECHO_MS 150
#define AMP 0.2

struct EchoFilter : public AbstractPlugin
{
   union
   {
      float echo_buffer[0x10000];
      __m128 echo_buffer_vec[0x10000 / 4];
   };

   union
   {
      float buffer[4096];
      __m128 buffer_vec[4096 / 4];
   };

   float scratch_buf[4];

   unsigned buf_size;

   unsigned ptr;
   unsigned scratch_ptr;
   float amp;
   float input_rate;

   EchoFilter()
   {
      ptr = 0;
      scratch_ptr = 0;
      amp = 0.0;
      input_rate = 32000.0;
      std::fill(std::begin(echo_buffer), std::end(echo_buffer), 0.0);
      std::fill(std::begin(buffer), std::end(buffer), 0.0);
      std::fill(std::begin(scratch_buf), std::end(scratch_buf), 0.0);

      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.id = Echo;
      opt.description = "Delay";
      opt.d.min = 1.0;
      opt.d.max = 800.0;
      opt.d.current = ECHO_MS;
      dsp_options.push_back(opt);

      opt.id = Amp;
      opt.description = "Amplification";
      opt.d.min = 0.0;
      opt.d.max = 1.0;
      opt.d.current = AMP;
      dsp_options.push_back(opt);
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case Echo:
            buf_size = val * (input_rate * 2) / 1000;
            buf_size = (buf_size + 3) & ~3;
            if (buf_size > sizeof(echo_buffer) / sizeof(float))
               buf_size = sizeof(echo_buffer) / sizeof(float);
            break;

         case Amp:
            amp = val;
            break;
      }
   }

   enum IDs : PluginOption::ID { Echo, Amp };

   unsigned Process(const float *input, unsigned frames)
   {
      unsigned frames_out = 0;
      float *buffer_out = buffer;

      __m128 amp = _mm_set1_ps(this->amp);

      // Fill up scratch buffer and flush.
      if (scratch_ptr)
      {
         for (unsigned i = scratch_ptr; i < 4; i += 2)
         {
            scratch_buf[i] = *input++;
            scratch_buf[i] = *input++;
            frames--;
         }

         scratch_ptr = 0;

         __m128 reg = _mm_load_ps(scratch_buf);
         __m128 echo_ = _mm_load_ps(echo_buffer + ptr);
         __m128 result = _mm_add_ps(reg, _mm_mul_ps(amp, echo_));
         _mm_store_ps(echo_buffer + ptr, result);
         _mm_store_ps(buffer_out, result);

         buffer_out += 4;
         ptr = (ptr + 4) % buf_size;
         frames_out++;
      }

      // Main processing.
      unsigned i;
      for (i = 0; (i + 4) <= (frames * 2); i += 4, input += 4, buffer_out += 4, frames_out += 2)
      {
         __m128 reg = _mm_loadu_ps(input); // Might not be aligned.
         __m128 echo_ = _mm_load_ps(echo_buffer + ptr);
         __m128 result = _mm_add_ps(reg, _mm_mul_ps(amp, echo_));
         _mm_store_ps(echo_buffer + ptr, result);
         _mm_store_ps(buffer_out, result);

         ptr = (ptr + 4) % buf_size;
      }

      // Flush rest to scratch buffer.
      for (; i < (frames * 2); i++)
         scratch_buf[scratch_ptr++] = *input++;

      return frames_out;
   }
};

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   EchoFilter *echo = reinterpret_cast<EchoFilter*>(data);
   output->samples = echo->buffer;
   output->frames = echo->Process(input->samples, input->frames);
   output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<EchoFilter*>(data);
}

static void *dsp_init(const ssnes_dsp_info_t *info)
{
   EchoFilter *echo = new EchoFilter;

   echo->input_rate = info->input_rate;
   echo->buf_size = ECHO_MS * (info->input_rate * 2) / 1000;
   echo->amp = AMP;

   fprintf(stderr, "[Echo] loaded!\n");

   return echo;
}

static void dsp_config(void *)
{}

static const ssnes_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   SSNES_DSP_API_VERSION,
   dsp_config,
   "Echo plugin (SSE2)"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE
   ssnes_dsp_plugin_init(void) { return &dsp_plug; }

