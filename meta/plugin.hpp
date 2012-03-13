#ifndef __PLUGIN_HPP
#define __PLUGIN_HPP

#include "../ssnes_dsp.h"
#include "library.hpp"
#include <stddef.h>
#include <string>
#include <list>
#include "../abstract_plugin.hpp"

namespace Global
{
   void set_dsp_info(const ssnes_dsp_info_t &info);
   const ssnes_dsp_info_t& get_dsp_info();
}

class Plugin
{
   public:
      Plugin(const ssnes_dsp_info_t *info, const char *lib = nullptr);
      Plugin();
      ~Plugin();

      void operator=(Plugin&) = delete;
      Plugin(const Plugin&) = delete;

      void process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in);
      void show();
      std::string ident() const;
      const std::string& path() const;

      void enabled(bool enable);
      bool enabled() const;

      bool is_resampler() const;
      AbstractPlugin::Layout layout() const;
      const std::list<PluginOption>& options() const;

      void set_option_double(PluginOption::ID id, double value);
      void set_option_int(PluginOption::ID id, int value);
      void set_option_selection(PluginOption::ID id, PluginOption::ID sel);

   private:
      Library library;
      const ssnes_dsp_plugin_t *plug;
      void *plug_handle;

      typedef const ssnes_dsp_plugin_t* (*plug_init_t)(void);

      bool is_enabled;
      std::string plug_path;
};

#endif
