#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# Copyright 2017 <+YOU OR YOUR COMPANY+>.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import hnez_ofdm_swig as hnez_ofdm

class qa_ho_interleave (gr_unittest.TestCase):
    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        data= tuple(i for i in range(100))
        data_src= blocks.vector_source_b(data, False, 1, [])
        stream_tagger= blocks.stream_to_tagged_stream(
            gr.sizeof_char, 1, len(data), "packet_len"
        )
        dst= blocks.vector_sink_b()

        encoder= hnez_ofdm.ho_interleave(90, True, 'packet_len')
        decoder= hnez_ofdm.ho_interleave(90, False, 'packet_len')

        self.tb.connect((data_src, 0), (stream_tagger, 0))
        self.tb.connect((stream_tagger, 0), (encoder, 0))
        self.tb.connect((encoder, 0), (decoder, 0))
        self.tb.connect((decoder, 0), (dst, 0))

        self.tb.run ()

        actual_result= dst.data()

        self.assertSequenceEqual(
            data,
            actual_result[:len(data)]
        )

if __name__ == '__main__':
    gr_unittest.run(qa_ho_interleave, "qa_ho_interleave.xml")
