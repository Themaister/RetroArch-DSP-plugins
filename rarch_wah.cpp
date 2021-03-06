#include "rarch_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wahwah.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

struct PlugWah : public AbstractPlugin
{
   WahWah wah_l;
   WahWah wah_r;
   float buf[4096];

   PlugWah(float freq, float startphase, float res, float depth, float freqofs) : AbstractPlugin()
   {
      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.id = FREQ;
      opt.description = "LFO frequency";
      opt.d.min = 0.1;
      opt.d.max = 10.0;
      opt.d.current = freq;
      opt.conf_name = "wah_lfo_frequency";
      dsp_options.push_back(opt);

      opt.id = STARTPHASE;
      opt.description = "LFO start phase";
      opt.d.min = 0.0;
      opt.d.max = 360.0;
      opt.d.current = startphase;
      opt.conf_name = "wah_lfo_start_phase";
      dsp_options.push_back(opt);

      opt.id = RES;
      opt.description = "LFO resonance";
      opt.d.min = 0.0;
      opt.d.max = 10.0;
      opt.d.current = res;
      opt.conf_name = "wah_lfo_resonance";
      dsp_options.push_back(opt);

      opt.id = DEPTH;
      opt.description = "LFO depth";
      opt.d.min = 0.0;
      opt.d.max = 1.0;
      opt.d.current = depth;
      opt.conf_name = "wah_lfo_depth";
      dsp_options.push_back(opt);

      opt.id = FREQOFS;
      opt.description = "LFO frequency offset";
      opt.d.min = 0.0;
      opt.d.max = 0.95;
      opt.d.current = freqofs;
      opt.conf_name = "wah_lfo_frequency_offset";
      dsp_options.push_back(opt);

      load_options("rarch_effect.cfg");
   }

   ~PlugWah()
   {
      save_options("rarch_effect.cfg");
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case FREQ:
            wah_l.SetLFOFreq(val);
            wah_r.SetLFOFreq(val);
            break;

         case STARTPHASE:
            wah_l.SetLFOStartPhase(val);
            wah_r.SetLFOStartPhase(val);
            break;

         case RES:
            wah_l.SetResonance(val);
            wah_r.SetResonance(val);
            break;

         case DEPTH:
            wah_l.SetDepth(val);
            wah_r.SetDepth(val);
            break;

         case FREQOFS:
            wah_l.SetFreqOffset(val);
            wah_r.SetFreqOffset(val);
            break;
      }
   }

   enum IDs : PluginOption::ID { FREQ, STARTPHASE, RES, DEPTH, FREQOFS };
};

static void* dsp_init(const rarch_dsp_info_t *info)
{
   float freq = 1.5; 
   float startphase = 0.0;
   float res = 2.5;
   float depth = 0.70;
   float freqofs = 0.30;

   PlugWah *wah = new PlugWah(freq, startphase, res, depth, freqofs);
   wah->wah_l.SetDepth(depth);
   wah->wah_l.SetFreqOffset(freqofs);
   wah->wah_l.SetLFOFreq(freq);
   wah->wah_l.SetLFOStartPhase(startphase);
   wah->wah_l.SetResonance(res);
   wah->wah_l.init(info->input_rate);
   wah->wah_r.SetDepth(depth);
   wah->wah_r.SetFreqOffset(freqofs);
   wah->wah_r.SetLFOFreq(freq);
   wah->wah_r.SetLFOStartPhase(startphase);
   wah->wah_r.SetResonance(res);
   wah->wah_r.init(info->input_rate);

   return wah;
}

static void dsp_process(void *data, rarch_dsp_output_t *output,
      const rarch_dsp_input_t *input)
{
   PlugWah *wah = reinterpret_cast<PlugWah*>(data);
	output->samples = wah->buf;
	int num_samples = input->frames * 2;
	for (int i = 0; i<num_samples;)
	{
		wah->buf[i] = wah->wah_l.Process(input->samples[i]);
		i++;
		wah->buf[i] = wah->wah_r.Process(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugWah*>(data);
}

static void dsp_config(void*)
{}

const rarch_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	RARCH_DSP_API_VERSION,
	dsp_config,
	"Wah plugin"
};

RARCH_API_EXPORT const rarch_dsp_plugin_t* RARCH_API_CALLTYPE rarch_dsp_plugin_init(void)
{
   return &dsp_plug;
}
