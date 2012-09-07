#include "../rarch_dsp.h"
#include "meta.hpp"

static void dsp_free(void *handle)
{
   delete reinterpret_cast<MetaDSP*>(handle);
}

static void dsp_config(void *handle)
{
   MetaDSP *dsp = reinterpret_cast<MetaDSP*>(handle);
   dsp->show();
}

static void dsp_process(void *handle, rarch_dsp_output_t *out, const rarch_dsp_input_t *in)
{
   MetaDSP *dsp = reinterpret_cast<MetaDSP*>(handle);
   dsp->process(out, in);
}

static void *dsp_init(const rarch_dsp_info_t *info)
{
   return new MetaDSP(info->input_rate, info->output_rate);
}

static void dsp_events(void *handle)
{
   reinterpret_cast<MetaDSP*>(handle)->events();
}

const rarch_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   RARCH_DSP_API_VERSION,
   dsp_config,
   "DSP Chain",
   dsp_events,
};

RARCH_API_EXPORT const rarch_dsp_plugin_t * RARCH_API_CALLTYPE
   rarch_dsp_plugin_init(void)
{
   return &dsp_plug;
}
