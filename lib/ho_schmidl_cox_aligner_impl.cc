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
#include "ho_schmidl_cox_aligner_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_schmidl_cox_aligner::sptr
    ho_schmidl_cox_aligner::make(int fft_len, int cp_len)
    {
      return gnuradio::get_initial_sptr
        (new ho_schmidl_cox_aligner_impl(fft_len, cp_len));
    }

    /*
     * The private constructor
     */
    ho_schmidl_cox_aligner_impl::ho_schmidl_cox_aligner_impl(int fft_len, int cp_len)
      : gr::block("ho_schmidl_cox_aligner",
                  gr::io_signature::make(1, 1, sizeof(gr_complex)),
                  gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      out_alignment= fft_len + cp_len;

      msgid_top_queued= -1;
      msgid_top_unqueued= -1;
      msgid_top_acked -1;

      set_history(out_alignment);

      pmt::pmt_t msg_ack_port= pmt::mp("msg_ack");
      message_port_register_in(msg_ack_port);
      set_msg_handler(msg_ack_port,
                      boost::bind(&ho_schmidl_cox_aligner_impl::on_msg_ack, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    ho_schmidl_cox_aligner_impl::~ho_schmidl_cox_aligner_impl()
    {
    }

    void
    ho_schmidl_cox_aligner_impl::on_msg_ack(pmt::pmt_t msg)
    {
      if(msg && pmt::is_number(msg)) {
        int64_t ack= pmt::to_long(msg);

        if(ack > msgid_top_acked) {
          msgid_top_acked= ack;
        }
      }
    }

    void
    ho_schmidl_cox_aligner_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      size_t samples_in_queue= sample_queue.size() * out_alignment;

      if(noutput_items < samples_in_queue) {
        ninput_items_required[0] = 0;
      }
      else {
        int req= noutput_items - samples_in_queue;

        if (req < out_alignment) {
          req= out_alignment;
        }

        ninput_items_required[0] = req;
      }

      fprintf(stderr, "ho_scalign: forecast %d -> %d\n",
              ninput_items_required[0],
              noutput_items);
    }

    int
    ho_schmidl_cox_aligner_impl::general_work (int noutput_items,
                                               gr_vector_int &ninput_items,
                                               gr_vector_const_void_star &input_items,
                                               gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];


      /* Take samples from the input and queue them
       * for outputting */

      int idx_in= 0;

      fprintf(stderr, "ho_scalign: enter general_work(%d, %d)\n",
              noutput_items, ninput_items[0]);

      while(idx_in < (ninput_items[0] - out_alignment)) {
        fprintf(stderr, "%d < (%d - %d) = %d => %s\n",
                idx_in, ninput_items[0], out_alignment,
                ninput_items[0] - out_alignment,
                (idx_in < (ninput_items[0] - out_alignment)) ? "true" : "false");

        fprintf(stderr, "ho_scalign: append loop idx_in: %d\n", idx_in);

        if(msgid_top_queued > msgid_top_acked) {
          /* The currently streamed message id has not
           * yet been acknoledged, so it should be added
           * to the output queue */

          struct sample_queue_elem_t elem;

          elem.msg_id= msgid_top_queued;
          elem.samples= new gr_complex[out_alignment];
          memcpy(elem.samples,
                 &in[idx_in],
                 sizeof(gr_complex) * out_alignment);
        }

        std::vector<tag_t> tags;
        get_tags_in_window(tags, 0,
                           idx_in, idx_in + out_alignment,
                           pmt::mp("schmidl_cox_phase"));

        if(tags.empty()) {
          fprintf(stderr, "ho_scalign: no tags in sight\n", idx_in);

          // There is no new start tag, no need to realign

          idx_in+= out_alignment;
        }
        else {
          // There is a new start tag, realign stream

          fprintf(stderr, "ho_scalign: found tag\n");

          tag_t tag= tags.back();

          msgid_top_queued++;

          idx_in= tag.offset - nitems_read(0);
        }
      }

      fprintf(stderr, "ho_scalign: munching %d items\n", idx_in);

      consume_each(idx_in);


      /* Pop elements from the queue and write them to the output
       * as long as there are elements and output space left */

      int idx_out=0;

      while((idx_out < (noutput_items - out_alignment)) &&
            !sample_queue.empty()) {

        fprintf(stderr, "ho_scalign: spit some elements idx_out: %d\n", idx_out);

        struct sample_queue_elem_t elem= sample_queue.front();

        if(elem.msg_id > msgid_top_acked) {
          memcpy(&out[idx_out],
                 elem.samples,
                 sizeof(gr_complex) * out_alignment);

          if(elem.msg_id > msgid_top_unqueued) {
            msgid_top_unqueued= elem.msg_id;

            add_item_tag(0, nitems_written(0) + idx_out,
                         pmt::mp("message_id"),
                         pmt::from_long(elem.msg_id));
          }

          idx_out+= out_alignment;
        }

        sample_queue.pop();
      }

      return idx_out;
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
