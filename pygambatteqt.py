import pygambatte
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QHBoxLayout
from PyQt5.QtGui import QIcon, QImage, QPixmap
from PyQt5.QtMultimedia import QAudioOutput
from PyQt5 import QtCore
from PIL import ImageQt, Image
import sys

keymap = {int(QtCore.Qt.Key_Z) : 0x01,
		  int(QtCore.Qt.Key_X) : 0x02,
		  int(QtCore.Qt.Key_Space) : 0x04,
		  int(QtCore.Qt.Key_Return) : 0x08,
		  int(QtCore.Qt.Key_Right): 0x10,
		  int(QtCore.Qt.Key_Left) : 0x20,
		  int(QtCore.Qt.Key_Up) : 0x40,
		  int(QtCore.Qt.Key_Down) : 0x80}

class App(QWidget):
	def __init__(self):
		super(QWidget, self).__init__()
	
		self.title = "PyGambatteQt"
		self.left = 10
		self.top = 10
		self.width = 640
		self.height = 480

		self.initEmulator()

		self.initUI()

	def initEmulator(self):
		self.gb = pygambatte.GB()
		self.gb.load("../games/seasons.gbc", 0)

		self.keys = {k: False for k, v in keymap.items()}
		self.keyinput = 0
		self.gb.set_input(self.keyinput)

		self.screen_array = self.gb.get_video()

		self.audio = QAudioOutput()

		self.timer = QtCore.QTimer()
		self.timer.timeout.connect(self.runFrame)
		self.timer.start(17)

	def initUI(self):
		self.setWindowTitle(self.title)
		self.setGeometry(self.left, self.top, self.width, self.height)

		self.hbox = QHBoxLayout(self)

		self.label = QLabel(self)

		self.qimg = QImage(self.screen_array.data, self.screen_array.shape[1], self.screen_array.shape[0], QImage.Format_RGB32)
		self.pixmap = QPixmap.fromImage(self.qimg)
		self.pixmap = self.pixmap.scaledToWidth(320)
		self.label.setPixmap(self.pixmap)

		self.hbox.addWidget(self.label)

		self.show()
				
	def runFrame(self):
		self.keyinput = 0
		for k, v in self.keys.items():
			if v:
				self.keyinput |= keymap[k]
		self.gb.set_input(self.keyinput)
		self.gb.run_frame()
		self.screen_array = self.gb.get_video()
		self.qimg = QImage(self.screen_array.data, self.screen_array.shape[1], self.screen_array.shape[0], QImage.Format_RGB32)
		self.pixmap = QPixmap.fromImage(self.qimg)
		self.pixmap = self.pixmap.scaledToWidth(320)
		self.label.setPixmap(self.pixmap)

	def keyPressEvent(self, e):
		if e.key() in keymap:
			self.keys[int(e.key())] = True

	def keyReleaseEvent(self, e):
		if e.key() in keymap:
			self.keys[int(e.key())] = False
			

if __name__ == '__main__':
	app = QApplication(sys.argv)
	ex = App()
	sys.exit(app.exec_())
