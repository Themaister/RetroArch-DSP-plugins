#include "main_window.moc.hpp"

#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

ThreadWindowImpl::ThreadWindowImpl(std::shared_ptr<Plugin> *plugs, QWidget *parent)
   : QWidget(parent), plugins(plugs)
{
   setAttribute(Qt::WA_QuitOnClose, false);
   QVBoxLayout *vbox = new QVBoxLayout;
   vbox->addWidget(new QLabel("DSP plugin config:", this));
   
   tab = new QTabWidget(this);
   for (unsigned i = 0; i < MetaDSP::max_plugs; i++)
      tab->addTab(new PluginSettings(plugins[i], tab), QString::fromStdString(plugins[i]->ident()));

   vbox->addWidget(tab);

   setWindowTitle("SSNES Meta DSP");
   setLayout(vbox);
}

void MetaApplication::show()
{
   impl->show();
}

MetaApplication::MetaApplication(std::shared_ptr<Plugin> *plugs)
   : plugins(plugs)
{
   static int argc = 1;
   static const char *appname = "ssnes-dsp";
   static char *argv[] = { const_cast<char*>(appname), NULL };

   app = new QApplication(argc, argv);
   impl = new ThreadWindowImpl(plugins);
}

MetaApplication::~MetaApplication()
{
   delete impl;
   delete app;
}

PluginSettings::PluginSettings(std::shared_ptr<Plugin> &plug, QWidget *parent)
   : QWidget(parent), plugin(plug)
{
   QVBoxLayout *vbox = new QVBoxLayout;

   auto list = plugin->options();
   for (auto itr = list.begin(); itr != list.end(); ++itr)
      vbox->addWidget(new PluginSetting(plugin, *itr, this));

   setLayout(vbox);
}

PluginSetting::PluginSetting(std::shared_ptr<Plugin> &plug,
      const PluginOption &opt, QWidget *parent)
   : QWidget(parent), plugin(plug)
{
   id = opt.id;
   min = opt.min;
   max = opt.max;
   current = opt.current;

   QHBoxLayout *hbox = new QHBoxLayout;
   hbox->addWidget(new QLabel(QString::fromStdString(opt.description), this));
   slider = new QSlider(Qt::Horizontal, this);
   slider->setMinimum(0);
   slider->setMaximum(Intervals);
   connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updated()));
   hbox->addWidget(slider);

   value = new QLabel(QString::number(current, 'f', 1));
   hbox->addWidget(value);

   val2slide(current);
   setLayout(hbox);
}

void PluginSetting::updated()
{
   current = slide2val();
   value->setText(QString::number(current, 'f', 1));

   Global::lock();
   plugin->set_option(id, current);
   Global::unlock();
}

double PluginSetting::slide2val() const
{
   int pos = slider->value();
   return min + (max - min) * pos / Intervals;
}

void PluginSetting::val2slide(double val)
{
   int value = (val - min) * Intervals / (max - min);
   slider->setValue(value);
}


namespace Global
{
   static QMutex g_lock;

   void lock()
   {
      g_lock.lock();
   }

   void unlock()
   {
      g_lock.unlock();
   }
}
