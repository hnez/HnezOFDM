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

from random import Random
import numpy as np


class qa_ho_schmidl_cox_gate (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def gen_frame (self, rnd, fft_len, cp_len, num_syms):
        def fd_symbols():
            constellations= np.array((1+1j, 1-1j, -1+1j, -1-1j))

            preamble= np.zeros(fft_len, np.complex64)
            preamble[::2]= rnd.choice(constellations * 2**0.5, fft_len/2)

            yield preamble

            for i in range(num_syms):
                yield rnd.choice(constellations, fft_len)

        def td_symbols():
            for fd_sym in fd_symbols():
                td_sym= np.fft.ifft(fd_sym)

                sym= np.concatenate((td_sym[-cp_len:], td_sym))

                yield sym

        return np.concatenate(tuple(td_symbols()))

    def random_complex(self, rnd, width, length):
        amplitudes= rnd.normal(0, width, length)
        phases= rnd.uniform(-np.pi, np.pi, length)

        return amplitudes * np.exp(1j * phases)

    def test_001_t (self):
        fft_len= 1024
        cp_len= 20
        dat_len= 100000
        frame_len= 5
        frame_offs= (9000, 50000)

        rnd= np.random.RandomState(0)

        dat= self.random_complex(rnd, 0.05, dat_len)

        frames= list(
            self.gen_frame(rnd, fft_len, cp_len, frame_len)
            for off in frame_offs
        )

        for (frame, off) in zip(frames, frame_offs):
            dat[off:off+len(frame)]+= frame

            print('frame:', off, '-', off+len(frame))

        dat*= np.exp(1j * np.linspace(0, 0.003 * dat_len, dat_len))

        dat_src= blocks.vector_source_c(dat, False, 1, [])
        gate= hnez_ofdm.ho_schmidl_cox_gate(fft_len, cp_len, 0.3, 0.4)
        dat_sink= blocks.vector_sink_c(fft_len)

        self.tb.connect((dat_src, 0), (gate, 0))
        self.tb.connect((gate, 0), (dat_sink, 0))

        self.tb.run ()

        out_dat= dat_sink.data()

        for (f, t) in zip(frames, dat_sink.tags()):
            off= t.offset * fft_len
            preamble_t= out_dat[off:off+fft_len]
            preamble_f= f[cp_len:cp_len+fft_len]

            print('off: {}'.format(off))

            pfftt= np.fft.fft(preamble_t)
            pfftf= np.fft.fft(preamble_f)

            #print(', '.join(str(abs(s)) for s in pfftf))


if __name__ == '__main__':
    gr_unittest.run(qa_ho_schmidl_cox_gate, "qa_ho_schmidl_cox_gate.xml")
