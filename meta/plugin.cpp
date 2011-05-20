#include "plugin.hpp"

Plugin::Plugin(const ssnes_dsp_info_t *info, const char *lib) : 
   plug(NULL), plug_handle(NULL)
{
   if (!lib)
      return;

   library = Library(lib);
   if (!library)
      return;

   plug_init_t init = library.sym<plug_init_t>("ssnes_dsp_plugin_init");

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
}
