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
#include "ho_hamming74_impl.h"

const uint8_t lut_decode[] = {
  0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0E, 0x07,
  0x00, 0x09, 0x0E, 0x0B, 0x0E, 0x0D, 0x0E, 0x0E,
  0x00, 0x03, 0x03, 0x03, 0x04, 0x0D, 0x06, 0x03,
  0x08, 0x0D, 0x0A, 0x03, 0x0D, 0x0D, 0x0E, 0x0D,
  0x00, 0x05, 0x02, 0x0B, 0x05, 0x05, 0x06, 0x05,
  0x08, 0x0B, 0x0B, 0x0B, 0x0C, 0x05, 0x0E, 0x0B,
  0x08, 0x01, 0x06, 0x03, 0x06, 0x05, 0x06, 0x06,
  0x08, 0x08, 0x08, 0x0B, 0x08, 0x0D, 0x06, 0x0F,
  0x00, 0x09, 0x02, 0x07, 0x04, 0x07, 0x07, 0x07,
  0x09, 0x09, 0x0A, 0x09, 0x0C, 0x09, 0x0E, 0x07,
  0x04, 0x01, 0x0A, 0x03, 0x04, 0x04, 0x04, 0x07,
  0x0A, 0x09, 0x0A, 0x0A, 0x04, 0x0D, 0x0A, 0x0F,
  0x02, 0x01, 0x02, 0x02, 0x0C, 0x05, 0x02, 0x07,
  0x0C, 0x09, 0x02, 0x0B, 0x0C, 0x0C, 0x0C, 0x0F,
  0x01, 0x01, 0x02, 0x01, 0x04, 0x01, 0x06, 0x0F,
  0x08, 0x01, 0x0A, 0x0F, 0x0C, 0x0F, 0x0F, 0x0F
};

const uint8_t lut_encode[] = {
  0x00, 0x71, 0x62, 0x13, 0x54, 0x25, 0x36, 0x47,
  0x38, 0x49, 0x5A, 0x2B, 0x6C, 0x1D, 0x0E, 0x7F
};

namespace gr {
  namespace hnez_ofdm {

    ho_hamming74::sptr
    ho_hamming74::make(bool encode, const std::string& len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_hamming74_impl(encode, len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_hamming74_impl::ho_hamming74_impl(bool encode, const std::string& len_tag_key)
      : gr::tagged_stream_block("ho_hamming74",
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                len_tag_key)
    {
      do_encode= encode;
    }

    /*
     * Our virtual destructor.
     */
    ho_hamming74_impl::~ho_hamming74_impl()
    {
    }

    int
    ho_hamming74_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0];
      return noutput_items ;
    }

    int
    ho_hamming74_impl::work (int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items)
    {
      const uint8_t *in = (uint8_t *) input_items[0];
      uint8_t *out = (uint8_t *) output_items[0];

      int in_count= ninput_items[0];

      if(do_encode) {
        for(int i=0; i<in_count; i++) {
          out[i]= lut_encode[in[i] & 0x0f];
        }
      }
      else {
        for(int i=0; i<in_count; i++) {
          out[i]= lut_decode[in[i] & 0x7f];
        }
      }

      // Tell runtime system how many output items we produced.
      return in_count;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
