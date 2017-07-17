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
                       gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      this->fft_len= fft_len;
      this->power_thres_hi_sq= power_thres_hi * power_thres_hi;
      this->power_thres_lo_sq= power_thres_lo * power_thres_hi;

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
      gr_complex *out = (gr_complex *) output_items[0];

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

      /* The windowed array is shorter than noutputs by
       * one symbol as we only nead to detect preamble_a's
       * for which there is also a preamble_b in the input */
      int window_len= noutput_items - fft_len;
      gr_complex acc= 0;

      for(int widx=-half_len; widx<-1; widx++) {
        acc+= in[widx + half_len] * std::conj(in[widx + fft_len]);
      }

      int widx_start= -1;
      int widx_max= -1;
      float pw_sq_max= 0;

      /* Calculate the autocorrelation of
       * the input at offset half_len */
      for(int widx=0; widx < window_len; widx++) {
        // Add next item that slides into the window
        acc+= in[widx + half_len - 1] * std::conj(in[widx + fft_len - 1]);


        // Find peaks in Power
        float pw_sq= std::norm(acc) / (half_len * half_len);

        if(widx_start < 0) {
          if(pw_sq > power_thres_hi_sq) {
            widx_start= widx;
            pw_sq_max= 0;
          }
        }
        else {
          if(pw_sq > pw_sq_max) {
            widx_max= widx;
            pw_sq_max= pw_sq;
          }

          if(pw_sq < power_thres_lo_sq) {
            float phase= std::arg(acc);

            int oidx_abs_max= nitems_written(0) + widx_max;

            add_item_tag(0, oidx_abs_max,
                         pmt::mp("schmidl_cox_phase"),
                         pmt::from_double(phase));

            add_item_tag(0, oidx_abs_max,
                         pmt::mp("schmidl_cox_pw_max"),
                         pmt::from_double(sqrt(pw_sq_max)));

            widx_start= -1;
          }
        }

        // Subtract next item that slides out of the window
        acc-= in[widx] * std::conj(in[widx + half_len]);
      }

      memcpy(out, &in[fft_len * 2],
             sizeof(gr_complex) * noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
