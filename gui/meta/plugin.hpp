#ifndef __PLUGIN_HPP
#define __PLUGIN_HPP

#include "../../ssnes_dsp.h"
#include <stddef.h>
#include <string>

class Plugin
{
   public:
      Plugin(const ssnes_dsp_info_t *info, const char *lib = NULL);
      ~Plugin();

      void operator=(Plugin&) = delete;
      Plugin(const Plugin&) = delete;

      void process(ssnes_dsp_output_t *out, const ssnes_dsp_input_t *in);
      void show();
      std::string ident() const;

   private:
      void *lib_handle;
      const ssnes_dsp_plugin_t *plug;
      void *plug_handle;

      typedef const ssnes_dsp_plugin_t* (*plug_init_t)(void);
};

#endif
