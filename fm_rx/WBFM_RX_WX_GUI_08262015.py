#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: RTL WBFM RX WX GUI
# Author: John Petrich, W7FU
# Description: FM Receiver Demo
# Generated: Tue Oct 22 21:50:22 2019
##################################################


if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import wxgui
from gnuradio.eng_option import eng_option
from gnuradio.fft import window
from gnuradio.filter import firdes
from gnuradio.wxgui import fftsink2
from gnuradio.wxgui import forms
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import osmosdr
import time
import wx


class WBFM_RX_WX_GUI_08262015(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="RTL WBFM RX WX GUI")
        _icon_path = "/usr/share/icons/hicolor/32x32/apps/gnuradio-grc.png"
        self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

        ##################################################
        # Variables
        ##################################################
        self.var_freq = var_freq = 0
        self.samp_rate = samp_rate = 1024000
        self.band_segment = band_segment = 94.4e6
        self.audio_gain = audio_gain = .1

        ##################################################
        # Blocks
        ##################################################
        _var_freq_sizer = wx.BoxSizer(wx.VERTICAL)
        self._var_freq_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_var_freq_sizer,
        	value=self.var_freq,
        	callback=self.set_var_freq,
        	label='     Frequency Tune: +/- 2.5 MHz',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._var_freq_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_var_freq_sizer,
        	value=self.var_freq,
        	callback=self.set_var_freq,
        	minimum=-2.5e6,
        	maximum=2.5e6,
        	num_steps=1000,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.Add(_var_freq_sizer)
        self._band_segment_chooser = forms.radio_buttons(
        	parent=self.GetWin(),
        	value=self.band_segment,
        	callback=self.set_band_segment,
        	label='     Station Choice',
        	choices=[88.5e6,94.4e6,94.9e6,98.1e6],
        	labels=[88.5,94.4,94.9,98.1],
        	style=wx.RA_HORIZONTAL,
        )
        self.Add(self._band_segment_chooser)
        _audio_gain_sizer = wx.BoxSizer(wx.VERTICAL)
        self._audio_gain_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_audio_gain_sizer,
        	value=self.audio_gain,
        	callback=self.set_audio_gain,
        	label='     Audio Gain',
        	converter=forms.float_converter(),
        	proportion=0,
        )
        self._audio_gain_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_audio_gain_sizer,
        	value=self.audio_gain,
        	callback=self.set_audio_gain,
        	minimum=0,
        	maximum=5,
        	num_steps=1000,
        	style=wx.SL_HORIZONTAL,
        	cast=float,
        	proportion=1,
        )
        self.Add(_audio_gain_sizer)
        self.wxgui_fftsink2_0 = fftsink2.fft_sink_c(
        	self.GetWin(),
        	baseband_freq=band_segment,
        	y_per_div=10,
        	y_divs=8,
        	ref_level=0,
        	ref_scale=2.0,
        	sample_rate=samp_rate/2,
        	fft_size=1024,
        	fft_rate=10,
        	average=True,
        	avg_alpha=.2,
        	title='               VHF RF Spectrum ',
        	peak_hold=False,
        	size=(1000,400),
        )
        self.GridAdd(self.wxgui_fftsink2_0.win, 0, 0, 1, 1)
        self.rtlsdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + '' )
        self.rtlsdr_source_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0.set_center_freq(band_segment+var_freq, 0)
        self.rtlsdr_source_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0.set_gain_mode(False, 0)
        self.rtlsdr_source_0.set_gain(30, 0)
        self.rtlsdr_source_0.set_if_gain(2, 0)
        self.rtlsdr_source_0.set_bb_gain(0, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)

        self.low_pass_filter_1 = filter.fir_filter_fff(10, firdes.low_pass(
        	1, samp_rate, 10.0e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0 = filter.fir_filter_ccf(1, firdes.low_pass(
        	1, samp_rate, 250e3, 5e3, firdes.WIN_HAMMING, 6.76))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((audio_gain, ))
        self.audio_sink_0 = audio.sink(48000, '', True)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=samp_rate,
        	audio_decimation=2,
        )



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_wfm_rcv_0, 0), (self.low_pass_filter_1, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink_0, 0))
        self.connect((self.low_pass_filter_0, 0), (self.analog_wfm_rcv_0, 0))
        self.connect((self.low_pass_filter_1, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.wxgui_fftsink2_0, 0))

    def get_var_freq(self):
        return self.var_freq

    def set_var_freq(self, var_freq):
        self.var_freq = var_freq
        self._var_freq_slider.set_value(self.var_freq)
        self._var_freq_text_box.set_value(self.var_freq)
        self.rtlsdr_source_0.set_center_freq(self.band_segment+self.var_freq, 0)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.wxgui_fftsink2_0.set_sample_rate(self.samp_rate/2)
        self.rtlsdr_source_0.set_sample_rate(self.samp_rate)
        self.low_pass_filter_1.set_taps(firdes.low_pass(1, self.samp_rate, 10.0e3, 1e3, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate, 250e3, 5e3, firdes.WIN_HAMMING, 6.76))

    def get_band_segment(self):
        return self.band_segment

    def set_band_segment(self, band_segment):
        self.band_segment = band_segment
        self._band_segment_chooser.set_value(self.band_segment)
        self.wxgui_fftsink2_0.set_baseband_freq(self.band_segment)
        self.rtlsdr_source_0.set_center_freq(self.band_segment+self.var_freq, 0)

    def get_audio_gain(self):
        return self.audio_gain

    def set_audio_gain(self, audio_gain):
        self.audio_gain = audio_gain
        self._audio_gain_slider.set_value(self.audio_gain)
        self._audio_gain_text_box.set_value(self.audio_gain)
        self.blocks_multiply_const_vxx_0.set_k((self.audio_gain, ))


def main(top_block_cls=WBFM_RX_WX_GUI_08262015, options=None):

    tb = top_block_cls()
    tb.Start(True)
    tb.Wait()


if __name__ == '__main__':
    main()
