#ifndef __META_HPP
#define __META_HPP

// A meta plugin that chains other DSPs ... Pretty much
#include "../ssnes_dsp.h"

#include "plugin.hpp"
#include <memory>
#include <vector>

#ifdef META_GUI
#include "thread_window.hpp"
#endif

class MetaDSP
{
   public:
      MetaDSP(float input_rate, float output_rate);
      void show();
      // One process that does it all.
      void process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in);

   private:
      enum { max_plugs = 8 };
      std::shared_ptr<Plugin> plugins[max_plugs];
      std::shared_ptr<Plugin> resampler_plugin;
      float sample_rate;

      void log_options() const;

#ifdef META_GUI
      ThreadWindow window;
#endif
};

#endif
