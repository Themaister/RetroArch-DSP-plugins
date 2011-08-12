#ifndef THREAD_WINDOW_HPP__
#define THREAD_WINDOW_HPP__

#include <iostream>
#include "meta.hpp"

#include "main_window.moc.hpp"

#include <QApplication>

class ThreadWindow
{
   public:
      void start(std::shared_ptr<Plugin> *plugs)
      {
         app = new MetaApplication(plugs);
      }

      void show()
      {
         app->show();
      }

      ~ThreadWindow()
      {
         qApp->quit();
         app->exit();
         app->wait();
         delete app;
      }

   private:
      MetaApplication *app;
};

#endif

