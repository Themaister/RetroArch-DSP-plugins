#include "ssnes_dsp.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "iirfilters.h"
#include "inireader.h"
#include "abstract_plugin.hpp"

#ifdef PERF_TEST
#include "timer.hpp"
#endif

struct PlugIIR : public AbstractPlugin
{
   IIRFilter iir_l __attribute__((aligned(16)));
   IIRFilter iir_r __attribute__((aligned(16)));
   float buf[4096] __attribute__((aligned(16)));

#ifdef PERF_TEST
   Timer timer;
#endif

   PlugIIR(int input_rate, float freq, float gain) :
      AbstractPlugin(), rate(input_rate), type(0)
   {
      PluginOption opt = {0};

      opt.type = PluginOption::Type::Double;
      opt.id = FREQ;
      opt.description = "Frequency";
      opt.d.min = 50;
      opt.d.max = 16000;
      opt.d.current = freq;
      dsp_options.push_back(opt);

      opt.id = GAIN;
      opt.description = "Gain";
      opt.d.min = -50.0;
      opt.d.max = 50.0;
      opt.d.current = gain;
      dsp_options.push_back(opt);

      opt.type = PluginOption::Type::Selection;
      opt.id = TYPE;
      opt.description = "Filter type";
      opt.s.current = LPF;

      opt.s.selection.push_back(PluginOption::Selection(LPF, "Low-pass filter"));
      opt.s.selection.push_back(PluginOption::Selection(HPF, "High-pass filter"));
      opt.s.selection.push_back(PluginOption::Selection(BPCSGF, "Band-pass filter 1"));
      opt.s.selection.push_back(PluginOption::Selection(BPZPGF, "Band-pass filter 2"));
      opt.s.selection.push_back(PluginOption::Selection(APF, "All-pass filter"));
      opt.s.selection.push_back(PluginOption::Selection(NOTCH, "Notch filter"));
      opt.s.selection.push_back(PluginOption::Selection(RIAA_phono, "RIAA record/tape de-emphasis"));
      opt.s.selection.push_back(PluginOption::Selection(PEQ, "Peaking band EQ filter"));
      opt.s.selection.push_back(PluginOption::Selection(BBOOST, "Bassboost filter"));
      opt.s.selection.push_back(PluginOption::Selection(LSH, "Low-shelf filter"));
      opt.s.selection.push_back(PluginOption::Selection(HSH, "High-shelf filter"));
      opt.s.selection.push_back(PluginOption::Selection(RIAA_CD, "CD de-emphasis"));
      
      dsp_options.push_back(opt);
   }

   void set_option_double(PluginOption::ID id, double val)
   {
      switch (id)
      {
         case FREQ:
            iir_l.setFrequency(val);
            iir_r.setFrequency(val);
            iir_l.init(rate, type);
            iir_r.init(rate, type);
            break;

         case GAIN:
            iir_l.setGain(val);
            iir_r.setGain(val);
            iir_l.init(rate, type);
            iir_r.init(rate, type);
            break;

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

   void set_option_selection(PluginOption::ID id, PluginOption::ID optid)
   {
      switch (id)
      {
         case TYPE:
            type = optid;
            iir_l.init(rate, type);
            iir_r.init(rate, type);
            break;

         default:
            std::cerr << "Err ..." << std::endl;
      }
   }

   enum IDs : PluginOption::ID { FREQ, GAIN, TYPE };

   int rate;
   unsigned type;
};

static void* dsp_init(const ssnes_dsp_info_t *info)
{
   ConfigFile cfg("ssnes_effect.cfg");
   int type = cfg.get_int("iir_type", 0); 
   float freq = cfg.get_float("iir_filter_frequency", 1024.0);
   float gain = cfg.get_float("iir_filter_gain", 0.0);

   PlugIIR *iir = new PlugIIR(info->input_rate, freq, gain);
   iir->iir_l.setFrequency(freq);
   iir->iir_l.setQuality(0.707);
   iir->iir_l.setGain(gain);
   iir->iir_l.init(info->input_rate, type);
   iir->iir_r.setFrequency(freq);
   iir->iir_r.setQuality(0.707);
   iir->iir_r.setGain(gain);
   iir->iir_r.init(info->input_rate, type);

   return iir;
}

static void dsp_process(void *data, ssnes_dsp_output_t *output,
      const ssnes_dsp_input_t *input)
{
   PlugIIR *iir = reinterpret_cast<PlugIIR*>(data);

   output->samples = iir->buf;

#ifdef PERF_TEST
   iir->timer.start();
#endif

#ifdef __SSE2__
   iir->iir_l.ProcessBatch(iir->buf, input->samples, input->frames);
#else
   int num_samples = input->frames * 2;
   for (int i = 0; i<num_samples;)
	{
		iir->buf[i] = iir->iir_l.Process(input->samples[i]);
		i++;
		iir->buf[i] = iir->iir_r.Process(input->samples[i]);
		i++;
	}
#endif

#ifdef PERF_TEST
   iir->timer.stop(input->frames);
#endif

	output->frames = input->frames;
	output->should_resample = SSNES_TRUE;
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugIIR*>(data);
}

static void dsp_config(void*)
{}

const ssnes_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	SSNES_DSP_API_VERSION,
	dsp_config,
#ifdef __SSE2__
	"IIR filter set (SSE2)"
#else
	"IIR filter set"
#endif
};


SSNES_API_EXPORT const ssnes_dsp_plugin_t* SSNES_API_CALLTYPE ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
