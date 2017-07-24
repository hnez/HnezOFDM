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
#include <volk/volk.h>
#include "ho_schmidl_cox_gate_impl.h"

namespace gr {
  namespace hnez_ofdm {
    ho_schmidl_cox_gate_impl::d_energy_history_t::d_energy_history_t(size_t fft_len)
      : history(new gr_complex[fft_len]),
        history_len(fft_len)
    {
      reset();
    }

    void
    ho_schmidl_cox_gate_impl::d_energy_history_t::reset()
    {
      for(size_t i=0; i<history_len; i++) {
        history[i]= 0;
      }

      history_idx= 0;

      acc_ref= 0;
      acc_detect= 0;

      ready= false;
    }

    void
    ho_schmidl_cox_gate_impl::d_energy_history_t::update(gr_complex now)
    {
      gr_complex pop= history[history_idx];
      gr_complex mid= history[(history_idx + history_len/2) % history_len];

      // Update reference energy
      float push_ref= norm(now);
      float pop_ref= norm(pop);

      acc_ref+= push_ref - pop_ref;

      // Update detection energy
      gr_complex push_detect= now * conj(mid);
      gr_complex pop_detect= mid * conj(pop);

      acc_detect+= push_detect - pop_detect;

      // Update index
      history[history_idx]= now;
      history_idx= (history_idx + 1) % history_len;

      // Signal readieness if the history is filled
      ready= ready || !history_idx;
    }

    gr_complex
    ho_schmidl_cox_gate_impl::d_energy_history_t::det_energy_raw()
    {
      return (ready ? acc_detect : 0);
    }

    float
    ho_schmidl_cox_gate_impl::d_energy_history_t::det_power_relative()
    {
      if(acc_ref > 0) {
        /* The window over which acc_ref is calculated is twice as
         * large as the window over which acc_detect is calculated.
         * The 2 normalizes the energies to yield the relative power. */

        return (2 * abs(det_energy_raw()) / acc_ref);
      }
      else {
        return 0;
      }
    }

    ho_schmidl_cox_gate::sptr
    ho_schmidl_cox_gate::make(int fft_len, int cp_len, float rel_pw_lo, float rel_pw_hi)
    {
      return gnuradio::get_initial_sptr
        (new ho_schmidl_cox_gate_impl(fft_len, cp_len, rel_pw_lo, rel_pw_hi));
    }

