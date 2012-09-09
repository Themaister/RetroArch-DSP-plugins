#include "rarch_dsp.h"
#include <iostream>
#include "abstract_plugin.hpp"
#include <math.h>
#include "eq.h"
#include <stdlib.h>

#ifdef PERF_TEST
#include <time.h>
#endif

struct PlugEQ : public AbstractPlugin
{
   PlugEQ(float rate) : AbstractPlugin()
   {
#ifdef PERF_TEST
      process_frames = 0;
      process_time = 0.0;
#endif

      plug_layout = AbstractPlugin::Layout::Horizontal;

      const float bands[] = { 100, 400, 800, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 9000, 10000, 12000 };
      eq_l = dsp_eq_new(rate, bands, sizeof(bands) / sizeof(bands[0]));
      eq_r = dsp_eq_new(rate, bands, sizeof(bands) / sizeof(bands[0]));

      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      const char *desc[] = {
         "100 Hz",
         "400 Hz",
         "800 Hz",
         "1 kHz",
         "1.5 kHz",
         "2 kHz",
         "2.5 kHz",
         "3 kHz",
         "3.5 kHz",
         "4 kHz",
         "4.5 kHz",
         "5 kHz",
         "5.5 kHz",
         "6 kHz",
         "6.5 kHz",
         "7 kHz",
         "7.5 kHz",
         "8 kHz",
         "9 kHz",
         "10 kHz",
         "12 kHz",
      };

      for (unsigned i = 0; i < sizeof(desc) / sizeof(desc[0]); i++)
      {
         opt.id = i;
         opt.description = desc[i];
         opt.d.min = -50.0;
         opt.d.max = 20.0;
         opt.d.current = 0.0;
         dsp_options.push_back(opt);
      }
   }

   ~PlugEQ()
   {
      dsp_eq_free(eq_l);
      dsp_eq_free(eq_r);
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      std::cerr << "Setting ID: " << static_cast<unsigned>(id) << std::endl;
      dsp_eq_set_gain(eq_l, id, db2gain(val));
      dsp_eq_set_gain(eq_r, id, db2gain(val));
   }

   float buf_l[4096];
   float buf_r[4096];
   float out_buffer[8092];

   size_t process(const float *in, unsigned frames)
   {
#ifdef PERF_TEST
      struct timespec tv_old;
      struct timespec tv_new;
      clock_gettime(CLOCK_MONOTONIC, &tv_old);
#endif

      float *buffer_l = buf_l;
      float *buffer_r = buf_r;

      for (unsigned i = 0; i < frames; i++)
      {
         buffer_l += dsp_eq_process(eq_l, buffer_l, buf_l + 4096 - buffer_l, in[(i << 1) + 0]);
         buffer_r += dsp_eq_process(eq_r, buffer_r, buf_r + 4096 - buffer_r, in[(i << 1) + 1]);
      }

      size_t out_samp = buffer_l - buf_l;

      for (unsigned i = 0; i < out_samp; i++)
      {
         out_buffer[(i << 1) + 0] = buf_l[i];
         out_buffer[(i << 1) + 1] = buf_r[i];
      }

#ifdef PERF_TEST
      clock_gettime(CLOCK_MONOTONIC, &tv_new);
      double time = (double)(tv_new.tv_sec - tv_old.tv_sec) + (tv_new.tv_nsec - tv_old.tv_nsec) / 1000000000.0;
      process_time += time;
      process_frames += frames;
      fprintf(stderr, "[Equalizer]: Processing @ %10.0f frames/s.\n", static_cast<float>(process_frames/process_time));
#endif

      return out_samp;
   }

   static float db2gain(float val)
   {
      return powf(10.0, val / 20.0);
   }

   static float noise()
   {
      return 2.0 * (static_cast<float>(rand()) / RAND_MAX - 0.5);
   }

   dsp_eq_state_t *eq_l;
   dsp_eq_state_t *eq_r;

#ifdef PERF_TEST
   double process_time;
   uint64_t process_frames;
#endif
};

static void *dsp_init(const rarch_dsp_info_t *info)
{
   return new PlugEQ(info->input_rate);
}

static void dsp_process(void *data, rarch_dsp_output_t *output,
      const rarch_dsp_input *input)
{
   PlugEQ *eq = reinterpret_cast<PlugEQ*>(data);

   output->samples = eq->out_buffer;
   size_t out_frames = eq->process(input->samples, input->frames);
   output->frames = out_frames;
   output->should_resample = RARCH_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugEQ*>(data);
}

static void dsp_config(void *)
{}

const rarch_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   RARCH_DSP_API_VERSION,
   dsp_config,
   "Equalizer"
};

RARCH_API_EXPORT const rarch_dsp_plugin_t* RARCH_API_CALLTYPE rarch_dsp_plugin_init(void)
{
   return &dsp_plug;
}

