#!/usr/bin/env python2
# -*- coding: utf-8 -*-
#
# Copyright 2017 Leonard Göhrs <leonard@goehrs.eu>.
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

class qa_ho_add_header (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        data= (1, 2, 3, 4, 5, 6, 7, 8)

        data_src= blocks.vector_source_b(data, False, 1, [])
        stream_tagger= blocks.stream_to_tagged_stream(gr.sizeof_char, 1, len(data), "packet_len")
        header_adder= hnez_ofdm.ho_add_header("packet_len")
        dst= blocks.vector_sink_b()

        self.tb.connect((data_src, 0), (stream_tagger, 0))
        self.tb.connect((stream_tagger, 0), (header_adder, 0))
        self.tb.connect((header_adder, 0), (dst, 0))

        self.tb.run ()

        expected_result= (0, 0, 0, 8, 235, 244, 114, 39) + data
        actual_result= dst.data()

        self.assertSequenceEqual(
            expected_result,
            actual_result[:len(expected_result)]
        )

if __name__ == '__main__':
    gr_unittest.run(qa_ho_add_header, "qa_ho_add_header.xml")
