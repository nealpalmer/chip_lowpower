# chip_lowpower
C.H.I.P. computer power reductions (for nextthing CHIP boards)

Changes the core CPU clock from 1Ghz to 24Mhz (200Mhz also available in the code).
Disables ALL of the video clocks and modules.
Slows down the AHB and APB busses.

Gets down to about 110mA power draw on 5V input (instead of 250mA).

Tried changing the DRAM clock, but it only keeps running about 50% of the time after the change (not surprising), so this code is disabled.
