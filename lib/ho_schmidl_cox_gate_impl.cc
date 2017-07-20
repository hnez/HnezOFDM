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
#include "ho_schmidl_cox_gate_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_schmidl_cox_gate::sptr
    ho_schmidl_cox_gate::make(int fft_len, int cp_len, float rel_pw_lo, float rel_pw_hi, double sample_rate)
    {
      return gnuradio::get_initial_sptr
        (new ho_schmidl_cox_gate_impl(fft_len, cp_len, rel_pw_lo, rel_pw_hi, sample_rate));
    }

    /*
     * The private constructor
     */
    ho_schmidl_cox_gate_impl::ho_schmidl_cox_gate_impl(int fft_len, int cp_len,
                                                       float rel_pw_lo, float rel_pw_hi,
                                                       double sample_rate)
      : gr::block("ho_schmidl_cox_gate",
                  gr::io_signature::make(1, 1, sizeof(gr_complex)),
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len)),
      d_fft_len(fft_len),
      d_cp_len(cp_len),
      d_rel_pw_lo(rel_pw_lo),
      d_rel_pw_hi(rel_pw_hi),
      d_sample_rate(sample_rate),
      d_am_aligned(false)
    {
      /* TODO: find out if the +1 is actually correct */
      set_history(fft_len + 1);
    }

    /*
     * Our virtual destructor.
     */
    ho_schmidl_cox_gate_impl::~ho_schmidl_cox_gate_impl()
    {
    }

    void
    ho_schmidl_cox_gate_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items * (d_fft_len + d_cp_len);
    }

    int
    ho_schmidl_cox_gate_impl::general_work (int noutput_items,
                                            gr_vector_int &ninput_items,
                                            gr_vector_const_void_star &input_items,
                                            gr_vector_void_star &output_items)
    {
      const gr_complex *in_history= (gr_complex *) input_items[0];
      gr_complex *out= (gr_complex *) output_items[0];

      int len_in= ninput_items[0];
      int fft_half_len= d_fft_len/2;
      int in_alignment= d_fft_len + d_cp_len;
      int out_alignment= d_fft_len;

      const gr_complex *in= &in_history[d_fft_len];

      gr_complex energy_detect= 0;
      float energy_ref= 0;


      /* First the energy_detect and energy_ref accumulators
       * have to be primed with the values from history */
      for(int idx_in=-fft_half_len; idx_in<0; idx_in++) {
        energy_detect+= in[idx_in] * std::conj(in[idx_in - fft_half_len]);
        energy_ref+= std::norm(in[idx_in]);
      }

      int idx_in=0, idx_out=0;
      for(;
          (idx_in < (len_in - in_alignment)) && (idx_out < noutput_items);
          idx_in+=in_alignment) {

        if(d_am_aligned) {
          // Output the symbol but not the cyclic prefix

          memcpy(&out[idx_out * out_alignment],
                 &in[idx_in + d_cp_len],
                 out_alignment);

          idx_out+= 1;
        }

        int idx_in_new= -1;

        for(int idx_win=idx_in;
            idx_win < (idx_in + in_alignment);
            idx_win++) {
          // Add next item that slides into the window
          energy_detect+= in[idx_win] * std::conj(in[idx_win - fft_half_len]);
          energy_ref+= std::norm(in[idx_win]);

          /* - power_detect keeps track of the power of the time-shifted
           *   input sigal times the unshifted signal.
           *   This contains peaks if both are correlated but is also
           *   dependent on the overall input power.
           * - power_ref keeps track of the overall input power
           * - relative_power is power_detect normalized by the mean input
           *   power in the analyzed window. */
          float power_detect= std::abs(energy_detect) / fft_half_len;
          float power_ref= energy_ref / fft_half_len;
          float relative_power= (power_ref > 0) ? (power_detect / power_ref) : 0.0;


          /* The Peaks in relative_power look something like the ACII-Art below:
           *
           *        ~~~~
           *       /    \        --- d_rel_pw_hi
           *      /      \
           *     /        \      --- d_rel_pw_lo
           * ~~~           ~~~~~
           *         |
           *         +---------- --- d_rel_pw_max
           *
           * There should be some hysteresis between d_rel_pw_hi and
           * d_rel_pw_lo to prevent detecting the same peak twice.
           * The plateau is due to the cyclic prefixing and its length
           * is influenced by the length of the channels impulse response
           * and the cyclic prefix length  */
          if (d_am_realigning) {
            if (relative_power < d_rel_pw_lo) {
              d_am_realigning= false;
              d_am_aligned= true;

              uint64_t idx_abs_now= nitems_read(0) + idx_win;
              uint64_t idx_abs_first_sym= d_rel_pw_max.abs_idx + in_alignment;

              idx_in_new= idx_abs_first_sym - idx_abs_now;
            }
          }
          else {
            if (relative_power > d_rel_pw_hi) {
              d_am_realigning= true;
              d_am_aligned= false;

              d_rel_pw_max.power= 0;
            }
          }

          // This is used for realignment
          if (relative_power > d_rel_pw_max.power) {
            d_rel_pw_max.power= relative_power;
            d_rel_pw_max.energy= energy_detect;
            d_rel_pw_max.abs_idx= nitems_read(0) + idx_win;
          }

          // Subtract next item that slides out of the window
          energy_detect-= in[idx_win - fft_half_len] * std::conj(in[idx_win - d_fft_len]);
          energy_ref-= std::norm(in[idx_win - fft_half_len]);
        }

        if(!d_am_realigning &&
           (idx_in_new > 0) &&
           (idx_in_new < len_in)) {

          for(;idx_in < idx_in_new; idx_in++) {
            // Fast-forward the accumulators

            energy_detect+= in[idx_in] * std::conj(in[idx_in - fft_half_len]);
            energy_ref+= std::norm(in[idx_in]);

            energy_detect-= in[idx_in - fft_half_len] * std::conj(in[idx_in - d_fft_len]);
            energy_ref-= std::norm(in[idx_in - fft_half_len]);
          }
        }
      }

      consume_each (idx_in);

      // Tell runtime system how many output items we produced.
      return idx_out;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
