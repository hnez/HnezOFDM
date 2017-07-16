/* -*- c++ -*- */
/* 
 * Copyright 2017  Leonard Göhrs <leonard@goehrs.eu>.
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

#ifndef INCLUDED_HNEZ_OFDM_HO_QAM4_MULTIMOD_IMPL_H
#define INCLUDED_HNEZ_OFDM_HO_QAM4_MULTIMOD_IMPL_H

#include <hnez_ofdm/ho_qam4_multimod.h>

namespace gr {
  namespace hnez_ofdm {

    class ho_qam4_multimod_impl : public ho_qam4_multimod
    {
    private:
      int output_width;
      pmt::pmt_t len_tag_key;

    public:
      ho_qam4_multimod_impl(int output_width, const std::string &len_tag_key);
      ~ho_qam4_multimod_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_QAM4_MULTIMOD_IMPL_H */

