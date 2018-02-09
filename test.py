#!/usr/bin/python

import pygambatte

gb = pygambatte.GB()
gb.load("../games/seasons.gbc", 0)
state = gb.saveState()

wram = gb.getWorkRam()
sram = gb.getSaveRam()
vram = gb.getVideoRam()
ioram = gb.getIoRam()

wram.tofile("wram.bin")
sram.tofile("sram.bin")
vram.tofile("vram.bin")
ioram.tofile("ioram.bin")

state.tofile("savestate.bin")

#print len(state)
#print state

