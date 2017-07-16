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
#include <gnuradio/digital/crc32.h>
#include "ho_add_header_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_add_header::sptr
    ho_add_header::make(const std::string& len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_add_header_impl(len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_add_header_impl::ho_add_header_impl(const std::string& len_tag_key)
      : gr::tagged_stream_block("ho_add_header",
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                len_tag_key)
    {
    }

    /*
     * Our virtual destructor.
     */
    ho_add_header_impl::~ho_add_header_impl()
    {
    }

    int
    ho_add_header_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0] + 8;
      return noutput_items ;
    }

    int
    ho_add_header_impl::work (int noutput_items,
                              gr_vector_int &ninput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
    {
      const uint8_t *in = (const uint8_t *) input_items[0];
      uint8_t *out = (uint8_t *) output_items[0];

      uint32_t payload_len= ninput_items[0];
      uint32_t crc= gr::digital::crc32(in, payload_len);

      out[0]= (payload_len >> 24) & 0xff;
      out[1]= (payload_len >> 16) & 0xff;
      out[2]= (payload_len >>  8) & 0xff;
      out[3]= (payload_len >>  0) & 0xff;

      out[4]= (crc >> 24) & 0xff;
      out[5]= (crc >> 16) & 0xff;
      out[6]= (crc >>  8) & 0xff;
      out[7]= (crc >>  0) & 0xff;

      memcpy(&out[8], in, payload_len);

      // Tell runtime system how many output items we produced.
      return (payload_len + 8);
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
