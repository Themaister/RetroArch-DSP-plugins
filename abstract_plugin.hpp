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

   enum class Type
   {
      Double,
      Integer,
      Selection
   } type;
   
   struct
   {
      double min;
      double max;
      double current;
   } d;

   struct
   {
      int min;
      int max;
      int current;
   } i;

   struct Selection
   {
      Selection(ID id_, const std::string &str)
         : id(id_), description(str) {}
      ID id;
      std::string description;
   };

   struct
   {
      std::list<Selection> selection;
      ID current;
   } s;
};

// Every plugin used by meta-plugin must inherit from this!
// Failing to do so will crash stuff hard! :D
class AbstractPlugin
{
   public:
      virtual bool is_resampler() const { return false; };
      virtual const std::list<PluginOption>& options() { return dsp_options; }
      virtual void set_option_double(PluginOption::ID id, double val)
      {
         std::cerr << "[MetaDSP]: Unimplemented, setting option (double) #" << id <<
            "to: " << val << std::endl;
      }

      virtual void set_option_int(PluginOption::ID id, int val)
      {
         std::cerr << "[MetaDSP]: Unimplemented, setting option (int) #" << id <<
            "to: " << val << std::endl;
      }

      virtual void set_option_selection(PluginOption::ID id, PluginOption::ID selection)
      {
         std::cerr << "[MetaDSP]: Unimplemented, setting selection (selection) #" << static_cast<int>(selection) << std::endl;
      }

      virtual ~AbstractPlugin() {}

   protected:
      std::list<PluginOption> dsp_options;
};

#endif

