#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freeverb.h"
#include "inireader.h"
revmodel rev_l;
revmodel rev_r;
static void* dsp_init(const ssnes_dsp_info_t *info)
{
	 CIniReader iniReader("ssnes_effect.cfg");
	 float drytime = iniReader.ReadFloat("reverb", "dry_time", 0.43); 
	 float wettime = iniReader.ReadFloat("reverb","wet_time",0.57);
	 float damping = iniReader.ReadFloat("reverb","damping",0.45);
	 float roomwidth = iniReader.ReadFloat("reverb","room_width",0.56);
	 float roomsize = iniReader.ReadFloat("reverb","room_size",0.56);
	 rev_l.setdamp(damping);
	 rev_l.setdry(drytime);
	 rev_l.setwet(wettime);
	 rev_l.setwidth(roomwidth);
	 rev_l.setroomsize(roomsize);
	 rev_r.setdamp(damping);
	 rev_r.setdry(drytime);
	 rev_r.setwet(wettime);
	 rev_r.setwidth(roomwidth);
	 rev_r.setroomsize(roomsize);
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
		buf[i] = rev_l.processsample(input->samples[i]);
		i++;
		buf[i] = rev_r.processsample(input->samples[i]);
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
	"Reverberatation plugin"
};
const ssnes_dsp_plugin_t* ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
