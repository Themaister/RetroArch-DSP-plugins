#include "../../ssnes_dsp.h"
#include "meta.hpp"

static void dsp_free(void *handle)
{
   MetaDSP *dsp = reinterpret_cast<MetaDSP*>(handle);
   delete dsp;
}

static void dsp_config(void *handle)
{
   MetaDSP *dsp = reinterpret_cast<MetaDSP*>(handle);
   dsp->show();
}

static void dsp_process(void *handle, ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
   MetaDSP *dsp = reinterpret_cast<MetaDSP*>(handle);
   dsp->process(out, in);
}

static void *dsp_init(const ssnes_dsp_info_t *info)
{
   return reinterpret_cast<void*>(new MetaDSP(info->input_rate));
}

const ssnes_dsp_plugin_t dsp_plug = {
   dsp_init,
   dsp_process,
   dsp_free,
   SSNES_DSP_API_VERSION,
   dsp_config,
   "DSP Chain"
};

SSNES_API_EXPORT const ssnes_dsp_plugin_t * SSNES_API_CALLTYPE
   ssnes_dsp_plugin_init(void)
{
   return &dsp_plug;
}
