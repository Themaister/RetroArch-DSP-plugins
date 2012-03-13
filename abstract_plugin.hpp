#ifndef __ABSTRACT_PLUGIN_HPP
#define __ABSTRACT_PLUGIN_HPP

#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include "inireader.h"

struct PluginOption
{
   typedef unsigned ID;
   ID id;
   std::string description;
   std::string conf_name;

   enum class Type
   {
      Double,
      Integer,
      Selection
   } type;

   struct Selection
   {
      Selection(ID id_, const std::string &str)
         : id(id_), description(str) {}
      ID id;
      std::string description;
   };
  
   union
   {
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

   };

   struct
   {
      std::list<Selection> selection;
      ID current;
   } s;
};

template <class T>
static auto find_id(T& t, PluginOption::ID id) -> decltype(begin(t))
{
   return std::find_if(t.begin(), t.end(),
         [id](const PluginOption &opt) {
            return opt.id == id;
         });
}

// Every plugin used by meta-plugin must inherit from this!
// Failing to do so will crash stuff hard! :D
class AbstractPlugin
{
   public:
      enum class Layout
      {
         Vertical,
         Horizontal
      };

      AbstractPlugin() : plug_layout(Layout::Vertical) {}

      virtual bool is_resampler() const { return false; }
      virtual Layout layout() const { return plug_layout; }
      virtual const std::list<PluginOption>& options() { return dsp_options; }

      void set_option(PluginOption::ID id, double val)
      {
         auto elem = find_id(dsp_options, id);
         if (elem != dsp_options.end())
            elem->d.current = val;

         set_option_double(id, val);
      }

      void set_option(PluginOption::ID id, int val)
      {
         auto elem = find_id(dsp_options, id);
         if (elem != dsp_options.end())
            elem->i.current = val;

         set_option_int(id, val);
      }

      void set_option(PluginOption::ID id, PluginOption::ID selection)
      {
         auto elem = find_id(dsp_options, id);
         if (elem != dsp_options.end())
            elem->s.current = selection;

         set_option_selection(id, selection);
      }

      virtual ~AbstractPlugin() {}

   protected:
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

      void load_options(const std::string &conf_path)
      {
         ConfigFile cfg(conf_path);
         for (auto &opt : dsp_options)
         {
            switch (opt.type)
            {
               case PluginOption::Type::Integer:
                  set_option(opt.id, cfg.get_int(opt.conf_name, opt.i.current));
                  break;

               case PluginOption::Type::Double:
                  set_option(opt.id, cfg.get_double(opt.conf_name, opt.d.current));
                  break;

               case PluginOption::Type::Selection:
                  set_option(opt.id,
                        static_cast<PluginOption::ID>(cfg.get_int(opt.conf_name,
                              opt.s.current)));
                  break;

               default:
                  break;
            }
         }
      }

      void save_options(const std::string &conf_path)
      {
         ConfigFile cfg(conf_path);
         for (auto &opt : dsp_options)
         {
            switch (opt.type)
            {
               case PluginOption::Type::Integer:
                  cfg.set_int(opt.conf_name, opt.i.current);
                  break;

               case PluginOption::Type::Double:
                  cfg.set_double(opt.conf_name, opt.d.current);
                  break;

               case PluginOption::Type::Selection:
                  cfg.set_int(opt.conf_name, opt.s.current);
                  break;

               default:
                  break;
            }
         }

         cfg.write(conf_path);
      }

      std::list<PluginOption> dsp_options;
      Layout plug_layout;
};

#endif

