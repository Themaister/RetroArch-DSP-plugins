#include "meta.hpp"

#ifndef MAIN_WINDOW_HPP__
#define MAIN_WINDOW_HPP__

#include <QWidget>
#include <QVector>
#include <QFile>
#include <QProcess>
#include <memory>

#ifdef META_THREADED
#include <QMutex>
#endif

#include <QTabWidget>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QComboBox>

namespace Global
{
   void lock();
   void unlock();
}

class PluginSettingDouble : public QWidget
{
   Q_OBJECT

   public:
      PluginSettingDouble(std::shared_ptr<Plugin> &plug, const PluginOption &opt,
            Qt::Orientation orient, QWidget *parent = 0);

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

class PluginSettingInteger : public QWidget
{
   Q_OBJECT

   public:
      PluginSettingInteger(std::shared_ptr<Plugin> &plug, const PluginOption &opt,
            Qt::Orientation orient, QWidget *parent = 0);

   private slots:
      void updated(int);

   private:
      PluginOption::ID id;
      std::shared_ptr<Plugin> &plugin;
};

class PluginSettingSelection : public QWidget
{
   Q_OBJECT

   public:
      PluginSettingSelection(std::shared_ptr<Plugin> &plug, const PluginOption &opt,
            Qt::Orientation orient, QWidget *parent = 0);

   private slots:
      void indexChanged(int);

   private:
      PluginOption::ID id;
      QComboBox *combo;
      std::shared_ptr<Plugin> &plugin;
};

class PluginSettings : public QWidget
{
   Q_OBJECT

   public:
      PluginSettings(std::shared_ptr<Plugin> &plug, QWidget *parent = 0);

   private slots:
      void enable(int);
      void open();
      void remove();
      
   private:
      std::shared_ptr<Plugin> &plugin;
      QLineEdit *path;

      QBoxLayout *options;
      QList<QWidget*> widgets;
      QTabWidget *tab_widget;

      void update_controls();
};

class WaveTransferInterface : public QObject
{
   Q_OBJECT

   public:
      WaveTransferInterface(QObject *parent = 0);

      void push(const float *samples, size_t frames);

   signals:
      void data(const float *samples, size_t frames);
};

class WaveRecorder : public QWidget
{
   Q_OBJECT

   public:
      WaveRecorder(QWidget *parent = 0);
      ~WaveRecorder();

   public slots:
      void data(const float *data, size_t frames);

   private slots:
      void find_file();
      void start();
      void stop();

   private:
      bool is_recording;
      bool is_encoding;
      QVector<int16_t> conv_buffer;
      QFile file;
      QProcess process;
      QLabel *progress;
      QLineEdit *path;
      QLineEdit *command;

      void flush_record();
      void update_size();
};

class ThreadWindowImpl : public QWidget
{
   Q_OBJECT

   public:
      ThreadWindowImpl(std::shared_ptr<Plugin> *plugs,
            WaveTransferInterface *wave_iface,
            QWidget *parent = 0);

   private:
      std::shared_ptr<Plugin> *plugins;
      QTabWidget *tab;
      friend class PluginSettings;
};

class MetaApplication
{
   public:
      MetaApplication(std::shared_ptr<Plugin> *plugs, WaveTransferInterface *wave_iface);
      ~MetaApplication();
      void show();

   private:
      ThreadWindowImpl *impl;
      std::shared_ptr<Plugin> *plugins;
      QApplication *app;
};

#endif
