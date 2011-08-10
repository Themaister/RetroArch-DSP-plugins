#ifndef THREAD_WINDOW_HPP__
#define THREAD_WINDOW_HPP__

#include <iostream>
#include <signal.h>
#include <thread>
#include "meta.hpp"

#include "meta_layout.hpp"
#include <phoenix.hpp>
using namespace phoenix;

class ThreadWindowImpl : public Window
{
   public:
      ThreadWindowImpl() : should_show(false), should_quit(false)
      {
         setTitle("SSNES Meta DSP");
         onClose = [this]() { this->setVisible(false); };

         timer.onTimeout = [this]() {
            if (should_quit)
               OS::quit();
            else if (should_show)
            {
               this->setVisible();
               should_show = false;
            }
         };

         timer.setInterval(50);
         timer.setEnabled(true);
      }

      void set_plugin_state(std::shared_ptr<Plugin> *plugs)
      { 
         layout.set_plugin_state(plugins = plugs);
         append(layout.layout());
         setGeometry(layout.layout().minimumGeometry());
      }

      void quit() { should_quit = true; }
      void show() { should_show = true; }

   private:
      Window win;
      Timer timer;

      volatile sig_atomic_t should_show;
      volatile sig_atomic_t should_quit;

      std::shared_ptr<Plugin> *plugins;
      MetaThreadLayout layout;
};

class ThreadWindow
{
   public:
      void start(std::shared_ptr<Plugin> *plugs)
      {
         thread = std::thread(&ThreadWindow::gui_thread, this, plugs);
      }

      void gui_thread(std::shared_ptr<Plugin> *plugs)
      {
         // Window needs to be created in the thread it's supposed to run in, so allocated dynamically :)
         impl = new ThreadWindowImpl;
         impl->set_plugin_state(plugs);
         OS::main();
         delete impl;
      }

      void show()
      {
         impl->show();
      }

      ~ThreadWindow()
      {
         impl->quit();
         thread.join();
      }

   private:
      ThreadWindowImpl *impl;
      std::thread thread;
};

#endif

