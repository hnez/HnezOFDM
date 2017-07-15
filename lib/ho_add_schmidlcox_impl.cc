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

      for(int i=0; i<fft_len; i+=2) {
        uint8_t rnd= 104729 * (i+1);

        float real= (rnd & 0x01) ? M_SQRT2 : -M_SQRT2;
        float imag= (rnd & 0x02) ? M_SQRT2 : -M_SQRT2;

        preamble_a[i]= gr_complex(real, imag);
        preamble_a[i+1]= 0;
      }

      for(int i=0; i<fft_len; i++) {
        uint8_t rnd= 104729 * (i+1);

        float real= (rnd & 0x04) ? M_SQRT2 : -M_SQRT2;
        float imag= (rnd & 0x08) ? M_SQRT2 : -M_SQRT2;

        preamble_b[i]= gr_complex(real, imag);
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

    int
    ho_add_schmidlcox_impl::work (int noutput_items,
                                  gr_vector_int &ninput_items,
                                  gr_vector_const_void_star &input_items,
                                  gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      size_t chunk_size= fft_len * sizeof(gr_complex);

      memcpy(&out[0], preamble_a, chunk_size);
      memcpy(&out[chunk_size], preamble_b, chunk_size);
      memcpy(&out[chunk_size*2], in, chunk_size * ninput_items[0]);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
