import pygambatte
import numpy as np
from PIL import Image
import binascii

print "Making GB object ..."
gb = pygambatte.GB()
print "Loading seasons.gbc ..."
gb.load("../games/seasons.gbc", 0)

for i in range(1000):
	#print "Running for one frame ..."
	gb.run_frame()
	#gb.print_interrupt_timing()

data = gb.get_video()
open("pre_transform.bin", "w").write(data.data)
#print binascii.hexlify(bytearray(data.data))
print data.shape

#open("post_transform.bin", "w").write(foo.data)
#print binascii.hexlify(bytearray(foo))

img = Image.fromarray(data, 'RGB')
img.show()

#gb.set_input(pygambatte.Button.START)

#for i in range(60):
	#print "Running for one frame ..."
#	gb.run_frame()

#gb.set_input(pygambatte.Button.A)

#for i in range(10):
#	gb.run_frame()

#data = gb.get_video()
#img = Image.fromarray(data[:,:,2::-1], 'RGB')
#img.show()


#print gb.get_video()
	

