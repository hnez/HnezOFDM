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

from __future__ import print_function

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import hnez_ofdm_swig as hnez_ofdm

import numpy as np


class qa_ho_schmidl_cox_gate (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def random_complex(self, rnd, width, length):
        amplitudes= rnd.normal(0, width, length)
        phases= rnd.uniform(-np.pi, np.pi, length)

        return amplitudes * np.exp(1j * phases)

    def test_001_t (self):
        rnd= np.random.RandomState(0)

        pad_len= 9000
        fft_len= 1024
        cp_len= 32

        ph_rot= 0.2 / fft_len

        preamble_halves= self.random_complex(rnd, 1, fft_len/2)
        preamble= np.concatenate((preamble_halves, preamble_halves))

        test_symbol= self.random_complex(rnd, 1, fft_len)

        frame= np.concatenate((
            preamble[-cp_len:], preamble, test_symbol[-cp_len:], test_symbol
        ))

        noise_pre= self.random_complex(rnd, 0.5, pad_len)
        noise_during= self.random_complex(rnd, 0.0001, len(frame))
        noise_post= self.random_complex(rnd, 0.5, pad_len)

        sent= np.concatenate((noise_pre, frame + noise_during, noise_post))
        sent*= np.exp(1j * np.linspace(
            0,
            ph_rot * len(sent),
            len(sent))
        )

        dat_src= blocks.vector_source_c(sent, False, 1, [])
        gate= hnez_ofdm.ho_schmidl_cox_gate(fft_len, cp_len, 0.8, 0.9)
        dat_sink= blocks.vector_sink_c(fft_len)

        self.tb.connect((dat_src, 0), (gate, 0))
        self.tb.connect((gate, 0), (dat_sink, 0))

        self.tb.run ()

        received= dat_sink.data()

        snd_preamble_fd= np.fft.fft(preamble)
        rcv_preamble_fd= np.fft.fft(received[:fft_len])

        preamble_d= ((abs(rcv_preamble_fd) - abs(snd_preamble_fd))**2).sum() / fft_len

        self.assertLess(preamble_d, 10e-6)

        snd_symbol_fd= np.fft.fft(test_symbol)
        rcv_symbol_fd= np.fft.fft(received[fft_len:fft_len*2])

        symbol_d= ((abs(rcv_symbol_fd) - abs(snd_symbol_fd))**2).sum() / fft_len

        self.assertLess(symbol_d, 10e-6)

if __name__ == '__main__':
    gr_unittest.run(qa_ho_schmidl_cox_gate, "qa_ho_schmidl_cox_gate.xml")
