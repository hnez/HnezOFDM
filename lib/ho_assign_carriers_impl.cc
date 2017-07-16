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
#include "ho_assign_carriers_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_assign_carriers::sptr
    ho_assign_carriers::make(int num_carriers, int fft_len, const std::string& len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_assign_carriers_impl(num_carriers, fft_len, len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_assign_carriers_impl::ho_assign_carriers_impl(int num_carriers, int fft_len, const std::string& len_tag_key)
      : gr::tagged_stream_block("ho_assign_carriers",
                                gr::io_signature::make(1, 1, sizeof(gr_complex) * num_carriers),
                                gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len),
                                len_tag_key)
    {
      this->num_carriers= num_carriers;
      this->fft_len= fft_len;

      occupied_carriers= new bool[fft_len];
      memset(occupied_carriers, 0, fft_len);

      for(int i=0; i < num_carriers; i++) {
        occupied_carriers[(fft_len * i)/num_carriers]= true;
      }
    }

    /*
     * Our virtual destructor.
     */
    ho_assign_carriers_impl::~ho_assign_carriers_impl()
    {
      delete[] occupied_carriers;
    }

    int
    ho_assign_carriers_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0];
      return noutput_items ;
    }

    int
    ho_assign_carriers_impl::work (int noutput_items,
                                   gr_vector_int &ninput_items,
                                   gr_vector_const_void_star &input_items,
                                   gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      int in_count= ninput_items[0];

      for (int sym_num=0; sym_num < in_count; sym_num++) {
        const gr_complex *in_sym= &in[sym_num * num_carriers];
        gr_complex *out_sym= &out[sym_num * fft_len];

        for(int ci=0, fi=0; fi<fft_len; fi++) {
          if(occupied_carriers[fi]) {
            out_sym[fi]= in_sym[ci];
            ci++;
          }
          else {
            out_sym[fi]= 1;
          }
        }
      }

      // Tell runtime system how many output items we produced.
      return in_count;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
