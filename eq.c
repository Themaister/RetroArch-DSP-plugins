#include "eq.h"

#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define COEFF_SIZE 256
#define FILT_SIZE (COEFF_SIZE * 2)

static complex float phase_lut[2 * FILT_SIZE + 1];
static complex float * const phase_lut_ptr = phase_lut + FILT_SIZE;

static void generate_phase_lut(void)
{
   for (int i = -FILT_SIZE; i <= FILT_SIZE; i++)
   {
      float phase = (float)i / FILT_SIZE;
      phase_lut_ptr[i] = cexpf(M_PI * I * phase);
   }
}

static inline unsigned bitrange(unsigned len)
{
   unsigned ret = 0;
   while ((len >>= 1))
      ret++;

   return ret;
}

static inline unsigned bitswap(unsigned i, unsigned range)
{
   unsigned ret = 0;
   for (unsigned shifts = 0; shifts < range; shifts++)
      ret |= i & (1 << (range - shifts - 1)) ? (1 << shifts) : 0;

   return ret;
}

// When interleaving the butterfly buffer, addressing puts bits in reverse.
// [0, 1, 2, 3, 4, 5, 6, 7] => [0, 4, 2, 6, 1, 5, 3, 7] 
static void interleave(complex float *butterfly_buf, size_t samples)
{
   unsigned range = bitrange(samples);
   for (unsigned i = 0; i < samples; i++)
   {
      unsigned target = bitswap(i, range);
      if (target > i)
      {
         complex float tmp = butterfly_buf[target];
         butterfly_buf[target] = butterfly_buf[i];
         butterfly_buf[i] = tmp;
      }
   }
}

static void butterfly(complex float *a, complex float *b, complex float mod)
{
   mod *= *b;
   complex float a_ = *a + mod;
   complex float b_ = *a - mod;
   *a = a_;
   *b = b_;
}

static void butterflies(complex float *butterfly_buf, int phase_dir, size_t step_size, size_t samples)
{
   for (unsigned i = 0; i < samples; i += 2 * step_size)
   {
      int phase_step = FILT_SIZE * phase_dir / (int)step_size;
      for (unsigned j = i; j < i + step_size; j++)
         butterfly(&butterfly_buf[j], &butterfly_buf[j + step_size], phase_lut_ptr[phase_step * (int)(j - i)]);
   }
}

static void calculate_fft_butterfly(complex float *butterfly_buf, size_t samples)
{
   // Interleave buffer to work with FFT.
   interleave(butterfly_buf, samples);

   // Fly, lovely butterflies! :D
   for (unsigned step_size = 1; step_size < samples; step_size *= 2)
      butterflies(butterfly_buf, -1, step_size, samples);
}

static void calculate_fft(const float *data, complex float *butterfly_buf, size_t samples)
{
   for (unsigned i = 0; i < samples; i++)
      butterfly_buf[i] = data[i];

   // Interleave buffer to work with FFT.
   interleave(butterfly_buf, samples);

   // Fly, lovely butterflies! :D
   for (unsigned step_size = 1; step_size < samples; step_size *= 2)
      butterflies(butterfly_buf, -1, step_size, samples);
}

static void calculate_ifft(complex float *butterfly_buf, size_t samples)
{
   // Interleave buffer to work with FFT.
   interleave(butterfly_buf, samples);

   // Fly, lovely butterflies! In opposite direction! :D
   for (unsigned step_size = 1; step_size < samples; step_size *= 2)
      butterflies(butterfly_buf, 1, step_size, samples);

   float factor = 1.0 / samples;
   for (unsigned i = 0; i < samples; i++)
      butterfly_buf[i] *= factor;
}

struct eq_band
{
   float gain;
   unsigned min_bin;
   unsigned max_bin;
};

struct dsp_eq_state
{
   struct eq_band *bands;
   unsigned num_bands;

   complex float fft_coeffs[FILT_SIZE];
   float cosine_window[COEFF_SIZE];

   float last_buf[COEFF_SIZE];
   float stage_buf[FILT_SIZE];
   unsigned stage_ptr;
};

static void calculate_band_range(struct eq_band *band, float norm_freq)
{
   unsigned max_bin = (unsigned)round(norm_freq * COEFF_SIZE);

   band->gain    = 1.0;
   band->max_bin = max_bin;
}

