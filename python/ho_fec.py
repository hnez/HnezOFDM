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

from gnuradio import gr
from gnuradio import blocks
import hnez_ofdm_swig as hnez_ofdm

class ho_fec(gr.hier_block2):
    """
    docstring for block ho_fec
    """
    def __init__(self, encode, len_tag_key='packet_len'):
        gr.hier_block2.__init__(
            self,
            "ho_fec",
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char)
        )

        if encode:
            repack_pre= blocks.repack_bits_bb(8, 4, len_tag_key, False, gr.GR_LSB_FIRST)
            hamming= hnez_ofdm.ho_hamming74(True, len_tag_key)
            repack_post= blocks.repack_bits_bb(7, 8, len_tag_key, False, gr.GR_LSB_FIRST)

        else:
            repack_pre= blocks.repack_bits_bb(8, 7, len_tag_key, False, gr.GR_LSB_FIRST)
            hamming= hnez_ofdm.ho_hamming74(False, len_tag_key)
            repack_post= blocks.repack_bits_bb(4, 8, len_tag_key, False, gr.GR_LSB_FIRST)

        self.connect((self, 0), (repack_pre, 0))
        self.connect((repack_pre, 0), (hamming, 0))
        self.connect((hamming, 0), (repack_post, 0))
        self.connect((repack_post, 0), (self, 0))
