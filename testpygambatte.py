import pygambatte
import numpy

gb = pygambatte.GB()
gb.load("../games/seasons.gbc", 0)

print "Game loaded..."

state = gb.getSaveState()

print state 

print "Saved state to buffer..."

N = 10

gb.runFor(N)

print "Ran for %d samples..." % (N)

state2 = gb.getSaveState()

print len(state)
print len(state2)

test = numpy.logical_xor(state, state2)

for i in range(len(test)):
	if test[i] != 0:
		print "%x %02x %02x" % (i, state[i], state2[i])

print "Comparing states..."

if numpy.array_equal(state, state2):
	print "Same!"
else:
	print "Different!"

print "Loading old state..."

print gb.loadSaveState(state)

print "Loaded state from buffer..."

