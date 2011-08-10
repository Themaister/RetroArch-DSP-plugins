#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "phaser.h"
#include "wahwah.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

struct PlugPhaser : public AbstractPlugin
{
   Phaser phase_l;
   Phaser phase_r;
   float buf[4096];
};

static void* dsp_init(const ssnes_dsp_info_t *info)
{
   CIniReader iniReader("ssnes_effect.cfg");
   float freq = iniReader.ReadFloat("phaser", "lfo_frequency", 0.4); 
   float startphase = iniReader.ReadFloat("phaser","lfo_start_phase",0);
   float fb = iniReader.ReadFloat("phaser","lfo_feedback",0);
   int depth = iniReader.ReadInteger("phaser","lfo_depth",100);
   int stages = iniReader.ReadInteger("phaser","lfo_stage_amount",2);
   int drywet = iniReader.ReadInteger("phaser","lfo_dry_wet_ratio",128);

   PlugPhaser *phaser = new PlugPhaser;
   phaser->phase_l.SetLFOFreq(freq);
   phaser->phase_l.SetLFOStartPhase(startphase);
   phaser->phase_l.SetFeedback(fb);
   phaser->phase_l.SetDepth(depth);
   phaser->phase_l.SetStages(stages);
   phaser->phase_l.SetDryWet(drywet);
   phaser->phase_l.init(info->input_rate);
   phaser->phase_r.SetLFOFreq(freq);
   phaser->phase_r.SetLFOStartPhase(startphase);
   phaser->phase_r.SetFeedback(fb);
   phaser->phase_r.SetDepth(depth);
   phaser->phase_r.SetStages(stages);
   phaser->phase_r.SetDryWet(drywet);
   phaser->phase_r.init(info->input_rate);

   return phaser;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   PlugPhaser *phaser = reinterpret_cast<PlugPhaser*>(data);

   output->samples = phaser->buf;
   int num_samples = input->frames * 2;
   for (int i = 0; i<num_samples;)
   {
		phaser->buf[i] = phaser->phase_l.Process(input->samples[i]);
		i++;
		phaser->buf[i] = phaser->phase_r.Process(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugPhaser*>(data);
}

static void dsp_config(void *)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"Phaser plugin"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}

