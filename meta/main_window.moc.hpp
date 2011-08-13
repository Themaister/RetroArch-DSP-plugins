#include "meta.hpp"

#ifndef MAIN_WINDOW_HPP__
#define MAIN_WINDOW_HPP__

#include <QWidget>
#include <QThread>
#include <memory>

class ThreadWindowImpl : public QWidget
{
   Q_OBJECT

   public:
      ThreadWindowImpl(QWidget *parent = 0);
      void plugs(std::shared_ptr<Plugin> *plugs);

   public slots:
      void update_plugs();

   private:
      std::shared_ptr<Plugin> *plugins;
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
