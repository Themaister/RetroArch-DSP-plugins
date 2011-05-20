#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "phaser.h"
#include "wahwah.h"
#include "inireader.h"
Phaser    phase_l;
Phaser    phase_r;
static void* dsp_init(const ssnes_dsp_info_t *info)
{
	 CIniReader iniReader("ssnes_effect.cfg");
	 float freq = iniReader.ReadFloat("phaser", "lfo_frequency", 0.4); 
	 float startphase = iniReader.ReadFloat("phaser","lfo_start_phase",0);
	 float fb = iniReader.ReadFloat("phaser","lfo_feedback",0);
	 int depth = iniReader.ReadInteger("phaser","lfo_depth",100);
	 int stages = iniReader.ReadInteger("phaser","lfo_stage_amount",2);
	 int drywet = iniReader.ReadInteger("phaser","lfo_dry_wet_ratio",128);
	 phase_l.SetLFOFreq(freq);
	 phase_l.SetLFOStartPhase(startphase);
	 phase_l.SetFeedback(fb);
	 phase_l.SetDepth(depth);
	 phase_l.SetStages(stages);
	 phase_l.SetDryWet(drywet);
	 phase_l.init(info->input_rate);
	 phase_r.SetLFOFreq(freq);
	 phase_r.SetLFOStartPhase(startphase);
	 phase_r.SetFeedback(fb);
	 phase_r.SetDepth(depth);
	 phase_r.SetStages(stages);
	 phase_r.SetDryWet(drywet);
	 phase_r.init(info->input_rate);
    (void)info;
    return (void*)-1;
}

float buf[4096];
static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
	(void)data;
	output->samples = buf;
	int num_samples = input->frames * 2;
	for (int i = 0; i<num_samples;)
	{
		buf[i] = phase_l.Process(input->samples[i]);
		i++;
		buf[i] = phase_r.Process(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   (void)data;
}

static void dsp_config(void *data)
{
	(void)data;
	// Normally we unhide a GUI window or something,
	// but we're just going to print to the log instead.
}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"Phaser plugin"
};

const ssnes_dsp_plugin_t* ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
