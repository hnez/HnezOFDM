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
#include "ho_schmidl_cox_tagger_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_schmidl_cox_tagger::sptr
    ho_schmidl_cox_tagger::make(int fft_len,
                                float power_thres_hi,
                                float power_thres_lo)
    {
      return gnuradio::get_initial_sptr
        (new ho_schmidl_cox_tagger_impl(fft_len, power_thres_hi, power_thres_lo));
    }

    /*
     * The private constructor
     */
    ho_schmidl_cox_tagger_impl::ho_schmidl_cox_tagger_impl(int fft_len,
                                                           float power_thres_hi,
                                                           float power_thres_lo)
      : gr::sync_block("ho_schmidl_cox_tagger",
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       gr::io_signature::make(2, 2, sizeof(gr_complex)))
    {
      this->fft_len= fft_len;
      this->power_thres_hi= power_thres_hi;
      this->power_thres_lo= power_thres_lo;

      // We need at least the two preamble symbols
      set_history(2 * fft_len);
    }

    /*
     * Our virtual destructor.
     */
    ho_schmidl_cox_tagger_impl::~ho_schmidl_cox_tagger_impl()
    {
    }

    int
    ho_schmidl_cox_tagger_impl::work(int noutput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out_orig = (gr_complex *) output_items[0];
      gr_complex *out_pw = (gr_complex *) output_items[1];
      
      int half_len= fft_len/2;

      /* Symbol Types: A - Preamble A - Repeating at half length
       *               B - Preamble B - Not repeating
       *               L - Payload
       *
       * Window idx:  - - - -|- - - -|0 0 0 0 0 0 0 0|0 0 1 1 1 1 1 1|1 1 1 1
       *              8 7 6 5|4 3 2 1|0 1 2 3 4 5 6 7|8 9 0 1 2 3 4 5|6 7 8 9
       *                     |       |               |               |
       * Output idx:  - - - -|- - - -|- - - - - - - -|0 0 0 0 0 0 0 0|0 0 1 1
       *                     |       |8 7 6 5 4 3 2 1|0 1 2 3 4 5 6 7|8 9 0 1
       *                     |       |               |               |
       * Symbol type:        |A A A A|A A A A B B B B|B B B B L L L L|L L L L
       * Symbol idx:         |0 1 2 3|4 5 6 7 0 1 2 3|4 5 6 7 0 1 2 3|4 5 6 7
       *                     |       |               |               |
       * Symbol type  A A A A|A A A A|B B B B B B B B|L L L L L L L L|
       * Symbol idx:  0 1 2 3|4 5 6 7|0 1 2 3 4 5 6 7|0 1 2 3 4 5 6 7|
       */

      /* The window length is shorter than noutputs by
       * one symbol as we only nead to detect preamble_a's
       * for which there is also already a preamble_b in the input */
      int window_len= noutput_items;
      gr_complex energy_detect= 0;
      float energy_ref= 0;

      for(int widx=-half_len; widx<-1; widx++) {
        energy_detect+= in[widx + half_len] * std::conj(in[widx + fft_len]);
        energy_ref+= std::norm(in[widx + half_len]);
      }

      int widx_start= -1;
      int widx_max= -1;
      float relative_power_max= 0;

      /* Calculate the autocorrelation of
       * the input at offset half_len */
      for(int widx=0; widx < window_len; widx++) {
        // Add next item that slides into the window
        energy_detect+= in[widx + half_len - 1] * std::conj(in[widx + fft_len - 1]);
        energy_ref+= std::norm(in[widx + half_len - 1]);

        /* - power_detect keeps track of the power of the time-shifted
         *   input sigal times the unshifted signal.
         *   This contains peaks if both are correlated but is also
         *   dependent on the overall input power.
         * - power_ref keeps track of the overall input power
         * - relative_power is power_detect normalized by the mean input
         *   power in the analyzed window. */
        float power_detect= std::abs(energy_detect) / half_len;
        float power_ref= energy_ref / half_len;
        float relative_power= (power_ref != 0.0) ? (power_detect / power_ref) : 0.0;

        /* The Peaks in relative_power look something like the ACII-Art below:
         *
         *        ~~~~
         *       /    \        --- power_thres_hi
         *      /      \
         *     /        \      --- power_thres_lo
         * ~~~           ~~~~~
         *         |
         *         +---------- --- widx_max / relative_power_max
         *
         * There should be some hysteresis between power_thres_hi and
         * power_thres_lo to prevent detecting the same peak twice.
         * The plateau is due to the cyclic prefixing and its length
         * is influenced by the length of the channels impulse response
         * and the cyclic prefix length  */
        if(widx_start < 0) {
          if(relative_power > power_thres_hi) {
            widx_start= widx;
            relative_power_max= relative_power;
          }
        }
        else {
          if(relative_power > relative_power_max) {
            widx_max= widx;
            relative_power_max= relative_power;
          }

          if(relative_power < power_thres_lo) {
            float phase= std::arg(energy_detect);

            int oidx_abs_max= nitems_written(0) + widx_max;

            add_item_tag(0, oidx_abs_max,
                         pmt::mp("schmidl_cox_phase"),
                         pmt::from_double(phase));

            add_item_tag(0, oidx_abs_max,
                         pmt::mp("schmidl_cox_relpw_max"),
                         pmt::from_double(relative_power_max));

            widx_start= -1;
          }
        }

        out_pw[widx]= relative_power;

        // Subtract next item that slides out of the window
        energy_detect-= in[widx] * std::conj(in[widx + half_len]);
        energy_ref-= std::norm(in[widx]);
      }

      /* Because of the set_history call in the constructor
       * copying the input directly to the output will introduce
       * a delay of two symbols */
      memcpy(out_orig, in, sizeof(gr_complex) * noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
