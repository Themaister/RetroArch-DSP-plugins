#include "meta.hpp"
#include "../inireader.h"
#include <iostream>

MetaDSP::MetaDSP(float input_rate, float output_rate) : sample_rate(input_rate)
{
   ssnes_dsp_info_t info = { sample_rate, sample_rate };
   CIniReader cfg("ssnes_effect.cfg");
   auto plug0 = cfg.ReadString("meta", "plugin0", "");
   auto plug1 = cfg.ReadString("meta", "plugin1", "");
   auto plug2 = cfg.ReadString("meta", "plugin2", "");
   auto plug3 = cfg.ReadString("meta", "plugin3", "");
   auto plug4 = cfg.ReadString("meta", "plugin4", "");
   auto plug5 = cfg.ReadString("meta", "plugin5", "");
   auto plug6 = cfg.ReadString("meta", "plugin6", "");
   auto plug7 = cfg.ReadString("meta", "plugin7", "");
   auto resamp_plug = cfg.ReadString("meta", "resampler_plugin", "");
   plugins[0] = std::make_shared<Plugin>(&info, plug0.c_str());
   plugins[1] = std::make_shared<Plugin>(&info, plug1.c_str());
   plugins[2] = std::make_shared<Plugin>(&info, plug2.c_str());
   plugins[3] = std::make_shared<Plugin>(&info, plug3.c_str());
   plugins[4] = std::make_shared<Plugin>(&info, plug4.c_str());
   plugins[5] = std::make_shared<Plugin>(&info, plug5.c_str());
   plugins[6] = std::make_shared<Plugin>(&info, plug6.c_str());
   plugins[7] = std::make_shared<Plugin>(&info, plug7.c_str());

   for (unsigned i = 0; i < max_plugs; i++)
   {
      if (plugins[i]->is_resampler())
         plugins[i] = std::make_shared<Plugin>();
   }

   info.output_rate = output_rate;
   resampler_plugin = std::make_shared<Plugin>(&info, resamp_plug.c_str());

   for (unsigned i = 0; i < max_plugs; i++)
      std::cerr << "[MetaDSP]: Plugin #" << i << ": " << plugins[i]->ident() << std::endl;
   std::cerr << "[MetaDSP]: Resampler: " << resampler_plugin->ident() << std::endl;
}

void MetaDSP::show()
{}

// Process the chain.
void MetaDSP::process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
   ssnes_dsp_input_t input[max_plugs];
   ssnes_dsp_output_t output[max_plugs];

   plugins[0]->process(&output[0], in);

   unsigned i;
   for (i = 1; i < max_plugs; i++)
   {
      input[i - 1].samples = output[i - 1].samples;
      input[i - 1].frames = output[i - 1].frames;
      plugins[i]->process(&output[i], &input[i - 1]);
   }
   i--;

   input[i].frames = output[i].frames;
   input[i].samples = output[i].samples;
   resampler_plugin->process(out, &input[i]);
}

