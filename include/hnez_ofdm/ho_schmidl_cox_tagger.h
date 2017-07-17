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


#ifndef INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_TAGGER_H
#define INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_TAGGER_H

#include <hnez_ofdm/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace hnez_ofdm {

    /*!
     * \brief <+description of block+>
     * \ingroup hnez_ofdm
     *
     */
    class HNEZ_OFDM_API ho_schmidl_cox_tagger : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<ho_schmidl_cox_tagger> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of hnez_ofdm::ho_schmidl_cox_tagger.
       *
       * To avoid accidental use of raw pointers, hnez_ofdm::ho_schmidl_cox_tagger's
       * constructor is in a private implementation
       * class. hnez_ofdm::ho_schmidl_cox_tagger::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fft_len,
                       float power_thres_hi,
                       float power_thres_lo);
    };

  } // namespace hnez_ofdm
} // namespace gr

#endif /* INCLUDED_HNEZ_OFDM_HO_SCHMIDL_COX_TAGGER_H */
