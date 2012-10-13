#include "rarch_dsp.h"
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

   PlugPhaser(float freq, float startphase, float fb, int depth, int stages, int drywet) : AbstractPlugin()
   {
      PluginOption opt = {0};
      opt.type = PluginOption::Type::Double;

      opt.id = LFO_FREQ;
      opt.description = "LFO frequency";
      opt.d.min = 0.05;
      opt.d.max = 5.0;
      opt.d.current = freq;
      opt.conf_name = "phaser_lfo_frequency";
      dsp_options.push_back(opt);

      opt.id = STARTPHASE;
      opt.description = "LFO start phase";
      opt.d.min = 0;
      opt.d.max = 256;
      opt.d.current = startphase;
      opt.conf_name = "phaser_lfo_start_phase";
      dsp_options.push_back(opt);

      opt.id = LFO_FB;
      opt.description = "LFO feedback";
      opt.d.min = 0;
      opt.d.max = 1;
      opt.d.current = fb;
      opt.conf_name = "phaser_lfo_feedback";
      dsp_options.push_back(opt);

      opt.id = DEPTH;
      opt.description = "LFO depth";
      opt.d.min = 0;
      opt.d.max = 200;
      opt.d.current = depth;
      opt.conf_name = "phaser_lfo_depth";
      dsp_options.push_back(opt);

      opt.id = DRYWET;
      opt.description = "LFO dry/wet ratio";
      opt.d.min = 0;
      opt.d.max = 256;
      opt.d.current = drywet;
      opt.conf_name = "phaser_lfo_dry_wet_ratio";
      dsp_options.push_back(opt);

      opt.type = PluginOption::Type::Integer;
      opt.id = STAGES;
      opt.description = "LFO stage amount";
      opt.i.min = 0;
      opt.i.max = 10;
      opt.i.current = stages;
      opt.conf_name = "phaser_lfo_stage_amount";
      dsp_options.push_back(opt);

      load_options("rarch_effect.cfg");
   }

   ~PlugPhaser()
   {
      save_options("rarch_effect.cfg");
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case LFO_FREQ:
            phase_l.SetLFOFreq(val);
            phase_r.SetLFOFreq(val);
            break;

         case STARTPHASE:
            phase_l.SetLFOStartPhase(val);
            phase_r.SetLFOStartPhase(val);
            break;

         case LFO_FB:
            phase_l.SetFeedback(val);
            phase_r.SetFeedback(val);
            break;

         case DEPTH:
            phase_l.SetDepth(val);
            phase_r.SetDepth(val);
            break;

         case DRYWET:
            phase_l.SetDryWet(val);
            phase_r.SetDryWet(val);
            break;

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

   void set_option_int(PluginOption::ID id, int val)
   {
      switch (id)
      {
         case STAGES:
            phase_l.SetStages(val);
            phase_r.SetStages(val);
            break;

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

   enum IDs : PluginOption::ID { LFO_FREQ, STARTPHASE, LFO_FB,
      DEPTH, STAGES, DRYWET };
};

static void* dsp_init(const rarch_dsp_info_t *info)
{
   float freq = 0.4; 
   float startphase = 0;
   float fb = 0;
   int depth = 100;
   int stages = 2;
   int drywet = 128;

   PlugPhaser *phaser = new PlugPhaser(freq, startphase, fb, depth, stages, drywet);
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

static void dsp_process(void *data, rarch_dsp_output_t *output,
      const rarch_dsp_input_t *input)
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
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugPhaser*>(data);
}

static void dsp_config(void *)
{}

const rarch_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	RARCH_DSP_API_VERSION,
	dsp_config,
	"Phaser plugin"
};

RARCH_API_EXPORT const rarch_dsp_plugin_t* RARCH_API_CALLTYPE rarch_dsp_plugin_init(void)
{
   return &dsp_plug;
}

