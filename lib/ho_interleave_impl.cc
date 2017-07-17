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
#include "ho_interleave_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_interleave::sptr
    ho_interleave::make(int chunk_len, bool encode, const std::string& len_tag_key)
    {
      return gnuradio::get_initial_sptr
        (new ho_interleave_impl(chunk_len, encode, len_tag_key));
    }

    /*
     * The private constructor
     */
    ho_interleave_impl::ho_interleave_impl(int chunk_len, bool encode, const std::string& len_tag_key)
      : gr::tagged_stream_block("ho_interleave",
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                gr::io_signature::make(1, 1, sizeof(uint8_t)),
                                len_tag_key)
    {
      this->chunk_len= chunk_len;

      int chunk_len_bits= chunk_len * 8;

      int map[chunk_len_bits];

      // Initialize the map with a 1:1 mapping
      for(size_t i=0; i<chunk_len_bits; i++) {
        map[i]= i;
      }

      uint32_t lfsr_state= 1;

      // Scramble the 1:1 mapping
      for(int it=0; it<32; it++) {
        for(size_t a=0; a<chunk_len_bits; a++) {
          // I have no idea what i am doing here
          size_t b= lfsr(&lfsr_state) % chunk_len_bits;

          int tmp= map[b];
          map[b]= map[a];
          map[a]= tmp;
        }
      }


      /* Store the forward or backward map in
         permute_map */
      permute_map= new int[chunk_len_bits];

      for(size_t i=0; i<chunk_len_bits; i++) {
        if(encode) {
          permute_map[i]= map[i];
        }
        else {
          permute_map[map[i]]= i;
        }
      }
    }

    /*
     * Our virtual destructor.
     */
    ho_interleave_impl::~ho_interleave_impl()
    {
      delete[] permute_map;
    }

    int
    ho_interleave_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      /* There has to be a more elegant way to calculate this
         but i can not think of it right now */
      int chunks= (ninput_items[0] / chunk_len) + ((ninput_items[0] % chunk_len) ? 1 : 0);
      int noutput_items = chunks * chunk_len;

      return noutput_items;
    }

    void
    ho_interleave_impl::interleave_chunk(const uint8_t *in, uint8_t *out, int in_len)
    {
      memset(out, 0, chunk_len);

      int chunk_len_bits= chunk_len * 8;

      for(int in_bit_idx=0; in_bit_idx < chunk_len_bits; in_bit_idx++) {
        int in_byte_idx= in_bit_idx/8;
        int in_bit_in_byte= in_bit_idx%8;

        int out_bit_idx= permute_map[in_bit_idx];
        int out_byte_idx= out_bit_idx/8;
        int out_bit_in_byte= out_bit_idx%8;

        uint8_t in_byte= (in_byte_idx < in_len) ? in[in_byte_idx] : 0;
        uint8_t in_bit= (in_byte >> in_bit_in_byte) & 0x01;

        out[out_byte_idx]|= in_bit << out_bit_in_byte;
      }
    }

    uint32_t
    ho_interleave_impl::lfsr (uint32_t *state)
    {
      uint32_t res= 0;

      for(int i=0; i<32; i++) {
        uint_fast8_t bit= (*state) & 1;

        (*state)>>= 1;

        if(bit) (*state)^= 0x80000057;

        res= (res << 1) | bit;
      }

      return res;
    }

    int
    ho_interleave_impl::work (int noutput_items,
                              gr_vector_int &ninput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
    {
      const uint8_t *in = (const uint8_t *) input_items[0];
      uint8_t *out = (uint8_t *) output_items[0];

      int in_len= ninput_items[0];
      int cidx=0;

      for(cidx=0; cidx < in_len; cidx+=chunk_len) {
        int rem_len= in_len - cidx;
        int this_len= (rem_len < chunk_len) ? rem_len : chunk_len;

        interleave_chunk(&in[cidx], &out[cidx], this_len);
      }

      // Tell runtime system how many output items we produced.
      return cidx;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
