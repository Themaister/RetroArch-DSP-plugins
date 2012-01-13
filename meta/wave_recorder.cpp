#include "main_window.moc.hpp"
#include "utils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

// Will only work correctly on little-endian systems for now.

WaveRecorder::WaveRecorder(QWidget *parent) : QWidget(parent), is_recording(false)
{
   QVBoxLayout *vbox = new QVBoxLayout;

   QHBoxLayout *path_hbox = new QHBoxLayout;

   path = new QLineEdit(this);
   path->setReadOnly(true);

   path_hbox->addWidget(new QLabel("Path:", this));
   path_hbox->addWidget(path);

   QPushButton *path_btn = new QPushButton("Save ...", this);
   connect(path_btn, SIGNAL(clicked()), this, SLOT(find_file()));
   
   path_hbox->addWidget(path_btn);
   
   QHBoxLayout *control_hbox = new QHBoxLayout;

   QPushButton *start_btn = new QPushButton("Start", this);
   QPushButton *stop_btn = new QPushButton("Stop", this);

   connect(start_btn, SIGNAL(clicked()), this, SLOT(start()));
   connect(stop_btn, SIGNAL(clicked()), this, SLOT(stop()));

   control_hbox->addWidget(start_btn);
   control_hbox->addWidget(stop_btn);

   progress = new QLabel("Not recording ...", this);
   control_hbox->addWidget(progress);

   vbox->addLayout(path_hbox);
   vbox->addLayout(control_hbox);

   setLayout(vbox);
}

WaveRecorder::~WaveRecorder()
{
   stop();
   update_size();
}

void WaveRecorder::data(const float *data, size_t frames)
{
   if (is_recording)
   {
      if (conv_buffer.capacity() < frames * 2)
         conv_buffer.reserve(frames * 2);

      audio_convert_float_to_s16(conv_buffer.data(), data, frames * 2);
      file.write((const char*)conv_buffer.constData(), frames * 2 * sizeof(int16_t));
   }

   update_size();
}

void WaveRecorder::update_size()
{
   if (is_recording)
   {
      uint64_t seconds = (file.size() - 44) /
         (2 * sizeof(int16_t) * Global::get_dsp_info().output_rate);

      uint64_t minutes = seconds / 60;
      uint64_t hours = minutes / 60;
      
      QString str = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
      progress->setText(str);
   }
   else
      progress->setText("Not recording ...");
}

void WaveRecorder::start()
{
   if (is_recording)
      return;

   if (file.fileName().isEmpty())
   {
      QMessageBox::warning(this, "Recorder error!", "Please set path for WAV dump.");
      return;
   }

   if (!file.open(QIODevice::WriteOnly))
   {
      QString msg("Could not open file for writing: \"");
      msg += file.fileName();
      msg += "\"";

      QMessageBox::warning(this, "Recorder error!", msg);
      return;
   }

   uint32_t rate = Global::get_dsp_info().output_rate;
   uint32_t byte_rate = rate * 2 * sizeof(int16_t);

   const uint8_t wave_header[44] = {
      0x52, 0x49, 0x46, 0x46, // RIFF
      0, 0, 0, 0,             // ChunkSize, we'll know this at end.
      0x57, 0x41, 0x56, 0x45, // WAVE
      0x66, 0x6d, 0x74, 0x20, // fmt
      16, 0, 0, 0,            // SubChunk1Size
      1, 0,                   // PCM
      2, 0,                   // Stereo
      (uint8_t)(rate >> 0), (uint8_t)(rate >> 8), (uint8_t)(rate >> 16), (uint8_t)(rate >> 24), // Sample rate
      (uint8_t)(byte_rate >> 0), (uint8_t)(byte_rate >> 8), (uint8_t)(byte_rate >> 16), (uint8_t)(byte_rate >> 24),
      2 * sizeof(int16_t), 0,
      16, 0,                  // 16-bit
      0x64, 0x61, 0x74, 0x61, // data
      0, 0, 0, 0,             // SubChunk2Size, we'll know this at end.
   };

   file.write((const char*)wave_header, sizeof(wave_header));
   is_recording = true;
}

void WaveRecorder::stop()
{
   if (is_recording)
      flush_record();
}

void WaveRecorder::find_file()
{
   QString filename = QFileDialog::getSaveFileName(this,
         "Save File", file.fileName(), "Wave Files (*.wav)");

   if (!filename.isEmpty())
   {
      if (!filename.endsWith(".wav"))
         filename.append(".wav");

      file.setFileName(filename);
      path->setText(filename);
   }
}

void WaveRecorder::flush_record()
{
   size_t size = file.size();
   size_t size_sub8 = size - 8;
   size_t size_sub44 = size - 44;

   file.seek(4);
   file.putChar(size_sub8 >> 0);
   file.putChar(size_sub8 >> 8);
   file.putChar(size_sub8 >> 16);
   file.putChar(size_sub8 >> 24);

   file.seek(40);
   file.putChar(size_sub44 >> 0);
   file.putChar(size_sub44 >> 8);
   file.putChar(size_sub44 >> 16);
   file.putChar(size_sub44 >> 24);

   file.close();
   is_recording = false;
   update_size();
}

WaveTransferInterface::WaveTransferInterface(QObject *parent) : QObject(parent)
{}

void WaveTransferInterface::push(const float *samples, size_t frames)
{
   emit data(samples, frames);
}

