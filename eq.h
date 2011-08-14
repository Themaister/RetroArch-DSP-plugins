#ifndef DSP_EQ_H__
#define DSP_EQ_H__

typedef struct dsp_eq_state dsp_eq_state_t;

#ifdef __cplusplus
extern "C" {
#endif

dsp_eq_state_t *dsp_eq_new(float input_rate, const float *bands, unsigned num_bands);
void dsp_eq_set_gain(dsp_eq_state_t *eq, unsigned band, float gain);
float dsp_eq_process(dsp_eq_state_t *eq, float sample);
void dsp_eq_free(dsp_eq_state_t *eq);

#ifdef __cplusplus
}
#endif

#endif