static void recalculate_fft_filt(dsp_eq_state_t *eq)
{
   complex float freq_response[FILT_SIZE] = {0.0f};

   for (unsigned i = 0; i < eq->num_bands; i++)
   {
      for (unsigned j = eq->bands[i].min_bin; j <= eq->bands[i].max_bin; j++)
         freq_response[j] = eq->bands[i].gain;
   }

   memset(eq->fft_coeffs, 0, sizeof(eq->fft_coeffs));

   for (unsigned start = 1, end = COEFF_SIZE - 1; start < COEFF_SIZE / 2; start++, end--)
      freq_response[end] = freq_response[start];

   calculate_ifft(freq_response, COEFF_SIZE);

   // ifftshift(). Needs to be done for some reason ... TODO: Figure out why :D
   memcpy(eq->fft_coeffs + COEFF_SIZE / 2, freq_response +              0, COEFF_SIZE / 2 * sizeof(complex float));
   memcpy(eq->fft_coeffs +              0, freq_response + COEFF_SIZE / 2, COEFF_SIZE / 2 * sizeof(complex float));

   for (unsigned i = 0; i < COEFF_SIZE; i++)
      eq->fft_coeffs[i] *= eq->cosine_window[i];

   calculate_fft_butterfly(eq->fft_coeffs, FILT_SIZE);
}

dsp_eq_state_t *dsp_eq_new(float input_rate, const float *bands, unsigned num_bands)
{
   for (unsigned i = 1; i < num_bands; i++)
   {
      if (bands[i] <= bands[i - 1])
         return NULL;
   }

   if (num_bands < 2)
      return NULL;

   dsp_eq_state_t *state = calloc(1, sizeof(*state));
   if (!state)
      return NULL;

   state->num_bands = num_bands;

   state->bands = calloc(num_bands, sizeof(struct eq_band));
   if (!state->bands)
      goto error;

   calculate_band_range(&state->bands[0], ((bands[0] + bands[1]) / 2.0) / input_rate);
   state->bands[0].min_bin = 0;

   for (unsigned i = 1; i < num_bands - 1; i++)
   {
      calculate_band_range(&state->bands[i], ((bands[i + 1] + bands[i + 0]) / 2.0) / input_rate);
      state->bands[i].min_bin = state->bands[i - 1].max_bin + 1;

      if (state->bands[i].max_bin < state->bands[i].min_bin)
         fprintf(stderr, "[Equalizer]: Band @ %.2f Hz does not have enough spectral resolution to fit.\n", bands[i]);
   }

   state->bands[num_bands - 1].max_bin = COEFF_SIZE / 2;
   state->bands[num_bands - 1].min_bin = state->bands[num_bands - 2].max_bin + 1;
   state->bands[num_bands - 1].gain    = 1.0f;

   for (unsigned i = 0; i < COEFF_SIZE; i++)
      state->cosine_window[i] = cosf(M_PI * (i + 0.5 - COEFF_SIZE / 2) / COEFF_SIZE);

   generate_phase_lut();
   recalculate_fft_filt(state);

   return state;

error:
   dsp_eq_free(state);
   return NULL;
}

void dsp_eq_set_gain(dsp_eq_state_t *eq, unsigned band, float gain)
{
   assert(band < eq->num_bands);

   eq->bands[band].gain = gain;
   recalculate_fft_filt(eq);
}

size_t dsp_eq_process(dsp_eq_state_t *eq, float *output, size_t out_samples,
      const float *input, size_t in_samples, unsigned stride)
{
   size_t written = 0;
   while (in_samples)
   {
      size_t to_read = COEFF_SIZE - eq->stage_ptr;

      if (to_read > in_samples)
         to_read = in_samples;

      for (unsigned i = 0; i < to_read; i++, input += stride)
         eq->stage_buf[eq->stage_ptr + i] = *input;

      in_samples    -= to_read;
      eq->stage_ptr += to_read;

      if (eq->stage_ptr >= COEFF_SIZE)
      {
         if (out_samples < COEFF_SIZE)
            return written;

         complex float butterfly_buf[FILT_SIZE];
         calculate_fft(eq->stage_buf, butterfly_buf, FILT_SIZE);
         for (unsigned i = 0; i < FILT_SIZE; i++)
            butterfly_buf[i] *= eq->fft_coeffs[i];

         calculate_ifft(butterfly_buf, FILT_SIZE);

         for (unsigned i = 0; i < COEFF_SIZE; i++, output += stride, out_samples--, written++)
            *output = crealf(butterfly_buf[i]) + eq->last_buf[i];

         for (unsigned i = 0; i < COEFF_SIZE; i++)
            eq->last_buf[i] = crealf(butterfly_buf[i + COEFF_SIZE]);

         eq->stage_ptr = 0;
      }
   }

   return written;
}

void dsp_eq_free(dsp_eq_state_t *eq)
{
   if (eq)
   {
      if (eq->bands)
         free(eq->bands);
      free(eq);
   }
}

