#include "main_window.moc.hpp"

#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>
#include <string.h>
#include <stdlib.h>

#include <iostream>

ThreadWindowImpl::ThreadWindowImpl(QWidget *parent)
   : QWidget(parent), plugins(NULL)
{
   setAttribute(Qt::WA_QuitOnClose, false);
   QVBoxLayout *vbox = new QVBoxLayout;
   vbox->addWidget(new QPushButton("Foo!", this));
   vbox->addWidget(new QPushButton("Foo2!", this));

   setWindowTitle("SSNES Meta DSP");
   setLayout(vbox);
}

void ThreadWindowImpl::plugs(std::shared_ptr<Plugin> *plugs_)
{
   plugins = plugs_;
}

void ThreadWindowImpl::update_plugs()
{}

void MetaApplication::show()
{
   if (alive)
   {
      std::cerr << "Trying to show!" << std::endl;
      QTimer::singleShot(0, impl, SLOT(show()));
   }
}

MetaApplication::MetaApplication(std::shared_ptr<Plugin> *plugs)
   : QThread(0), plugins(plugs), alive(false)
{
   start();
}

void MetaApplication::run()
{
   int argc = 1;
   char *appname = strdup("ssnes-dsp");
   char *argv[] = { appname, NULL };

   QApplication app(argc, argv);
   impl = new ThreadWindowImpl;
   alive = true;
   impl->plugs(plugins);
   impl->show();
   app.exec();
   delete impl;
   free(appname);
}

