#include "meta.hpp"
#include "../../inireader.h"

MetaDSP::MetaDSP(float input_rate) : sample_rate(input_rate)
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
   plugins[0] = std::make_shared<Plugin>(&info, plug0.c_str());
   plugins[1] = std::make_shared<Plugin>(&info, plug1.c_str());
   plugins[2] = std::make_shared<Plugin>(&info, plug2.c_str());
   plugins[3] = std::make_shared<Plugin>(&info, plug3.c_str());
   plugins[4] = std::make_shared<Plugin>(&info, plug4.c_str());
   plugins[5] = std::make_shared<Plugin>(&info, plug5.c_str());
   plugins[6] = std::make_shared<Plugin>(&info, plug6.c_str());
   plugins[7] = std::make_shared<Plugin>(&info, plug7.c_str());
}

void MetaDSP::show()
{}

// Process the chain.
void MetaDSP::process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
   ssnes_dsp_input_t input[max_plugs - 1];
   ssnes_dsp_output_t output[max_plugs - 1];

   plugins[0]->process(&output[0], in);
   for (unsigned i = 1; i < max_plugs - 1; i++)
   {
      input[i - 1].samples = output[i - 1].samples;
      input[i - 1].frames = output[i - 1].frames;
      plugins[i]->process(&output[i], &input[i - 1]);
   }

   input[max_plugs - 2].frames = output[max_plugs - 2].frames;
   input[max_plugs - 2].samples = output[max_plugs - 2].samples;
   plugins[max_plugs - 1]->process(out, &input[max_plugs - 2]);
   out->should_resample = SSNES_TRUE;
}
