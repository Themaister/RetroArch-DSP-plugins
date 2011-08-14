#include "ssnes_dsp.h"
#include <iostream>
#include "abstract_plugin.hpp"
#include <math.h>
#include "eq.h"
#include <stdlib.h>

struct PlugEQ : public AbstractPlugin
{
   PlugEQ(float rate)
   {
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
      dsp_eq_set_gain(eq_l, id, db2gain(val));
      dsp_eq_set_gain(eq_r, id, db2gain(val));
   }

   float buf[4096];

   void process(const float *in, unsigned frames)
   {
      for (unsigned i = 0; i < frames; i++)
      {
         buf[(i << 1) + 0] = dsp_eq_process(eq_l, in[(i << 1) + 0]);
         buf[(i << 1) + 1] = dsp_eq_process(eq_r, in[(i << 1) + 1]);
      }
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
};

static void *dsp_init(const ssnes_dsp_info_t *info)
{
   return new PlugEQ(info->input_rate);
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input *input)
{
   PlugEQ *eq = reinterpret_cast<PlugEQ*>(data);

   output->samples = eq->buf;
   eq->process(input->samples, input->frames);
   output->frames = input->frames;
   output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugEQ*>(data);
}

static void dsp_config(void *)
{}

const ssnes_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   SSNES_DSP_API_VERSION,
   dsp_config,
   "Equalizer"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}

