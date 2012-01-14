#include "main_window.moc.hpp"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <iostream>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QVariant>

ThreadWindowImpl::ThreadWindowImpl(std::shared_ptr<Plugin> *plugs,
      WaveTransferInterface *wave_iface,
      QWidget *parent)
   : QWidget(parent), plugins(plugs)
{
   setAttribute(Qt::WA_QuitOnClose, false);
   QVBoxLayout *vbox = new QVBoxLayout;
   vbox->addWidget(new QLabel("DSP plugin config:", this));
   
   tab = new QTabWidget(this);
   for (unsigned i = 0; i < MetaDSP::max_plugs; i++)
      tab->addTab(new PluginSettings(plugins[i], tab), QString::fromUtf8(plugins[i]->ident().c_str()));

   vbox->addWidget(tab);
   vbox->addSpacing(10);

   vbox->addWidget(new QLabel("WAV recording:", this));
   WaveRecorder *recorder = new WaveRecorder(this);
   connect(wave_iface, SIGNAL(data(const float*, size_t)), recorder, SLOT(data(const float*, size_t)));
   vbox->addWidget(recorder);

   setWindowTitle("SSNES Meta DSP");
   setLayout(vbox);
}

void MetaApplication::show()
{
   impl->show();
}

MetaApplication::MetaApplication(std::shared_ptr<Plugin> *plugs,
      WaveTransferInterface *wave_iface)
   : plugins(plugs)
{
   static int argc = 1;
   static const char *appname = "ssnes-dsp";
   static char *argv[] = { const_cast<char*>(appname), NULL };

   app = new QApplication(argc, argv);
   impl = new ThreadWindowImpl(plugins, wave_iface);
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
   path->setText(QString::fromUtf8(plugin->path().c_str()));
   hbox->addWidget(path);

   QPushButton *open_btn = new QPushButton("Open", this);
   QPushButton *remove_btn = new QPushButton("Remove", this);
   hbox->addWidget(open_btn);
   hbox->addWidget(remove_btn);
   connect(open_btn, SIGNAL(clicked()), this, SLOT(open()));
   connect(remove_btn, SIGNAL(clicked()), this, SLOT(remove()));
   vbox->addLayout(hbox);

   options = new QBoxLayout(QBoxLayout::TopToBottom);
   update_controls();
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
      plugin = std::make_shared<Plugin>(&Global::get_dsp_info(), name.toUtf8().constData());
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
   options->setDirection(plugin->layout() == AbstractPlugin::Layout::Vertical ?
         QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);

   if (tab_widget)
      tab_widget->setTabText(tab_widget->indexOf(this), QString::fromUtf8(plugin->ident().c_str()));

   foreach (QWidget *widget, widgets)
   {
      options->removeWidget(widget);
      delete widget;
   }
   widgets.clear();

   auto list = plugin->options();
   for (auto itr = list.begin(); itr != list.end(); ++itr)
   {
      Qt::Orientation orient = plugin->layout() == AbstractPlugin::Layout::Vertical ?
         Qt::Horizontal : Qt::Vertical;

      QWidget *widget;
      switch (itr->type)
      {
         case PluginOption::Type::Double:
            widget = new PluginSettingDouble(plugin, *itr, orient, this);
            break;

         case PluginOption::Type::Integer:
            widget = new PluginSettingInteger(plugin, *itr, orient, this);
            break;

         case PluginOption::Type::Selection:
            widget = new PluginSettingSelection(plugin, *itr, orient, this);
            break;

         default:
            widget = 0;
      }

      options->addWidget(widget);
      widgets.append(widget);
   }
}

PluginSettingDouble::PluginSettingDouble(std::shared_ptr<Plugin> &plug,
      const PluginOption &opt,
      Qt::Orientation orient,
      QWidget *parent)
   : QWidget(parent), plugin(plug)
{
   id = opt.id;
   min = opt.d.min;
   max = opt.d.max;
   current = opt.d.current;

   QBoxLayout *box;
   if (orient == Qt::Vertical)
      box = new QVBoxLayout;
   else
      box = new QHBoxLayout;

   box->addWidget(new QLabel(QString::fromUtf8(opt.description.c_str()), this));
   slider = new QSlider(orient, this);
   slider->setMinimum(0);
   slider->setMaximum(Intervals);
   connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updated()));
   box->addWidget(slider);

   value = new QLabel(QString::number(current, 'f', 1));
   box->addWidget(value);

   val2slide(current);
   setLayout(box);
}

void PluginSettingDouble::updated()
{
   current = slide2val();
   value->setText(QString::number(current, 'f', 1));

   Global::lock();
   plugin->set_option_double(id, current);
   Global::unlock();
}

double PluginSettingDouble::slide2val() const
{
   int pos = slider->value();
   return min + (max - min) * pos / Intervals;
}

void PluginSettingDouble::val2slide(double val)
{
   int value = (val - min) * Intervals / (max - min);
   slider->setValue(value);
}

PluginSettingInteger::PluginSettingInteger(std::shared_ptr<Plugin> &plug,
      const PluginOption &opt,
      Qt::Orientation orient,
      QWidget *parent)
   : QWidget(parent), plugin(plug)
{
   id = opt.id;

   QBoxLayout *box;
   if (orient == Qt::Vertical)
      box = new QVBoxLayout;
   else
      box = new QHBoxLayout;

   box->addWidget(new QLabel(QString::fromUtf8(opt.description.c_str()), this));

   QSlider *slider = new QSlider(orient, this);
   slider->setMinimum(opt.i.min);
   slider->setMaximum(opt.i.max);
   slider->setValue(opt.i.current);
   connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updated(int)));

   QLabel *value = new QLabel(this);
   value->setNum(opt.i.current);
   connect(slider, SIGNAL(valueChanged(int)), value, SLOT(setNum(int)));

   box->addWidget(slider);
   box->addWidget(value);

   setLayout(box);
}

void PluginSettingInteger::updated(int value)
{
   Global::lock();
   plugin->set_option_int(id, value);
   Global::unlock();
}

PluginSettingSelection::PluginSettingSelection(std::shared_ptr<Plugin> &plug,
      const PluginOption &opt, 
      Qt::Orientation orient,
      QWidget *parent)
   : QWidget(parent), plugin(plug)
{
   id = opt.id;

   QBoxLayout *box;
   if (orient == Qt::Vertical)
      box = new QVBoxLayout;
   else
      box = new QHBoxLayout;

   box->addWidget(new QLabel(QString::fromUtf8(opt.description.c_str()), this));

   combo = new QComboBox(this);
   
   for (auto itr = opt.s.selection.begin(); itr != opt.s.selection.end(); ++itr)
      combo->addItem(QString::fromUtf8(itr->description.c_str()), QVariant::fromValue(itr->id));

   int index = combo->findData(QVariant::fromValue(opt.s.current));
   if (index >= 0)
      combo->setCurrentIndex(index);

   connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));
   box->addWidget(combo);

   setLayout(box);
}

void PluginSettingSelection::indexChanged(int index)
{
   PluginOption::ID optid = combo->itemData(index).value<PluginOption::ID>();
   Global::lock();
   plugin->set_option_selection(id, optid);
   Global::unlock();
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
