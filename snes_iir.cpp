#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "iirfilters.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

struct PlugIIR : public AbstractPlugin
{
   IIRFilter iir_l;
   IIRFilter iir_r;
   float buf[4096];

   PlugIIR(float freq, float gain)
   {
      /*
      PluginOption opt = {0};

      opt.id = FREQ;
      opt.description = "Frequency";
      opt.min = 50;
      opt.max = 16000;
      opt.current = freq;
      dsp_options.push_back(opt);

      opt.id = GAIN;
      opt.description = "Gain";
      opt.min = -20;
      opt.max = 20;
      opt.current = gain;
      dsp_options.push_back(opt);
      */
   }

   void set_option(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case FREQ:
            iir_l.setFrequency(val);
            iir_r.setFrequency(val);
            break;

         case GAIN:
            iir_l.setGain(val);
            iir_r.setGain(val);
            break;
      }
   }

   enum IDs : PluginOption::ID { FREQ, GAIN };
};

static void* dsp_init(const ssnes_dsp_info_t *info)
{
   CIniReader iniReader("ssnes_effect.cfg");
   int type = iniReader.ReadInteger("iir", "type", 0); 
   float freq = iniReader.ReadFloat("iir","filter_frequency",1024.0);
   int gain = iniReader.ReadInteger("iir","filter_gain",5);

   PlugIIR *iir = new PlugIIR(freq, gain);
   iir->iir_l.init(info->input_rate,type);
   iir->iir_l.setFrequency(freq);
   iir->iir_l.setQuality(0.707);
   iir->iir_l.setGain(gain);
   iir->iir_r.init(info->input_rate,type);
   iir->iir_r.setFrequency(freq);
   iir->iir_r.setQuality(0.707);
   iir->iir_r.setGain(gain);

   return iir;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   PlugIIR *iir = reinterpret_cast<PlugIIR*>(data);

   output->samples = iir->buf;
   int num_samples = input->frames * 2;
   for (int i = 0; i<num_samples;)
	{
		iir->buf[i] = iir->iir_l.Process(input->samples[i]);
		i++;
		iir->buf[i] = iir->iir_r.Process(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugIIR*>(data);
}

static void dsp_config(void*)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"IIR filter set"
};


SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
