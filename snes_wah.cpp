#include "ssnes_dsp.h"
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

   PlugWah(float freq, float startphase, float res, float depth, float freqofs)
   {
      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.id = FREQ;
      opt.description = "LFO frequency";
      opt.d.min = 0.1;
      opt.d.max = 10.0;
      opt.d.current = freq;
      dsp_options.push_back(opt);

      opt.id = STARTPHASE;
      opt.description = "LFO start phase";
      opt.d.min = 0.0;
      opt.d.max = 360.0;
      opt.d.current = startphase;
      dsp_options.push_back(opt);

      opt.id = RES;
      opt.description = "LFO resonance";
      opt.d.min = 0.0;
      opt.d.max = 10.0;
      opt.d.current = res;
      dsp_options.push_back(opt);

      opt.id = DEPTH;
      opt.description = "LFO depth";
      opt.d.min = 0.0;
      opt.d.max = 1.0;
      opt.d.current = depth;
      dsp_options.push_back(opt);

      opt.id = FREQOFS;
      opt.description = "LFO frequency offset";
      opt.d.min = 0.0;
      opt.d.max = 0.95;
      opt.d.current = freqofs;
      dsp_options.push_back(opt);
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

static void* dsp_init(const ssnes_dsp_info_t *info)
{
   CIniReader iniReader("ssnes_effect.cfg");
   float freq = iniReader.ReadFloat("wah", "lfo_frequency",1.5); 
   float startphase = iniReader.ReadFloat("wah","lfo_start_phase",0.0);
   float res = iniReader.ReadFloat("wah","lfo_resonance",2.5);
   float depth = iniReader.ReadFloat("wah","lfo_depth",0.70);
   float freqofs = iniReader.ReadFloat("wah","lfo_frequency_offset",0.30);

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

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
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
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugWah*>(data);
}

static void dsp_config(void*)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"Wah plugin"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
