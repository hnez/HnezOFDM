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


    ho_schmidl_cox_gate_impl::d_energy_history_t::d_energy_history_t(size_t fft_len)
      : len_window(fft_len/2),
        idx(0),
        hist_detect(new gr_complex[fft_len/2]),
        hist_reference(new float[fft_len/2])
    {
      reset();
    }

    void
    ho_schmidl_cox_gate_impl::d_energy_history_t::reset()
    {
      detect= 0;
      reference= 0;

      for(size_t i=0; i<len_window; i++) {
        hist_detect[i]= 0;
        hist_reference[i]= 0;
      }
    }

    void
    ho_schmidl_cox_gate_impl::d_energy_history_t::update(gr_complex now, gr_complex old)
    {
      hist_detect[idx]= now * std::conj(old);
      hist_reference[idx]= std::norm(now);

      detect+= hist_detect[idx];
      reference+= hist_reference[idx];

      idx= (idx + 1) % len_window;

      detect-= hist_detect[idx];
      reference-= hist_reference[idx];
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
      d_sample_rate(sample_rate),
      d_lengths({.fft=fft_len, .cp=cp_len, .preamble=fft_len/2}),
      d_relative_thresholds({.low=rel_pw_lo, .high=rel_pw_hi}),
      d_energy_history(fft_len),
      d_power_peak({.am_inside=false, .relative_power=0, .energy=0, .abs_idx=0}),
      d_am_aligned(false)
    {
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
      ninput_items_required[0] = noutput_items * (d_lengths.fft + d_lengths.cp);
    }

    int
    ho_schmidl_cox_gate_impl::general_work (int noutput_items,
                                            gr_vector_int &ninput_items,
                                            gr_vector_const_void_star &input_items,
                                            gr_vector_void_star &output_items)
    {
      const gr_complex *in_history= (gr_complex *) input_items[0];
      gr_complex *out= (gr_complex *) output_items[0];

      /* We will later use negative indices to refer to
       * elements in the history */
      const gr_complex *in= &in_history[history() - 1];

      int len_in= ninput_items[0];
      int len_out= noutput_items;
      int in_alignment= d_lengths.fft + d_lengths.cp;
      int out_alignment= d_lengths.fft;

      int idx_in=0, idx_out=0;
      while((idx_in < (len_in - in_alignment)) && (idx_out < len_out)) {
        if(d_am_aligned) {
          // Output the symbol but not the cyclic prefix

          memcpy(&out[idx_out * out_alignment],
                 &in[idx_in + d_lengths.cp],
                 out_alignment);

          idx_out++;
        }

        int idx_in_new= -1;

        for(int idx_win=idx_in;
            idx_win < (idx_in + in_alignment);
            idx_win++) {

          // Add next item that slides into the window
          d_energy_history.update(in[idx_win - d_lengths.preamble],
                                  in[idx_win - d_lengths.fft]);

          /* - power_detect keeps track of the power of the time-shifted
           *   input sigal times the unshifted signal.
           *   This contains peaks if both are correlated but is also
           *   dependent on the overall input power.
           * - power_ref keeps track of the overall input power
           * - relative_power is power_detect normalized by the mean input
           *   power in the analyzed window. */
          float power_detect= std::abs(d_energy_history.detect) / d_lengths.preamble;
          float power_ref= d_energy_history.reference / d_lengths.preamble;
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
          if (!d_power_peak.am_inside &&
              (relative_power > d_relative_thresholds.high)) {
            
              d_am_aligned= false;

              d_power_peak.am_inside= true;

              d_power_peak.relative_power= 0;
          }

          if (d_power_peak.am_inside) {
            if (relative_power > d_power_peak.relative_power) {
              d_power_peak.relative_power= relative_power;
              d_power_peak.energy= d_energy_history.detect;
              d_power_peak.abs_idx= nitems_read(0) + idx_win;
            }

            if (relative_power < d_relative_thresholds.low) {
              d_power_peak.am_inside= false;

              int64_t idx_win_abs= nitems_read(0) + idx_win;
              int64_t idx_in_new= d_power_peak.abs_idx - idx_win_abs;

              if((idx_in_new < (history() - 1)) || (idx_in > len_in)) {
                fprintf(stderr,
                        "schmid_cox_gate: realignment failed, idx_in_new=%li is out of bounds\n",
                        idx_in_new);
              }
              else {
                /* Theoretically we would now have to fo backwards from
                 * idx_in to idx_in_new and update the energy estimation.
                 * But we actually do not want to detect the same peak again.
                 * Calling reset() will flush the history and prevent detection
                 * of peaks for some time. */
                d_energy_history.reset();

                idx_in= idx_in_new;
              }
            }
          }
        }
      }
      
      consume_each (idx_in);

      // Tell runtime system how many output items we produced.
      return idx_out;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
