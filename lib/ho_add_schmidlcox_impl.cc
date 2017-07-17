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
#include "ho_add_schmidlcox_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_add_schmidlcox::sptr
    ho_add_schmidlcox::make(int fft_len, const std::string& len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_add_schmidlcox_impl(fft_len, len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_add_schmidlcox_impl::ho_add_schmidlcox_impl(int fft_len, const std::string& len_tag_key)
      : gr::tagged_stream_block("ho_add_schmidlcox",
                                gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len),
                                gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len),
                                len_tag_key)
    {
      this->fft_len= fft_len;

      preamble_a= new gr_complex[fft_len];
      preamble_b= new gr_complex[fft_len];

      uint32_t lfsr_state= 1;

      for(int i=0; i<fft_len; i++) {
        preamble_a[i]= gr_complex(lfsr(&lfsr_state) ? M_SQRT2 : -M_SQRT2,
                                  lfsr(&lfsr_state) ? M_SQRT2 : -M_SQRT2);

        preamble_b[i]= gr_complex(lfsr(&lfsr_state) ? M_SQRT1_2 : -M_SQRT1_2,
                                  lfsr(&lfsr_state) ? M_SQRT1_2 : -M_SQRT1_2);

        /* In the first preamble symbol only every
           second carrier is occupied */
        if (i%2) preamble_a[i]= 0;

        printf("preamble[%d]= %f+%fj / %f+%fj\n",
               i,
               preamble_a[i].real(), preamble_a[i].imag(),
               preamble_b[i].real(), preamble_b[i].imag());
      }
    }

    /*
     * Our virtual destructor.
     */
    ho_add_schmidlcox_impl::~ho_add_schmidlcox_impl()
    {
      delete[] preamble_a;
      delete[] preamble_b;
    }

    int
    ho_add_schmidlcox_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0] + 2;
      return noutput_items ;
    }

    bool
    ho_add_schmidlcox_impl::lfsr (uint32_t *state)
    {
      uint_fast8_t bit= (*state) & 1;

      (*state)>>= 1;

      if(bit) (*state)^= 0x80000057;

      return bit;
    }

    int
    ho_add_schmidlcox_impl::work (int noutput_items,
                                  gr_vector_int &ninput_items,
                                  gr_vector_const_void_star &input_items,
                                  gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int in_count= ninput_items[0];
      int out_count= in_count + 2;

      printf("in_count: %d\n", in_count);
      printf("out_count: %d\n", in_count);

      if(out_count > noutput_items) {
        throw std::runtime_error("Output buffer to small!");
      }

      size_t chunk_size= sizeof(gr_complex) * fft_len;

      memcpy(out, preamble_a, chunk_size);
      memcpy(&out[fft_len], preamble_b, chunk_size);
      memcpy(&out[fft_len*2], in, chunk_size * in_count);

      // Tell runtime system how many output items we produced.
      return (out_count);
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
