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
      const struct {
        int fft;
        int cp;
        int preamble;
      } d_lengths;

      const struct {
        float low;
        float high;
      } d_relative_thresholds;

      struct d_energy_history_t{
      private:
        const size_t history_len;
        const std::unique_ptr<gr_complex[]> history;

        size_t history_idx;

        float acc_ref;
        gr_complex acc_detect;

        bool ready;

      public:
        d_energy_history_t(size_t fft_len);

        void update(gr_complex now);
        void reset();

        gr_complex det_energy_raw();
        float det_power_relative();
      } d_energy_history;

      struct {
        bool am_inside;
        float relative_power;
        gr_complex energy;
        int64_t abs_idx;
      } d_power_peak;

      struct {
        gr_complex phase_acc;
        gr_complex phase_rot;
      } d_fq_compensation;

      bool d_am_aligned;
      uint64_t d_frame_id;

      void on_frame_ack(pmt::pmt_t msg);

    public:
      ho_schmidl_cox_gate_impl(int fft_len, int cp_len,
                               float rel_pw_lo, float rel_pw_hi);

      ~ho_schmidl_cox_gate_impl();

      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_GATE_IMPL_H */
