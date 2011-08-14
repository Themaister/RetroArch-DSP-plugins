#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freeverb.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

struct PlugReverb : public AbstractPlugin
{
   revmodel rev_l;
   revmodel rev_r;
   float buf[4096];

   PlugReverb(float drytime, float wettime, float damping,
         float roomwidth, float roomsize) : AbstractPlugin()
   {
      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.id = DRYTIME;
      opt.description = "Dry time";
      opt.d.min = 0.01;
      opt.d.max = 2.0;
      opt.d.current = drytime;
      dsp_options.push_back(opt);

      opt.id = WETTIME;
      opt.description = "Wet time";
      opt.d.min = 0.01;
      opt.d.max = 2.0;
      opt.d.current = wettime;
      dsp_options.push_back(opt);

      opt.id = DAMPING;
      opt.description = "Damping";
      opt.d.min = 0.01;
      opt.d.max = 2.0;
      opt.d.current = damping;
      dsp_options.push_back(opt);

      opt.id = ROOMWIDTH;
      opt.description = "Room width";
      opt.d.min = 0.01;
      opt.d.max = 2.0;
      opt.d.current = roomwidth;
      dsp_options.push_back(opt);

      opt.id = ROOMSIZE;
      opt.description = "Room size";
      opt.d.min = 0.01;
      opt.d.max = 2.0;
      opt.d.current = roomsize;
      dsp_options.push_back(opt);
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case DRYTIME:
            rev_l.setdry(val);
            rev_r.setdry(val);
            break;

         case WETTIME:
            rev_l.setwet(val);
            rev_r.setwet(val);
            break;

         case DAMPING:
            rev_l.setdamp(val);
            rev_r.setdamp(val);
            break;

         case ROOMWIDTH:
            rev_l.setwidth(val);
            rev_r.setwidth(val);
            break;

         case ROOMSIZE:
            rev_l.setroomsize(val);
            rev_r.setroomsize(val);
            break;
      }
   }

   enum IDs : PluginOption::ID { DRYTIME, WETTIME, DAMPING, ROOMWIDTH, ROOMSIZE };
};

static void* dsp_init(const ssnes_dsp_info_t *)
{
   CIniReader iniReader("ssnes_effect.cfg");
   float drytime = iniReader.ReadFloat("reverb", "dry_time", 0.43); 
   float wettime = iniReader.ReadFloat("reverb","wet_time",0.57);
   float damping = iniReader.ReadFloat("reverb","damping",0.45);
   float roomwidth = iniReader.ReadFloat("reverb","room_width",0.56);
   float roomsize = iniReader.ReadFloat("reverb","room_size",0.56);

   PlugReverb *rev = new PlugReverb(drytime, wettime, damping, roomwidth, roomsize);
   rev->rev_l.setdamp(damping);
   rev->rev_l.setdry(drytime);
   rev->rev_l.setwet(wettime);
   rev->rev_l.setwidth(roomwidth);
   rev->rev_l.setroomsize(roomsize);
   rev->rev_r.setdamp(damping);
   rev->rev_r.setdry(drytime);
   rev->rev_r.setwet(wettime);
   rev->rev_r.setwidth(roomwidth);
   rev->rev_r.setroomsize(roomsize);

   return rev;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   PlugReverb *rev = reinterpret_cast<PlugReverb*>(data);

	output->samples = rev->buf;
	int num_samples = input->frames * 2;
	for (int i = 0; i<num_samples;)
	{
		rev->buf[i] = rev->rev_l.processsample(input->samples[i]);
		i++;
		rev->buf[i] = rev->rev_r.processsample(input->samples[i]);
		i++;
	}
	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugReverb*>(data);
}

static void dsp_config(void*)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
	"Reverberatation plugin"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}

