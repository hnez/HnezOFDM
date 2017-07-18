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

#ifndef INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_ALIGNER_IMPL_H
#define INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_ALIGNER_IMPL_H

#include <queue>

#include <hnez_ofdm/ho_schmidl_cox_aligner.h>

namespace gr {
  namespace hnez_ofdm {
    class ho_schmidl_cox_aligner_impl : public ho_schmidl_cox_aligner
    {
    private:
      struct sample_queue_elem_t {
        int64_t msg_id;
        gr_complex *samples;

        ~sample_queue_elem_t()
        {
          fprintf(stderr, "Will i survive?\n");
          delete[] samples;
          fprintf(stderr, "Yes!\n");
        }
      };

      int out_alignment;

      int64_t msgid_top_queued;
      int64_t msgid_top_unqueued;
      int64_t msgid_top_acked;

      std::queue<struct sample_queue_elem_t> sample_queue;

      void on_msg_ack(pmt::pmt_t msg);

    public:
      ho_schmidl_cox_aligner_impl(int fft_len, int cp_len);
      ~ho_schmidl_cox_aligner_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_ALIGNER_IMPL_H */
