#include "main_window.moc.hpp"

#include <QPainter>
#include <complex>
#include <cmath>
#include <QCheckBox>
#include <QBrush>

PaintWidget::PaintWidget(unsigned min_width, unsigned min_height,
      const QVector<float> &res,
      QWidget *parent)
   : QWidget(parent), res(res)
{
   setMinimumSize(min_width, min_height);
}

void PaintWidget::paintEvent(QPaintEvent*)
{
   if (res.size() == 0)
      return;

   QPainter paint;
   paint.begin(this);
   QSize size = this->size();
   paint.setViewport(0, 0, size.width(), size.height());
   paint.setPen(Qt::green);
   QBrush brush(Qt::SolidPattern);
   brush.setColor(Qt::darkGreen);
   paint.setBrush(brush);

   float width = (float)size.width() / res.size();
   float x = 0.0f;

   QVector<QRectF> rects;
   foreach(float db, res)
   {
      float height = size.height() - 1 + 2.0 * db;
      if (height < 0.0f)
         height = 0.0f;

      rects.append(QRectF(x, size.height() - 1, width, -height));
      x += width;
   }

   paint.drawRects(rects);
}

SpectrumAnalyzer::SpectrumAnalyzer(QWidget *parent)
   : QWidget(parent), widget(new PaintWidget(256, 128, buffer_res)), enabled(false)
{
   QVBoxLayout *vbox = new QVBoxLayout;
   QCheckBox *en = new QCheckBox;
   en->setText("Enable");
   connect(en, SIGNAL(stateChanged(int)), this, SLOT(enable(int)));

   buffer_res.resize(bands);
   buffer_res.fill(-100.0f);

   vbox->addWidget(en);
   vbox->addWidget(widget);
   setLayout(vbox);
}

namespace FFT
{
   typedef std::complex<double> cdouble;

   static unsigned bitrange(unsigned len)
   {
      unsigned ret = 0;
      while ((len >>= 1))
         ret++;

      return ret;
   }

   static unsigned bitswap(unsigned i, unsigned range)
   {
      unsigned ret = 0;
      for (unsigned shifts = 0; shifts < range; shifts++)
         ret |= i & (1 << (range - shifts - 1)) ? (1 << shifts) : 0;

      return ret;
   }

   // When interleaving the butterfly buffer, addressing puts bits in reverse.
   // [0, 1, 2, 3, 4, 5, 6, 7] => [0, 4, 2, 6, 1, 5, 3, 7] 
   static void interleave(cdouble *butterfly_buf, size_t samples)
   {
      unsigned range = bitrange(samples);
      for (unsigned i = 0; i < samples; i++)
      {
         unsigned target = bitswap(i, range);
         if (target > i)
         {
            cdouble tmp = butterfly_buf[target];
            butterfly_buf[target] = butterfly_buf[i];
            butterfly_buf[i] = tmp;
         }
      }
   }

   static cdouble gen_phase(double index)
   {
      return std::exp(M_PI * cdouble(0, 1) * index);
   }

   static void butterfly(cdouble *a, cdouble *b, cdouble mod)
   {
      mod *= *b;
      cdouble a_ = *a + mod;
      cdouble b_ = *a - mod;
      *a = a_;
      *b = b_;
   }

   static void butterflies(cdouble *butterfly_buf, double phase_dir, size_t step_size, size_t samples)
   {
      for (unsigned i = 0; i < samples; i += 2 * step_size)
         for (unsigned j = i; j < i + step_size; j++)
            butterfly(&butterfly_buf[j], &butterfly_buf[j + step_size], gen_phase((phase_dir * (j - i)) / step_size));
   }

   static void calculate(const float *data, cdouble *butterfly_buf, size_t samples)
   {
      for (unsigned i = 0; i < samples; i++)
         butterfly_buf[i] = data[i];

      // Interleave buffer to work with FFT.
      interleave(butterfly_buf, samples);

      // Fly, lovely butterflies! :D
      for (unsigned step_size = 1; step_size < samples; step_size *= 2)
         butterflies(butterfly_buf, -1.0, step_size, samples);

      for (unsigned i = 1; i < samples / 2; i++)
         butterfly_buf[i] += std::conj(butterfly_buf[samples - i]);

      // Normalize amplitudes.
      for (unsigned i = 0; i < samples / 2; i++)
         butterfly_buf[i] /= samples;
   }

   static void db(float *data, const cdouble *butterfly_buf_l, const cdouble *butterfly_buf_r, size_t samples)
   {
      for (size_t i = 0; i < samples; i++)
         data[i] = 20.0f * std::log10(std::abs(butterfly_buf_l[i]) + std::abs(butterfly_buf_r[i]));
   }

   static void merge_bands(float *out, const float *in, unsigned freqs, unsigned bands)
   {
      unsigned ratio = freqs / bands;
      for (unsigned i = 0; i < bands; i++)
      {
         float sum = 0.0f;
         for (unsigned j = ratio * i; j < ratio * (i + 1); j++)
            sum += in[j];
         sum /= ratio;
         out[i] = sum;
      }
   }
}

void SpectrumAnalyzer::flush_fft()
{
   QVector<FFT::cdouble> buf_l;
   QVector<FFT::cdouble> buf_r;
   QVector<float> buf_res;
   buf_l.reserve(fft_size);
   buf_r.reserve(fft_size);
   buf_res.reserve(fft_freqs);

   FFT::calculate(buffer_l.data(), buf_l.data(), fft_size);
   FFT::calculate(buffer_r.data(), buf_r.data(), fft_size);
   FFT::db(buf_res.data(), buf_l.data(), buf_r.data(), fft_freqs);

   FFT::merge_bands(buffer_res.data(), buf_res.data(), fft_freqs, bands);

   widget->repaint();
}

void SpectrumAnalyzer::enable(int clicked)
{
   enabled = clicked;
}

void SpectrumAnalyzer::data(const float *data, size_t frames)
{
   if (!enabled)
      return;

   for (size_t i = 0; i < frames; i++)
   {
      buffer_l.push_back(data[2 * i + 0]);
      buffer_r.push_back(data[2 * i + 1]);
   }

   if (buffer_l.size() >= fft_size)
   {
      flush_fft();
      buffer_l.clear();
      buffer_r.clear();
   }
}

