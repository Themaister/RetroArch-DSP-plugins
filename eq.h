#ifndef DSP_EQ_H__
#define DSP_EQ_H__

typedef struct dsp_eq_state dsp_eq_state_t;
#include <stddef.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

#define COEFF_SIZE 256
#define FILT_SIZE (COEFF_SIZE * 2)


#ifdef __cplusplus
extern "C" {
#endif

dsp_eq_state_t *dsp_eq_new(float input_rate, const float *bands, unsigned num_bands);
void dsp_eq_set_gain(dsp_eq_state_t *eq, unsigned band, float gain);
size_t dsp_eq_process(dsp_eq_state_t *eq, float *out, size_t out_samples,
      const float *sample, size_t in_samples, unsigned out_stride);
void dsp_eq_free(dsp_eq_state_t *eq);

#ifdef __cplusplus
}
#endif

#endif
