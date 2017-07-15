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

class qa_ho_qam4_multimod (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        data= tuple(i for i in range(10))
        data_src= blocks.vector_source_b(data, False, 1, [])
        dst= blocks.vector_sink_c(40)

        modulator= hnez_ofdm.ho_qam4_multimod(40)

        self.tb.connect((data_src, 0), (modulator, 0))
        self.tb.connect((modulator, 0), (dst, 0))

        self.tb.run ()

        actual_result= dst.data()

        powers= tuple(map(abs, actual_result))

        self.assertFloatTuplesAlmostEqual(
            tuple(1.0 for p in powers),
            powers,
            6
        )

if __name__ == '__main__':
    gr_unittest.run(qa_ho_qam4_multimod, "qa_ho_qam4_multimod.xml")
