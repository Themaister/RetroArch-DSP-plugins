#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "echo.h"
#include "inireader.h"
Echo echo_l;
Echo echo_r;
static void* dsp_init(const ssnes_dsp_info_t *info)
{
	 CIniReader iniReader("ssnes_effect.cfg");
	 int amp = iniReader.ReadInteger("echo", "amplification", 128); 
	 int delay = iniReader.ReadInteger("echo","delay",200);
	 echo_l.SetAmp(amp);
	 echo_l.SetDelay(delay);
	 echo_l.SetSampleRate(info->input_rate);
	 echo_r.SetAmp(amp);
	 echo_r.SetDelay(delay);
	 echo_r.SetSampleRate(info->input_rate);
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
		buf[i] = echo_l.Process(input->samples[i]);
		i++;
		buf[i] = echo_r.Process(input->samples[i]);
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
	"Echo plugin"
};

const ssnes_dsp_plugin_t* ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
