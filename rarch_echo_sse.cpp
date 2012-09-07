#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "rarch_dsp.h"
#include <algorithm>
#include <iterator>
#include <type_traits>
#include "abstract_plugin.hpp"
#include "utils.hpp"

#ifdef PERF_TEST
#include "timer.hpp"
#endif

#include <emmintrin.h>

// 4 source echo.

#define ALIGNED __attribute__((aligned(16))) // Should use C++11 alignas(), but doesn't seem to work :(

#define ECHO_MS 150
#define AMP 0.0

struct EchoFilter : public AbstractPlugin
{
   float echo_buffer[4][0x10000] ALIGNED;
   float buffer[4096] ALIGNED;
   float scratch_buf[4] ALIGNED;

   unsigned buf_size[4];
   unsigned ptr[4];

   unsigned scratch_ptr;
   __m128 amp[4] ALIGNED;
   __m128 feedback ALIGNED;
   float input_rate;

#ifdef PERF_TEST
   Timer timer;
#endif

   EchoFilter()
   {
      Utils::set_all(ptr, 0.0f);
      Utils::set_all(amp, _mm_set1_ps(AMP));

      scratch_ptr = 0;
      feedback = _mm_set1_ps(0.0f);

      input_rate = 32000.0;
      Utils::set_all(echo_buffer, 0.0f);
      Utils::set_all(buffer, 0.0f);
      Utils::set_all(scratch_buf, 0.0f);

      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.d.min = 1.0;
      opt.d.max = 800.0;
      opt.d.current = ECHO_MS;

      opt.description = "Delay (#1)";
      opt.id = Echo1;
      opt.conf_name = "echo_sse_delay0";
      dsp_options.push_back(opt);
      opt.description = "Delay (#2)";
      opt.id = Echo2;
      opt.conf_name = "echo_sse_delay1";
      dsp_options.push_back(opt);
      opt.description = "Delay (#3)";
      opt.id = Echo3;
      opt.conf_name = "echo_sse_delay2";
      dsp_options.push_back(opt);
      opt.description = "Delay (#4)";
      opt.id = Echo4;
      opt.conf_name = "echo_sse_delay3";
      dsp_options.push_back(opt);

      opt.d.min = 0.0;
      opt.d.max = 0.5;
      opt.d.current = AMP;

      opt.description = "Amplification (#1)";
      opt.id = Amp1;
      opt.conf_name = "echo_sse_amp0";
      dsp_options.push_back(opt);
      opt.description = "Amplification (#2)";
      opt.id = Amp2;
      opt.conf_name = "echo_sse_amp1";
      dsp_options.push_back(opt);
      opt.description = "Amplification (#3)";
      opt.id = Amp3;
      opt.conf_name = "echo_sse_amp2";
      dsp_options.push_back(opt);
      opt.description = "Amplification (#4)";
      opt.id = Amp4;
      opt.conf_name = "echo_sse_amp3";
      dsp_options.push_back(opt);

      opt.description = "Echo feedback";
      opt.id = Feedback;
      opt.d.min = 0.0;
      opt.d.max = 1.0;
      opt.d.current = 0.0;
      opt.conf_name = "echo_sse_feedback";
      dsp_options.push_back(opt);

      load_options("rarch_effect.cfg");
   }

   ~EchoFilter()
   {
      save_options("rarch_effect.cfg");
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      unsigned index;
      switch (id)
      {
         case Echo1:
         case Echo2:
         case Echo3:
         case Echo4:
            index = static_cast<unsigned>(id) - static_cast<unsigned>(Echo1);
            buf_size[index] = val * (input_rate * 2) / 1000;
            buf_size[index] = (buf_size[index] + 3) & ~3;
            if (buf_size[index] > sizeof(echo_buffer[0]) / sizeof(float))
               buf_size[index] = sizeof(echo_buffer[0]) / sizeof(float);
            break;

         case Amp1:
         case Amp2:
         case Amp3:
         case Amp4:
            index = static_cast<unsigned>(id) - static_cast<unsigned>(Amp1);
            amp[index] = _mm_set1_ps(val);
            break;

         case Feedback:
            feedback = _mm_set1_ps(val);
            break;
      }
   }

   enum IDs : PluginOption::ID {
      Echo1, Echo2, Echo3, Echo4,
      Amp1, Amp2, Amp3, Amp4,
      Feedback
   };

   unsigned Process(const float *input, unsigned frames)
   {
      unsigned frames_out = 0;
      float *buffer_out = buffer;

      __m128 amp[4] = {
         this->amp[0],
         this->amp[1],
         this->amp[2],
         this->amp[3],
      };

      __m128 feedback = this->feedback;

#define DO_FILTER() \
      __m128 result[4]; \
      __m128 echo_[4]; \
      for (unsigned i = 0; i < 4; i++) \
      { \
         echo_[i] = _mm_load_ps(echo_buffer[i] + ptr[i]); \
         result[i] = _mm_mul_ps(amp[i], echo_[i]); \
      } \
      __m128 final_result = _mm_add_ps(_mm_add_ps(result[0], result[1]), _mm_add_ps(result[2], result[3])); \
      __m128 feedback_result = _mm_mul_ps(feedback, final_result); \
      final_result = _mm_add_ps(reg, final_result); \
      feedback_result = _mm_add_ps(reg, feedback_result); \
      for (unsigned i = 0; i < 4; i++) \
         _mm_store_ps(echo_buffer[i] + ptr[i], feedback_result); \
      _mm_store_ps(buffer_out, final_result); \
      for (unsigned i = 0; i < 4; i++) \
         ptr[i] = (ptr[i] + 4) % buf_size[i]


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

         DO_FILTER();

         frames_out += 2;
         buffer_out += 4;
      }

      // Main processing.
      unsigned i;
      for (i = 0; (i + 4) <= (frames * 2); i += 4, input += 4, buffer_out += 4, frames_out += 2)
      {
         __m128 reg = _mm_loadu_ps(input); // Might not be aligned.
         DO_FILTER();
      }

      // Flush rest to scratch buffer.
      for (; i < (frames * 2); i++)
         scratch_buf[scratch_ptr++] = *input++;

      return frames_out;
   }
};

static void dsp_process(void *data, rarch_dsp_output_t *output,
      const rarch_dsp_input_t *input)
{
   EchoFilter *echo = reinterpret_cast<EchoFilter*>(data);
   output->samples = echo->buffer;

#ifdef PERF_TEST
   echo->timer.start();
#endif
   output->frames = echo->Process(input->samples, input->frames);
#ifdef PERF_TEST
   echo->timer.stop(input->frames);
#endif

   output->should_resample = RARCH_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<EchoFilter*>(data);
}

static void *dsp_init(const rarch_dsp_info_t *info)
{
   EchoFilter *echo = new EchoFilter;

   echo->input_rate = info->input_rate;

   for (unsigned i = 0; i < 4; i++)
      echo->buf_size[i] = ECHO_MS * (info->input_rate * 2) / 1000;

   fprintf(stderr, "[Echo] loaded!\n");

   return echo;
}

static void dsp_config(void *)
{}

static const rarch_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   RARCH_DSP_API_VERSION,
   dsp_config,
   "Echo plugin (SSE2)"
};

RARCH_API_EXPORT const rarch_dsp_plugin_t* RARCH_API_CALLTYPE
   rarch_dsp_plugin_init(void) { return &dsp_plug; }

