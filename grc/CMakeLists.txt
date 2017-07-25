# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
install(FILES
    hnez_ofdm_ho_add_header.xml
    hnez_ofdm_ho_hamming74.xml
    hnez_ofdm_ho_interleave.xml
    hnez_ofdm_ho_fec.xml
    hnez_ofdm_ho_assign_carriers.xml
    hnez_ofdm_ho_qam4_multimod.xml
    hnez_ofdm_ho_add_schmidlcox.xml
    hnez_ofdm_ho_add_cyclicprefix.xml
    hnez_ofdm_ho_schmidl_cox_gate.xml DESTINATION share/gnuradio/grc/blocks
)
