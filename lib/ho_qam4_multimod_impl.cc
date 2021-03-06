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
#include "ho_qam4_multimod_impl.h"

const gr_complex constellation[]= {
  gr_complex( M_SQRT1_2,  M_SQRT1_2),
  gr_complex( M_SQRT1_2, -M_SQRT1_2),
  gr_complex(-M_SQRT1_2,  M_SQRT1_2),
  gr_complex(-M_SQRT1_2, -M_SQRT1_2)
};

namespace gr {
  namespace hnez_ofdm {

    ho_qam4_multimod::sptr
    ho_qam4_multimod::make(int output_width, const std::string &len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_qam4_multimod_impl(output_width, len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_qam4_multimod_impl::ho_qam4_multimod_impl(int output_width, const std::string &len_tag_key)
      : gr::sync_decimator("ho_qam4_multimod",
                           gr::io_signature::make(1, 1, sizeof(uint8_t)),
                           gr::io_signature::make(1, 1, sizeof(gr_complex) * output_width),
                           output_width/4)
    {
      this->output_width= output_width;
      this->len_tag_key= pmt::intern(len_tag_key);

      // The packet_len tag has to be transformed manually
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    ho_qam4_multimod_impl::~ho_qam4_multimod_impl()
    {
    }

    int
    ho_qam4_multimod_impl::work(int noutput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items)
    {
      const uint8_t *in = (const uint8_t *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int decimation= output_width/4;
      int ninput_items= noutput_items * decimation;
      int out_idx= 0;

      for(int in_idx=0; in_idx < ninput_items; in_idx++) {
        uint8_t in_byte= in[in_idx];

        out[out_idx++]= constellation[(in_byte>>6) & 0x03];
        out[out_idx++]= constellation[(in_byte>>4) & 0x03];
        out[out_idx++]= constellation[(in_byte>>2) & 0x03];
        out[out_idx++]= constellation[(in_byte>>0) & 0x03];
      }

      // Manually propagate tags
      
      uint64_t absidx_in= nitems_read(0);
      uint64_t absidx_out= nitems_written(0);

      std::vector<tag_t> tags;
      get_tags_in_range(tags, 0, absidx_in, absidx_in + ninput_items);

      for(tag_t tag : tags) {
        tag.offset/= decimation;
                
        if(pmt::eqv(tag.key, len_tag_key)) {
          // Only the packet_len tag has to be mangled
          
          int pkg_len_orig= pmt::to_long(tag.value);
          int pkg_len_new= pkg_len_orig / decimation;
          tag.value= pmt::from_long(pkg_len_new);
        }

        add_item_tag(0, tag);
      }
      
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
