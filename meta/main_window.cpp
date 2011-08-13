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
   impl->show();
}

MetaApplication::MetaApplication(std::shared_ptr<Plugin> *plugs)
   : plugins(plugs)
{
   static int argc = 1;
   static const char *appname = strdup("ssnes-dsp");
   static char *argv[] = { const_cast<char*>(appname), NULL };

   app = new QApplication(argc, argv);
   impl = new ThreadWindowImpl;
   impl->plugs(plugins);
}

MetaApplication::~MetaApplication()
{
   delete impl;
   delete app;
}

