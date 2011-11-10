#ifndef		IIRFILTERS_H
#define		IIRFILTERS_H

#include <stddef.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif

/* filter types */
enum {
	LPF, /* low pass filter */
	HPF, /* High pass filter */
	BPCSGF,/* band pass filter 1 */
	BPZPGF,/* band pass filter 2 */
	APF, /* Allpass filter*/
	NOTCH, /* Notch Filter */
	RIAA_phono, /* RIAA record/tape deemphasis */
	PEQ, /* Peaking band EQ filter */
	BBOOST, /* Bassboost filter */
	LSH, /* Low shelf filter */
	HSH, /* High shelf filter */
	RIAA_CD /* CD de-emphasis */
};

class IIRFilter
{
private:                           
#ifdef __SSE2__
   __m128 fir_coeff[2];
   __m128 fir_buf[2];

   __m128 iir_coeff;
   __m128 iir_buf;
#endif

	float pf_freq, pf_qfact, pf_gain;
	int type, pf_q_is_bandwidth; 
	float xn1,xn2,yn1,yn2;
	float omega, cs, a1pha, beta, b0, b1, b2, a0, a1,a2, A, sn;

public:
	
	IIRFilter();
	~IIRFilter();
	float Process(float samp);

#ifdef __SSE2__
   void ProcessBatch(float *out, const float *in, unsigned frames);
#endif

	void setFrequency(float val);
	void setQuality(float val);
	void setGain(float val);
	void init(int samplerate,int filter_type);
	void make_poly_from_roots(double const * roots, size_t num_roots, float * poly);
};


#endif		//ECHO_H
