#include "plugin.hpp"
#include <dlfcn.h>

Plugin::Plugin(const ssnes_dsp_info_t *info, const char *lib) : 
   lib_handle(NULL), plug(NULL), plug_handle(NULL)
{
   if (!lib)
      return;

   lib_handle = dlopen(lib, RTLD_LAZY);
   if (!lib_handle)
      return;

   plug_init_t init = reinterpret_cast<plug_init_t>(dlsym(lib_handle, "ssnes_dsp_plugin_init"));

   if (!init)
      return;

   plug = init();
   if (!plug)
      return;

   plug_handle = plug->init(info);
}

void Plugin::process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
   if (plug_handle)
      plug->process(plug_handle, out, in);
   else // Passthrough
   {
      out->samples = in->samples;
      out->frames = in->frames;
      out->should_resample = SSNES_TRUE;
   }
}

void Plugin::show()
{
   if (plug_handle)
      plug->config(plug_handle);
}

std::string Plugin::ident() const
{
   if (plug_handle)
      return plug->ident ? plug->ident : "Unknown";
   else
      return "Invalid";
}

Plugin::~Plugin()
{
   if (plug_handle)
      plug->free(plug_handle);

   if (lib_handle)
      dlclose(lib_handle);
}
