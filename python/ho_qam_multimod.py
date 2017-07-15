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

from gnuradio import gr

class ho_qam_multimod(gr.hier_block2):
    """
    docstring for block ho_qam_multimod
    """
    def __init__(self, num_symbols, num_points=4):
        gr.hier_block2.__init__(self,
            "ho_qam_multimod",
            gr.io_signature(<+MIN_IN+>, <+MAX_IN+>, gr.sizeof_<+ITYPE+>),  # Input signature
            gr.io_signature(<+MIN_OUT+>, <+MAX_OUT+>, gr.sizeof_<+OTYPE+>)) # Output signature

            # Define blocks and connect them
            self.connect()
