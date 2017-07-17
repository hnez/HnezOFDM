/* -*- c++ -*- */
/*
 * Copyright 2017 Leonard Göhrs <leonard@goehrs.eu>.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "ho_add_cyclicprefix_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_add_cyclicprefix::sptr
    ho_add_cyclicprefix::make(int fft_len, int cp_len)
    {
      return gnuradio::get_initial_sptr
        (new ho_add_cyclicprefix_impl(fft_len, cp_len));
    }

    /*
     * The private constructor
     */
    ho_add_cyclicprefix_impl::ho_add_cyclicprefix_impl(int fft_len, int cp_len)
      : gr::sync_block("ho_add_cyclicprefix",
                       gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len),
                       gr::io_signature::make(1, 1, sizeof(gr_complex) * (fft_len + cp_len)))
    {
      this->fft_len= fft_len;
      this->cp_len= cp_len;
    }

    /*
     * Our virtual destructor.
     */
    ho_add_cyclicprefix_impl::~ho_add_cyclicprefix_impl()
    {
    }

    int
    ho_add_cyclicprefix_impl::work(int noutput_items,
                                   gr_vector_const_void_star &input_items,
                                   gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for(int chunk_idx=0; chunk_idx < noutput_items; chunk_idx++) {
        const gr_complex *chunk_in= &in[chunk_idx * fft_len];
        gr_complex *chunk_out= &out[chunk_idx * (fft_len + cp_len)];

        // Cyclic prefix
        memcpy(&chunk_out[0],
               &chunk_in[fft_len - cp_len],
               sizeof(gr_complex) * cp_len);

        // Payload
        memcpy(&chunk_out[cp_len],
               &chunk_in[0],
               sizeof(gr_complex) * fft_len);
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
