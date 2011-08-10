#ifndef __ABSTRACT_PLUGIN_HPP
#define __ABSTRACT_PLUGIN_HPP

#include <string>
#include <list>

struct PluginOption
{
   unsigned id;
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
      virtual void set_option(const PluginOption&, double) {}
      virtual ~AbstractPlugin() {}
   protected:
      std::list<PluginOption> dsp_options;
};

#endif

