#include "rarch_dsp.h"
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

   PlugIIR(int input_rate, float freq = 1024, float gain = 0) :
      AbstractPlugin(), rate(input_rate), type(0)
   {
      PluginOption opt{};

      opt.type = PluginOption::Type::Double;
      opt.id = FREQ;
      opt.description = "Frequency";
      opt.d.min = 50;
      opt.d.max = 16000;
      opt.d.current = freq;
      opt.conf_name = "iir_filter_frequency";
      dsp_options.push_back(opt);

      opt.id = GAIN;
      opt.description = "Gain";
      opt.d.min = -50.0;
      opt.d.max = 50.0;
      opt.d.current = gain;
      opt.conf_name = "iir_filter_gain";
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
      
      opt.conf_name = "iir_type";
      dsp_options.push_back(opt);

      load_options("rarch_effect.cfg");
   }

   ~PlugIIR()
   {
      save_options("rarch_effect.cfg");
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

static void* dsp_init(const rarch_dsp_info_t *info)
{
   PlugIIR *iir = new PlugIIR(info->input_rate);
   iir->iir_l.setFrequency(1024);
   iir->iir_l.setQuality(0.707);
   iir->iir_l.setGain(0);
   iir->iir_l.init(info->input_rate, 0);
   iir->iir_r.setFrequency(1024);
   iir->iir_r.setQuality(0.707);
   iir->iir_r.setGain(0);
   iir->iir_r.init(info->input_rate, 0);

   return iir;
}

static void dsp_process(void *data, rarch_dsp_output_t *output,
      const rarch_dsp_input_t *input)
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
}

static void dsp_free(void *data)
{
   delete reinterpret_cast<PlugIIR*>(data);
}

static void dsp_config(void*)
{}

const rarch_dsp_plugin_t dsp_plug = {
	dsp_init,
	dsp_process,
	dsp_free,
	RARCH_DSP_API_VERSION,
	dsp_config,
#ifdef __SSE2__
	"IIR filter set (SSE2)"
#else
	"IIR filter set"
#endif
};


RARCH_API_EXPORT const rarch_dsp_plugin_t* RARCH_API_CALLTYPE rarch_dsp_plugin_init(void)
{
   return &dsp_plug;
}
