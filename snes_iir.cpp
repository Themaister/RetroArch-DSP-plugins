#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "iirfilters.h"
#include "inireader.h"
IIRFilter iir_l;
IIRFilter iir_r;
static void* dsp_init(const ssnes_dsp_info_t *info)
{
	 CIniReader iniReader("ssnes_effect.cfg");
	 int type = iniReader.ReadInteger("iir", "type", 0); 
	 float freq = iniReader.ReadFloat("iir","filter_frequency",1024.0);
	 int gain = iniReader.ReadInteger("iir","filter_gain",5);
	 iir_l.init(info->input_rate,type);
	 iir_l.setFrequency(freq);
	 iir_l.setQuality(0.707);
	 iir_l.setGain(gain);
	 iir_r.init(info->input_rate,type);
	 iir_r.setFrequency(freq);
	 iir_r.setQuality(0.707);
	 iir_r.setGain(gain);
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
		buf[i] = iir_l.Process(input->samples[i]);
		i++;
		buf[i] = iir_r.Process(input->samples[i]);
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
	"IIR filter set"
};


const ssnes_dsp_plugin_t* ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
