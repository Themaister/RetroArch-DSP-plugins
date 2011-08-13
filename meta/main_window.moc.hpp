#include "meta.hpp"

#ifndef MAIN_WINDOW_HPP__
#define MAIN_WINDOW_HPP__

#include <QWidget>
#include <QThread>
#include <memory>
#include <QMutex>
#include <QTabWidget>
#include <QLabel>
#include <QSlider>

namespace Global
{
   void lock();
   void unlock();
}

class PluginSetting : public QWidget
{
   Q_OBJECT

   public:
      PluginSetting(std::shared_ptr<Plugin> &plug, const PluginOption &opt, QWidget *parent = 0);

   private slots:
      void updated();

   private:
      PluginOption::ID id;
      double min;
      double max;
      double current;
      std::shared_ptr<Plugin> &plugin;

      enum { Intervals = 1000 };
      double slide2val() const;
      void val2slide(double val);

      QLabel *value;
      QSlider *slider;
};

class PluginSettings : public QWidget
{
   Q_OBJECT

   public:
      PluginSettings(std::shared_ptr<Plugin> &plug, QWidget *parent = 0);
      
   private:
      std::shared_ptr<Plugin> &plugin;
};

class ThreadWindowImpl : public QWidget
{
   Q_OBJECT

   public:
      ThreadWindowImpl(std::shared_ptr<Plugin> *plugs, QWidget *parent = 0);

   private:
      std::shared_ptr<Plugin> *plugins;
      QTabWidget *tab;
};

class MetaApplication
{
   public:
      MetaApplication(std::shared_ptr<Plugin> *plugs);
      ~MetaApplication();
      void show();

   private:
      ThreadWindowImpl *impl;
      std::shared_ptr<Plugin> *plugins;
      QApplication *app;
};

#endif
