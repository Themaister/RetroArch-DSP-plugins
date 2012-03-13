#include "meta.hpp"
#include "../inireader.h"
#include "../utils.hpp"
#include <iostream>

MetaDSP::MetaDSP(float input_rate, float output_rate) : sample_rate(input_rate)
{
   ssnes_dsp_info_t info = { sample_rate, sample_rate };
   Global::set_dsp_info(info);
   ConfigFile cfg("ssnes_effect.cfg");

   for (unsigned i = 0; i < max_plugs; i++)
   {
      plugins[i] = std::make_shared<Plugin>(&info, cfg.get_string(Utils::join("meta_plugin", i), "").c_str());
      if (plugins[i]->is_resampler())
         plugins[i] = std::make_shared<Plugin>();
      plugins[i]->enabled(cfg.get_bool(Utils::join("plugin_enabled", i), true));
   }

   info.output_rate = output_rate;

   resampler_plugin = std::make_shared<Plugin>(&info,
         cfg.get_string("meta_resampler_plugin", "").c_str());

   log_options();

#ifdef META_GUI
   window.start(plugins, &wave_iface);
#endif
}

MetaDSP::~MetaDSP()
{
   ConfigFile cfg("ssnes_effect.cfg");
   for (unsigned i = 0; i < max_plugs; i++)
   {
      cfg.set_string(Utils::join("meta_plugin", i), plugins[i]->path());
      cfg.set_bool(Utils::join("plugin_enabled", i), plugins[i]->enabled());
   }

   cfg.write("ssnes_effect.cfg");
}

void MetaDSP::log_options() const
{
   for (unsigned i = 0; i < max_plugs; i++)
   {
      std::cerr << "[MetaDSP]: Plugin #" << i << ": " << plugins[i]->ident() << std::endl;
      auto list = plugins[i]->options();

      for (auto itr = list.begin(); itr != list.end(); ++itr)
         std::cerr << "\tOption ID #" << itr->id << ": " << itr->description << std::endl; 
   }
}

void MetaDSP::show()
{
#ifdef META_GUI
   window.show();
#endif
}

void MetaDSP::events()
{
#ifdef META_GUI
   window.events();
#endif
}

// Process the chain.
void MetaDSP::process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
#ifdef META_GUI
   Global::lock();
#endif
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

#ifdef META_GUI
   wave_iface.push(out->samples, out->frames);
   Global::unlock();
#endif
}

