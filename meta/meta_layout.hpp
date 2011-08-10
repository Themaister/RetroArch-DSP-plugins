#include "meta.hpp"

#ifndef META_LAYOUT_HPP__
#define META_LAYOUT_HPP__

#include <phoenix.hpp>
using namespace phoenix;
using namespace nall;

class MetaSettings
{
   public:
      MetaSettings(std::shared_ptr<Plugin>& plug_);
      HorizontalLayout& layout();

   private:
      std::shared_ptr<Plugin>& plug;
      Label label;
      HorizontalLayout hbox;
};

class MetaThreadLayout
{
   public:
      MetaThreadLayout();
      void set_plugin_state(std::shared_ptr<Plugin> *plugs);
      VerticalLayout& layout();

   private:
      std::shared_ptr<Plugin> *plugins;
      VerticalLayout vbox;

      ComboBox box;

      linear_vector<std::shared_ptr<MetaSettings>> settings;
};

#endif

