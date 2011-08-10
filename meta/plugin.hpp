#ifndef __PLUGIN_HPP
#define __PLUGIN_HPP

#include "../ssnes_dsp.h"
#include "library.hpp"
#include <stddef.h>
#include <string>
#include <list>
#include "../abstract_plugin.hpp"

class Plugin
{
   public:
      Plugin(const ssnes_dsp_info_t *info, const char *lib = NULL);
      Plugin();
      ~Plugin();

      void operator=(Plugin&) = delete;
      Plugin(const Plugin&) = delete;

      void process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in);
      void show();
      std::string ident() const;

      bool is_resampler() const;
      const std::list<PluginOption>& options() const;
      void set_option(PluginOption::ID id, double value);

   private:
      Library library;
      const ssnes_dsp_plugin_t *plug;
      void *plug_handle;

      typedef const ssnes_dsp_plugin_t* (*plug_init_t)(void);
};

#endif
