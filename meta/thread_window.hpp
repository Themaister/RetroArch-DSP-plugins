#ifndef THREAD_WINDOW_HPP__
#define THREAD_WINDOW_HPP__

#include <phoenix.hpp>
#include <iostream>
#include <signal.h>
#include <thread>

using namespace phoenix;

class ThreadWindowImpl : public Window
{
   public:
      ThreadWindowImpl() : should_show(false), should_quit(false)
      {
         setTitle("SSNES Meta DSP");
         onClose = [this]() { this->setVisible(false); };

         btn.setText("Hai!");
         btn.onTick = []() { std::cerr << "Foo!" << std::endl; };

         vbox.setMargin(5);
         vbox.append(btn, 0, 0, 0);
         setGeometry(vbox.minimumGeometry());

         append(vbox);

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

      void quit() { should_quit = true; }
      void show() { should_show = true; }

   private:
      Window win;
      Button btn;
      Timer timer;

      volatile sig_atomic_t should_show;
      volatile sig_atomic_t should_quit;

      VerticalLayout vbox;
};

class ThreadWindow
{
   public:
      ThreadWindow()
      {
         thread = std::thread(&ThreadWindow::gui_thread, this);
      }

      void gui_thread()
      {
         // Window needs to be created in the thread it's supposed to run in, so allocated dynamically :)
         impl = new ThreadWindowImpl;
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

