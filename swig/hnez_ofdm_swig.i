/* -*- c++ -*- */

#define HNEZ_OFDM_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "hnez_ofdm_swig_doc.i"

%{
#include "hnez_ofdm/ho_add_header.h"
#include "hnez_ofdm/ho_hamming74.h"
//#include "hnez_ofdm/ho_interleave.h"
//#include "hnez_ofdm/ho_assign_carriers.h"
//#include "hnez_ofdm/ho_cyclicprefix.h"
//#include "hnez_ofdm/ho_pdu_to_stream_async.h"
%}


%include "hnez_ofdm/ho_add_header.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_add_header);
%include "hnez_ofdm/ho_hamming74.h"
GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_hamming74);
//%include "hnez_ofdm/ho_interleave.h"
//GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_interleave);
//%include "hnez_ofdm/ho_assign_carriers.h"
//GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_assign_carriers);
//%include "hnez_ofdm/ho_cyclicprefix.h"
//GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_cyclicprefix);
//%include "hnez_ofdm/ho_pdu_to_stream_async.h"
//GR_SWIG_BLOCK_MAGIC2(hnez_ofdm, ho_pdu_to_stream_async);
