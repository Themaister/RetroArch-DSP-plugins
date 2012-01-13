#ifndef THREAD_WINDOW_HPP__
#define THREAD_WINDOW_HPP__

#include <iostream>
#include "meta.hpp"

#include "main_window.moc.hpp"

#include <QApplication>

class ThreadWindow
{
   public:
      void start(std::shared_ptr<Plugin> *plugs, WaveTransferInterface *wave_iface)
      {
         app = new MetaApplication(plugs, wave_iface);
      }

      void show()
      {
         app->show();
      }

      void events()
      {
         QApplication::processEvents();
      }

      ~ThreadWindow()
      {
         delete app;
      }

   private:
      MetaApplication *app;
};

#endif

