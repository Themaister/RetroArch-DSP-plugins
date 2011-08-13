#include "plugin.hpp"

Plugin::Plugin(const ssnes_dsp_info_t *info, const char *lib) : 
   plug(NULL), plug_handle(NULL), is_enabled(true)
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

Plugin::Plugin() : plug(NULL), plug_handle(NULL), is_enabled(true)
{}

void Plugin::process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in)
{
   if (plug_handle && is_enabled)
      plug->process(plug_handle, out, in);
   else // Passthrough
   {
      out->samples = in->samples;
      out->frames = in->frames;
      out->should_resample = SSNES_TRUE;
   }
}

void Plugin::enabled(bool enable)
{
   is_enabled = enable;
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
      return "None";
}

Plugin::~Plugin()
{
   if (plug_handle)
      plug->free(plug_handle);
}

bool Plugin::is_resampler() const
{
   if (plug_handle)
      return reinterpret_cast<AbstractPlugin*>(plug_handle)->is_resampler();
   else
      return false;
}

const std::list<PluginOption>& Plugin::options() const
{
   if (plug_handle)
      return reinterpret_cast<AbstractPlugin*>(plug_handle)->options();
   else
   {
      static std::list<PluginOption> static_opts;
      return static_opts;
   }
}

void Plugin::set_option(PluginOption::ID id, double value)
{
   if (plug_handle)
      reinterpret_cast<AbstractPlugin*>(plug_handle)->set_option(id, value);
}

