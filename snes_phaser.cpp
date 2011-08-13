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

   PlugPhaser(float freq, float startphase, float fb, int depth, int stages, int drywet)
   {
      /*
      PluginOption opt = {0};

      opt.id = LFO_FREQ;
      opt.description = "LFO frequency";
      opt.min = 0.05;
      opt.max = 5.0;
      opt.current = freq;
      dsp_options.push_back(opt);

      opt.id = STARTPHASE;
      opt.description = "LFO start phase";
      opt.min = 0;
      opt.max = 256;
      opt.current = startphase;
      dsp_options.push_back(opt);

      opt.id = LFO_FB;
      opt.description = "LFO feedback";
      opt.min = 0;
      opt.max = 1;
      opt.current = fb;
      dsp_options.push_back(opt);

      opt.id = DEPTH;
      opt.description = "LFO depth";
      opt.min = 0;
      opt.max = 200;
      opt.current = depth;
      dsp_options.push_back(opt);

      opt.id = STAGES;
      opt.description = "LFO stage amount";
      opt.min = 0;
      opt.max = 10;
      opt.current = stages;
      dsp_options.push_back(opt);

      opt.id = DRYWET;
      opt.description = "LFO dry/wet ratio";
      opt.min = 0;
      opt.max = 256;
      opt.current = drywet;
      dsp_options.push_back(opt);
      */
   }

   void set_option(PluginOption::ID id, double val)
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

         case STAGES:
            phase_l.SetStages(val);
            phase_r.SetStages(val);
            break;

         case DRYWET:
            phase_l.SetDryWet(val);
            phase_r.SetDryWet(val);
            break;
      }
   }

   enum IDs : PluginOption::ID { LFO_FREQ, STARTPHASE, LFO_FB,
      DEPTH, STAGES, DRYWET };
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

