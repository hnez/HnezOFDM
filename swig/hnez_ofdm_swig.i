/* -*- c++ -*- */

#define HNEZ_OFDM_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "hnez_ofdm_swig_doc.i"

%{
#include "hnez_ofdm/ho_add_header.h"
#include "hnez_ofdm/ho_hamming74.h"
#include "hnez_ofdm/ho_interleave.h"
#include "hnez_ofdm/ho_qam4_multimod.h"
#include "hnez_ofdm/ho_assign_carriers.h"
#include "hnez_ofdm/ho_add_schmidlcox.h"
#include "hnez_ofdm/ho_add_cyclicprefix.h"
#include "hnez_ofdm/ho_schmidl_cox_tagger.h"
%}


%include "hnez_ofdm/ho_add_header.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_add_header);
%include "hnez_ofdm/ho_hamming74.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_hamming74);
%include "hnez_ofdm/ho_interleave.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_interleave);
%include "hnez_ofdm/ho_assign_carriers.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_assign_carriers);

%include "hnez_ofdm/ho_qam4_multimod.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_qam4_multimod);
%include "hnez_ofdm/ho_add_schmidlcox.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_add_schmidlcox);
%include "hnez_ofdm/ho_add_cyclicprefix.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_add_cyclicprefix);
%include "hnez_ofdm/ho_schmidl_cox_tagger.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_schmidl_cox_tagger);
