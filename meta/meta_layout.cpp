#include "meta_layout.hpp"

MetaThreadLayout::MetaThreadLayout()
{
   vbox.setMargin(5);
   box.append("Plugin #0");
   box.append("Plugin #1");
   box.append("Plugin #2");
   box.append("Plugin #3");
   box.append("Plugin #4");
   box.append("Plugin #5");
   box.append("Plugin #6");
   box.append("Plugin #7");
   vbox.append(box, 0, 0);
}

void MetaThreadLayout::set_plugin_state(std::shared_ptr<Plugin> *plugs)
{
   plugins = plugs;
   for (unsigned i = 0; i < MetaDSP::max_plugs; i++)
   {
      auto obj = std::make_shared<MetaSettings>(plugins[i]);
      vbox.append(obj->layout());
      settings.append(obj);
   }
}

VerticalLayout& MetaThreadLayout::layout()
{
   return vbox;
}


MetaSettings::MetaSettings(std::shared_ptr<Plugin>& plug_) : plug(plug_)
{
   label.setText(plug->ident().c_str());
   std::cerr << "Setting ident: " << plug->ident().c_str() << std::endl;

   hbox.setMargin(5);
   hbox.append(label, 0, 0);
}

HorizontalLayout& MetaSettings::layout()
{
   return hbox;
}