    ho_schmidl_cox_gate_impl::ho_schmidl_cox_gate_impl(int fft_len, int cp_len,
                                                       float rel_pw_lo, float rel_pw_hi)
      : gr::block("ho_schmidl_cox_gate",
                  gr::io_signature::make(1, 1, sizeof(gr_complex)),
                  gr::io_signature::make(1, 1, sizeof(gr_complex) * fft_len)),
      d_lengths({.fft=fft_len, .cp=cp_len, .preamble=fft_len/2}),
      d_relative_thresholds({.low=rel_pw_lo, .high=rel_pw_hi}),
      d_energy_history(fft_len),
      d_power_peak({.am_inside=false, .relative_power=0, .energy=0, .abs_idx=0}),
      d_fq_compensation({.phase_acc=1, .phase_rot=1}),
      d_am_aligned(false),
      d_frame_id(0)
    {
      // TODO: find out if the +1 is necessary
      set_history(fft_len + 1);

      pmt::pmt_t frame_ack_port= pmt::mp("frame_ack");
      message_port_register_in(frame_ack_port);
      set_msg_handler(frame_ack_port,
                      boost::bind(&ho_schmidl_cox_gate_impl::on_frame_ack, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    ho_schmidl_cox_gate_impl::~ho_schmidl_cox_gate_impl()
    {
    }

    void
    ho_schmidl_cox_gate_impl::on_frame_ack(pmt::pmt_t msg)
    {
      /* A later block can notify us, using this message port,
       * that it has finished processing a frame and that
       * we can stop outputting its symbols */

      if(msg && pmt::is_number(msg)) {
        uint64_t ack_id= pmt::to_uint64(msg);

        if(ack_id == d_frame_id) {
          d_am_aligned= false;
        }
      }
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

      /* idx_in is counted in samples
       * idx_out is counted in output symbols (fft_len samples) */
      int idx_in=0, idx_out=0;

      /* The loop makes sure there is always at least one complete symbol in
       * the input buffer and space for one output symbol in the output buffer */
      while((idx_in < (len_in - in_alignment - 1)) && (idx_out < len_out)) {

        /* If we are currently synchronized to a frame:
         * write a symbol from it to the output buffer. */
        if(d_am_aligned) {
          /* The phase accumulator might degenerate because of
           * accumulated rounding errors. Make sure it stays normalized. */
          d_fq_compensation.phase_acc/= abs(d_fq_compensation.phase_acc);

          /* Frequency shift and output the symbol but not the
           * cyclic prefix */
          volk_32fc_s32fc_x2_rotator_32fc(&out[idx_out * out_alignment],
                                          &in[idx_in],
                                          d_fq_compensation.phase_rot,
                                          &d_fq_compensation.phase_acc,
                                          out_alignment);

          /* Fast forward the frequency compensation over the
           * next cyclic prefix */
          d_fq_compensation.phase_acc*= pow(d_fq_compensation.phase_rot, d_lengths.cp);

          idx_out++;
        }

        bool do_realign= false;
        int64_t idx_in_realigned= 0;

        for(int idx_win=idx_in;
            idx_win < (idx_in + in_alignment);
            idx_win++) {

          // Add next item that slides into the window
          d_energy_history.update(in[idx_win + d_lengths.fft]);
          float relative_power= d_energy_history.det_power_relative();

          /* The Peaks in relative_power look something like the ACII-Art below:
           *
           *        ~~~~
           *       /    \        --- d_relative_thresholds.high
           *      /      \
           *     /        \      --- d_relative_thresholds.low
           * ~~~           ~~~~~
           *         |
           *         +---------- --- d_power_peak
           *
           * There should be some hysteresis between d_relative_thresholds.high and
           * d_relative_thresholds.low to prevent detecting the same peak twice.
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
              d_power_peak.energy= d_energy_history.det_energy_raw();
              d_power_peak.abs_idx= nitems_read(0) + idx_win;
            }

            if (relative_power < d_relative_thresholds.low) {
              d_power_peak.am_inside= false;

              idx_in_realigned= d_power_peak.abs_idx - (int64_t)nitems_read(0);

              int64_t history_start= -((int64_t)history() - 1);

              if((idx_in_realigned < history_start) || (idx_in_realigned > len_in)) {
                fprintf(stderr,
                        "schmid_cox_gate: realignment failed, idx_in_realigned=%li is out of bounds\n",
                        idx_in_realigned);
              }
              else {
                /* Theoretically we would now have to go backwards from
                 * idx_in to idx_in_realigned and update the energy estimation.
                 * But we actually do not want to detect the same peak again.
                 * Calling reset() will flush the history and prevent detection
                 * of peaks until it is filled again.
                 * This is a bit hackish, i know. */
                d_energy_history.reset();

                /* The preamble was shifted by the phase of d_power_peak.energy in
                 * d_lengths.preamble samples times, the following lines calculate the
                 * phase shift per sample, invert it and store it
                 * for later frequency offset compensation.
                 * For large frequency offsets this can become abiguos,
                 * to determine the maximum offset is left as an exercise to the reader. */
                gr_complex rot_per_sample= pow(d_power_peak.energy,
                                               1.0f/d_lengths.preamble);

                gr_complex norm_rot_per_sample= rot_per_sample / abs(rot_per_sample);

                d_fq_compensation.phase_rot= conj(norm_rot_per_sample);

                /* Add a tag to the output stream to notify the following
                 * blocks of the new frame*/
                uint64_t idx_abs= nitems_written(0) + idx_out;

                d_frame_id++;
                add_item_tag(0, idx_abs,
                             pmt::mp("frame_id"),
                             pmt::from_uint64(d_frame_id));

                add_item_tag(0, idx_abs,
                             pmt::mp("preamble_power"),
                             pmt::from_double(d_power_peak.relative_power));

                add_item_tag(0, idx_abs,
                             pmt::mp("fq_compensation"),
                             pmt::from_double(arg(d_fq_compensation.phase_rot)));

                /* Jump back to the start of the preamble */
                do_realign= true;
                d_am_aligned= true;
              }
            }
          }
        }

        idx_in= do_realign ? idx_in_realigned : (idx_in + in_alignment);
      }

      consume_each (idx_in);
      return idx_out;
    }

  }
}
