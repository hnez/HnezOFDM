/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_HNEZ_OFDM_HO_PDU_TO_STREAM_ASYNC_IMPL_H
#define INCLUDED_HNEZ_OFDM_HO_PDU_TO_STREAM_ASYNC_IMPL_H

#include <hnez_ofdm/ho_pdu_to_stream_async.h>

namespace gr {
  namespace hnez_ofdm {

    class ho_pdu_to_stream_async_impl : public ho_pdu_to_stream_async
    {
     private:
      // Nothing to declare in this block.

     public:
      ho_pdu_to_stream_async_impl();
      ~ho_pdu_to_stream_async_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_PDU_TO_STREAM_ASYNC_IMPL_H */

