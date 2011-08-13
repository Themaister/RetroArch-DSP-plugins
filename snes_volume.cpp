#include "ssnes_dsp.h"
#include <iostream>
#include "abstract_plugin.hpp"
#include <math.h>

struct PlugVolume : public AbstractPlugin
{
   PlugVolume()
   {
      PluginOption opt = {0};

      opt.type = PluginOption::Type::Double;
      opt.id = VOLUME;
      opt.description = "Volume";
      opt.d.min = -80.0;
      opt.d.max = +24.0;
      opt.d.current = 0.0;
      dsp_options.push_back(opt);

      opt.type = PluginOption::Type::Integer;
      opt.id = PAN;
      opt.description = "Pan";
      opt.i.min = -100;
      opt.i.max = 100;
      opt.i.current = 0;
      dsp_options.push_back(opt);

      m_vol = 1.0;
      m_pan_vol_l = 1.0;
      m_pan_vol_r = 1.0;
   }

   enum IDs : PluginOption::ID { VOLUME, PAN };

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case VOLUME:
            m_vol = db2gain(val);
            break;

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

   void set_option_int(PluginOption::ID id, int val)
   {
      switch (id)
      {
         case PAN:
            pan2gain(m_pan_vol_l, m_pan_vol_r, val);

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

#ifdef __GNUC__
   float buf[4096] __attribute__((aligned(16)));
#else
   float buf[4096];
#endif

   void process(const float *in, unsigned frames)
   {
      float vol_left = m_vol * m_pan_vol_l;
      float vol_right = m_vol * m_pan_vol_r;
      for (unsigned i = 0; i < frames; i++)
      {
         buf[(i << 1) + 0] = in[(i << 1) + 0] * vol_left;
         buf[(i << 1) + 1] = in[(i << 1) + 1] * vol_right;
      }
   }

   float m_vol;
   float m_pan_vol_l;
   float m_pan_vol_r;

   static float db2gain(float val)
   {
      return powf(10.0, val / 20.0);
   }

   static void pan2gain(float &left, float &right, int val)
   {
      left = (100 - val) / 100.0f;
      right = (val + 100) / 100.0f;
      if (left > 1.0)
         left = 1.0;
      if (right > 1.0)
         right = 1.0;
   }
};

static void *dsp_init(const ssnes_dsp_info_t *)
{
   return new PlugVolume;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input *input)
{
   PlugVolume *vol = reinterpret_cast<PlugVolume*>(data);

   output->samples = vol->buf;
   vol->process(input->samples, input->frames);
   output->frames = input->frames;
   output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugVolume*>(data);
}

static void dsp_config(void *)
{}

const ssnes_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   SSNES_DSP_API_VERSION,
   dsp_config,
   "Volume plugin"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}

