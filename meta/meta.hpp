#ifndef __META_HPP
#define __META_HPP

// A meta plugin that chains other DSPs ... Pretty much
#include "../rarch_dsp.h"

#include "plugin.hpp"
#include <memory>
#include <vector>

#ifdef META_GUI
#include "thread_window.hpp"
#endif

class MetaDSP
{
   public:
      MetaDSP(float input_rate);
      ~MetaDSP();
      void show();
      void events();
      // One process that does it all.
      void process(rarch_dsp_output_t *out, const rarch_dsp_input_t *in);

      enum { max_plugs = 8 };

   private:
      std::shared_ptr<Plugin> plugins[max_plugs];
      std::shared_ptr<Plugin> resampler_plugin;
      float sample_rate;

      void log_options() const;

#ifdef META_GUI
      WaveTransferInterface wave_iface;
      ThreadWindow window;
#endif
};

#endif
