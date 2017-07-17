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

#include <gnuradio/blocks/pdu.h>
#include <gnuradio/io_signature.h>
#include "ho_pdu_to_stream_async_impl.h"

namespace gr {
  namespace hnez_ofdm {

    ho_pdu_to_stream_async::sptr
    ho_pdu_to_stream_async::make()
    {
      return gnuradio::get_initial_sptr
        (new ho_pdu_to_stream_async_impl());
    }

    /*
     * The private constructor
     */
    ho_pdu_to_stream_async_impl::ho_pdu_to_stream_async_impl()
      : gr::sync_block("ho_pdu_to_stream_async",
                       gr::io_signature::make(0, 0, 0),
                       gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      message_port_register_in(PDU_PORT_ID);
    }

    /*
     * Our virtual destructor.
     */
    ho_pdu_to_stream_async_impl::~ho_pdu_to_stream_async_impl()
    {
    }

    int
    ho_pdu_to_stream_async_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items)
    {
      gr_complex *out = (gr_complex *) output_items[0];

      pmt::pmt_t msg; //= delete_head_nowait(PDU_PORT_ID);

      if(0) { //pmt::is_pair(msg) && pmt::is_c32vector(pmt::cdr(msg))) {
        pmt::pmt_t in_vec= pmt::cdr(msg);

        int in_items= pmt::blob_length(in_vec)/sizeof(gr_complex);

        if(in_items > noutput_items) {
          throw std::runtime_error("Got a message that was longer than the output buffer");
        }

        size_t len= 0;
        const uint8_t* in_ptr = (const uint8_t*) uniform_vector_elements(in_vec, len);
        memcpy(out, in_ptr, sizeof(gr_complex) * in_items);

        return in_items;
      }
      else {
        int pad_len= (noutput_items < 512) ? noutput_items : 512;

        memset(out, 0, sizeof(gr_complex) * pad_len);

        return pad_len;
      }
    }

  } /* namespace hnez_ofdm */
} /* namespace gr */
