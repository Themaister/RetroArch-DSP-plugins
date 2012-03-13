#include "plugin.hpp"

namespace Global
{
   static ssnes_dsp_info_t info;

   void set_dsp_info(const ssnes_dsp_info_t &info_)
   {
      info = info_;
   }

   const ssnes_dsp_info_t& get_dsp_info()
   {
      return info;
   }
}

Plugin::Plugin(const ssnes_dsp_info_t *info, const char *lib) : 
   plug(nullptr), plug_handle(nullptr), is_enabled(true), plug_path(lib)
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

Plugin::Plugin() : plug(nullptr), plug_handle(nullptr), is_enabled(true)
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

bool Plugin::enabled() const
{
   return is_enabled;
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

const std::string& Plugin::path() const
{
   return plug_path;
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

void Plugin::set_option_double(PluginOption::ID id, double value)
{
   if (plug_handle)
      reinterpret_cast<AbstractPlugin*>(plug_handle)->set_option(id, value);
}

void Plugin::set_option_int(PluginOption::ID id, int value)
{
   if (plug_handle)
      reinterpret_cast<AbstractPlugin*>(plug_handle)->set_option(id, value);
}

void Plugin::set_option_selection(PluginOption::ID id, PluginOption::ID sel)
{
   if (plug_handle)
      reinterpret_cast<AbstractPlugin*>(plug_handle)->set_option(id, sel);
}

AbstractPlugin::Layout Plugin::layout() const
{
   if (plug_handle)
      reinterpret_cast<AbstractPlugin*>(plug_handle)->layout();
   else
      return AbstractPlugin::Layout::Vertical;
}
