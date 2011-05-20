#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "wahwah.h"
#include "inireader.h"
WahWah    wah_l;
WahWah    wah_r;
static void* dsp_init(const ssnes_dsp_info_t *info)
{
	 CIniReader iniReader("ssnes_effect.cfg");
	 float freq = iniReader.ReadFloat("wah", "lfo_frequency",1.5); 
	 float startphase = iniReader.ReadFloat("wah","lfo_start_phase",0.0);
	 float res = iniReader.ReadFloat("wah","lfo_resonance",2.5);
	 float depth = iniReader.ReadInteger("wah","lfo_depth",0.70);
	 float freqofs = iniReader.ReadInteger("wah","lfo_frequency_offset",0.30);
	 wah_l.SetDepth(depth);
	 wah_l.SetFreqOffset(freqofs);
	 wah_l.SetLFOFreq(freq);
	 wah_l.SetLFOStartPhase(startphase);
	 wah_l.SetResonance(res);
	 wah_l.init(info->input_rate);
	 wah_r.SetDepth(depth);
	 wah_r.SetFreqOffset(freqofs);
	 wah_r.SetLFOFreq(freq);
	 wah_r.SetLFOStartPhase(startphase);
	 wah_r.SetResonance(res);
	 wah_r.init(info->input_rate);
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
		buf[i] = wah_l.Process(input->samples[i]);
		i++;
		buf[i] = wah_r.Process(input->samples[i]);
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
	"Wah plugin"
};

const ssnes_dsp_plugin_t* ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
