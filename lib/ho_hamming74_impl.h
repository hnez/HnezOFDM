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

#ifndef INCLUDED_HNEZ_OFDM_HO_HAMMING74_IMPL_H
#define INCLUDED_HNEZ_OFDM_HO_HAMMING74_IMPL_H

#include <hnez_ofdm/ho_hamming74.h>

namespace gr {
  namespace hnez_ofdm {

    class ho_hamming74_impl : public ho_hamming74
    {
    private:
      bool do_encode;

    protected:
      int calculate_output_stream_length(const gr_vector_int &ninput_items);

    public:
      ho_hamming74_impl(bool encode, const std::string& len_tag_key);
      ~ho_hamming74_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_int &ninput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_HAMMING74_IMPL_H */

