#include "main_window.moc.hpp"

#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

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
   : QWidget(parent), plugin(plug), tab_widget(qobject_cast<QTabWidget*>(parent))
{
   QVBoxLayout *vbox = new QVBoxLayout;
   vbox->setAlignment(Qt::AlignTop);

   QCheckBox *box = new QCheckBox("Enable", this);
   box->setCheckState(Qt::Checked);
   connect(box, SIGNAL(stateChanged(int)), this, SLOT(enable(int)));
   vbox->addWidget(box);

   QHBoxLayout *hbox = new QHBoxLayout;
   path = new QLineEdit(this);
   path->setReadOnly(true);
   path->setText(QString::fromStdString(plugin->path()));
   hbox->addWidget(path);

   QPushButton *open_btn = new QPushButton("Open", this);
   QPushButton *remove_btn = new QPushButton("Remove", this);
   hbox->addWidget(open_btn);
   hbox->addWidget(remove_btn);
   connect(open_btn, SIGNAL(clicked()), this, SLOT(open()));
   connect(remove_btn, SIGNAL(clicked()), this, SLOT(remove()));
   vbox->addLayout(hbox);

   options = new QVBoxLayout;
   auto list = plugin->options();
   for (auto itr = list.begin(); itr != list.end(); ++itr)
   {
      QWidget *widget = new PluginSetting(plugin, *itr, this);
      widgets.append(widget);
      options->addWidget(widget);
   }

   vbox->addLayout(options);

   setLayout(vbox);
}

void PluginSettings::enable(int val)
{
   Global::lock();
   if (val == Qt::Checked)
      plugin->enabled(true);
   else if (val == Qt::Unchecked)
      plugin->enabled(false);
   Global::unlock();
}

void PluginSettings::open()
{
#if defined(_WIN32)
#define LIBRARY_EXTENSION "*.dll"
#elif defined(__APPLE__)
#define LIBRARY_EXTENSION "*.dylib"
#else
#define LIBRARY_EXTENSION "*.so"
#endif

   QFileInfo info(plugin->path().c_str());
   QDir dir = info.dir();
   QString dir_path = dir.path();

   // TODO: Try to find a way for this not to block :s
   QString name = QFileDialog::getOpenFileName(this, "Open DSP plugin ...",
         dir_path,
         "Dynamic library (" LIBRARY_EXTENSION ")");
   if (!name.isEmpty())
   {
      Global::lock();
      plugin = std::make_shared<Plugin>(&Global::get_dsp_info(), name.toStdString().c_str());
      path->setText(name); 
      Global::unlock();
      update_controls();
   }
}

void PluginSettings::remove()
{
   Global::lock();
   plugin = std::make_shared<Plugin>();
   path->clear();
   Global::unlock();
   update_controls();
}

void PluginSettings::update_controls()
{
   if (tab_widget)
      tab_widget->setTabText(tab_widget->indexOf(this), QString::fromStdString(plugin->ident()));

   foreach (QWidget *widget, widgets)
   {
      options->removeWidget(widget);
      delete widget;
   }
   widgets.clear();

   auto list = plugin->options();
   for (auto itr = list.begin(); itr != list.end(); ++itr)
   {
      QWidget *widget = new PluginSetting(plugin, *itr, this);
      options->addWidget(widget);
      widgets.append(widget);
   }
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
#ifdef META_THREADED
   static QMutex g_lock;

   void lock()
   {
      g_lock.lock();
   }

   void unlock()
   {
      g_lock.unlock();
   }
#else
   void lock() {}
   void unlock() {}
#endif
}
