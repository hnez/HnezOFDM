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

#ifndef INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_GATE_IMPL_H
#define INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_GATE_IMPL_H

#include <hnez_ofdm/ho_schmidl_cox_gate.h>

namespace gr {
  namespace hnez_ofdm {

    class ho_schmidl_cox_gate_impl : public ho_schmidl_cox_gate
    {
    private:
      int d_fft_len;
      int d_cp_len;
      float d_rel_pw_lo;
      float d_rel_pw_hi;
      double d_sample_rate;

      bool d_am_aligned;

      bool d_am_realigning;
      struct {
        uint64_t abs_idx;
        float power;
        gr_complex energy;
      } d_rel_pw_max;

    public:
      ho_schmidl_cox_gate_impl(int fft_len, int cp_len, float rel_pw_lo, float rel_pw_hi, double sample_rate);
      ~ho_schmidl_cox_gate_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_GATE_IMPL_H */

