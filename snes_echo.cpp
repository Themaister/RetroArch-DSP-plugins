#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "echo.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

struct PlugEcho : public AbstractPlugin
{
   Echo echo_l;
   Echo echo_r;
   float buf[4096];
};

static void* dsp_init(const ssnes_dsp_info_t *info)
{
   CIniReader iniReader("ssnes_effect.cfg");
   int amp = iniReader.ReadInteger("echo", "amplification", 128); 
   int delay = iniReader.ReadInteger("echo","delay",200);

   PlugEcho *echo = new PlugEcho;
   echo->echo_l.SetAmp(amp);
   echo->echo_l.SetDelay(delay);
   echo->echo_l.SetSampleRate(info->input_rate);
   echo->echo_r.SetAmp(amp);
   echo->echo_r.SetDelay(delay);
   echo->echo_r.SetSampleRate(info->input_rate);

   return echo;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   PlugEcho *echo = reinterpret_cast<PlugEcho*>(data);
	output->samples = echo->buf;
	int num_samples = input->frames * 2;
	for (int i = 0; i<num_samples;)
	{
		echo->buf[i] = echo->echo_l.Process(input->samples[i]);
		i++;
		echo->buf[i] = echo->echo_r.Process(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugEcho*>(data);
}

static void dsp_config(void *)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"Echo plugin"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
