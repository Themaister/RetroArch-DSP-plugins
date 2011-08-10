#ifndef __ABSTRACT_PLUGIN_HPP
#define __ABSTRACT_PLUGIN_HPP

#include <string>
#include <list>
#include <iostream>

struct PluginOption
{
   typedef unsigned ID;

   ID id;
   std::string description;
   
   double min;
   double max;
   double current;
};

// Every plugin used by meta-plugin must inherit from this!
class AbstractPlugin
{
   public:
      virtual bool is_resampler() const { return false; };
      virtual const std::list<PluginOption>& options() { return dsp_options; }
      virtual void set_option(PluginOption::ID id, double val)
      {
         std::cerr << "[MetaDSP]: Unimplemented, setting option #" << id <<
            "to: " << val << std::endl;
      }

      virtual ~AbstractPlugin() {}

   protected:
      std::list<PluginOption> dsp_options;
};

#endif

