import pygambatte
import pandas as pd

gb = pygambatte.GB()
gb.load("../games/seasons.gbc", 0)

for j in range(5):
	results = []
	results2 = []
	for i in range(1000000):
		a, b = gb.runFor(1)
		results.append(b)
		results2.append(a)

	print(pd.DataFrame(results)[0].value_counts())
	print(pd.DataFrame(results2)[0].value_counts())
