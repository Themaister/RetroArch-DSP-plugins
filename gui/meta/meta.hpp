#ifndef __META_HPP
#define __META_HPP

// A meta plugin that chains other DSPs ... Pretty much
#include "../../ssnes_dsp.h"

#include <phoenix.hpp>
#include <memory>
#include <vector>

class MetaDSP
{
   public:
      MetaDSP(float input_rate);
      void show();
      // One process that does it all.
      void process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in);

   private:
      ConfigWindow window;
      std::vector<std::shared_ptr<Plugin>> plugins;
};

#endif
