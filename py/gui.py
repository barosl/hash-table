#!/usr/bin/env python
# -*- coding: utf-8 -*-

from PySide.QtCore import *
from PySide.QtGui import *
import sys
from htable import HashTable

CARD_W, CARD_H = 40, 40
TABLE_SPACING = 10
TABLE_GROUND = 100
WND_SPACING = 10
ANIME_SPAN = 10
ANIME_SPEED = 30
BUTT_W = 60
BUTT_H = 30

app = QApplication(sys.argv)

class Card(QWidget):
	move_next = Signal()

	def __init__(self, label, parent=None):
		QWidget.__init__(self, parent)

		self.label = label

		self.resize(CARD_W, CARD_H)

	def paintEvent(self, ev):
		pa = QPainter()
		pa.begin(self)

		w = self.width()
		h = self.height()

		pa.drawRect(0, 0, w-1, h-1)
		pa.fillRect(1, 1, w-3, h-3, Qt.white)

		pa.setFont(QFont(None, 20))
		pa.drawText(QRect(1, 1, w-3, h-3), Qt.AlignCenter, self.label)

		pa.end()

	def set_dest(self, x, y):
		self.anime_cnt = 0

		self.src_x = self.x()
		self.src_y = self.y()
		self.dest_x = x
		self.dest_y = y

	def on_update(self):
		new_x = self.src_x + (self.dest_x - self.src_x)*self.anime_cnt/ANIME_SPEED
		new_y = self.src_y + (self.dest_y - self.src_y)*self.anime_cnt/ANIME_SPEED

		self.move(new_x, new_y)

		if self.anime_cnt < ANIME_SPEED: self.anime_cnt += 1
		else: self.move_next.emit()

class Table(QWidget):
	def __init__(self, size, parent=None):
		QWidget.__init__(self, parent)

		self.cards = [None]*size

		self.resize(TABLE_SPACING*(size+1) + CARD_W*size, TABLE_SPACING*5 + CARD_H*4)

		self.orders = []
		self.cmds = []

		self.timer = None
		self.cur_cmd = None

		self.links = []
		for i in xrange(size):
			link = QLabel(self)
			link.move(TABLE_SPACING*(i+1) + CARD_W*i, TABLE_SPACING*4 + CARD_H*3)
			link.resize(CARD_W, CARD_H)
			link.setFont(QFont(None, 20))
			self.links.append(link)

		for i in xrange(size):
			label = QLabel(self)
			label.setText(unicode(i))
			label.move(TABLE_SPACING*(i+1) + CARD_W*i, TABLE_SPACING)
			label.resize(CARD_W, CARD_H)
			label.setFont(QFont(None, 20))

	@Slot()
	def play_more(self):
		self.cur_cmd = None

		self.retrieve_cmds()

		if not self.cmds:
			self.timer.stop()
			self.timer = None
			return

		self.play()

	def play(self):
		self.retrieve_cmds()

		if self.cur_cmd or not self.cmds: return

		self.cur_cmd = self.cmds.pop(0)
		self.cur_cmd[0].set_dest(self.cur_cmd[1], self.cur_cmd[2])

		if not self.timer:
			self.timer = QTimer(self)
			self.timer.timeout.connect(self.on_update)
			self.timer.start(ANIME_SPAN)

	def move_card(self, src, dest):
		self.orders.append(['m', src, dest])
		self.play()

	def on_update(self):
		try: self.cur_cmd[0].on_update()
		except: raw_input()

	def add_card(self, pos, label):
		self.orders.append(['a', pos, label])
		self.play()

	def retrieve_cmds(self):
		if self.cmds or not self.orders: return

		cur_order = self.orders.pop(0)

		if cur_order[0] == 'm':
			card, src, dest = self.cards[cur_order[1]], cur_order[1], cur_order[2]

			self.cards[cur_order[1]] = None
			old_card = self.cards[cur_order[2]]
			self.cards[cur_order[2]] = card

			self.cmds.append([card, TABLE_SPACING*(src+1) + CARD_W*src, TABLE_SPACING*3 + CARD_H*2])
			self.cmds.append([card, TABLE_SPACING*(dest+1) + CARD_W*dest, TABLE_SPACING*3 + CARD_H*2])
			self.cmds.append([card, TABLE_SPACING*(dest+1) + CARD_W*dest, TABLE_SPACING*2 + CARD_H])
			self.cmds.append([card, TABLE_SPACING*(dest+1) + CARD_W*dest, TABLE_SPACING*2 + CARD_H])
			if old_card: self.cmds.append([old_card, 9999, 9999]) # FIXME
			else: self.cmds.append([card, TABLE_SPACING*(dest+1) + CARD_W*dest, TABLE_SPACING*2 + CARD_H])

		elif cur_order[0] == 'a':
			pos, label = cur_order[1], cur_order[2]

			card = Card(label, self)
			card.move(TABLE_SPACING*(pos+1) + CARD_W*pos, TABLE_SPACING*3 + CARD_H*2)
			card.move_next.connect(self.play_more)
			card.show()

			self.cmds.append([card, TABLE_SPACING*(pos+1) + CARD_W*pos, TABLE_SPACING*2 + CARD_H])
			self.cmds.append([card, TABLE_SPACING*(pos+1) + CARD_W*pos, TABLE_SPACING*2 + CARD_H])
			self.cmds.append([card, TABLE_SPACING*(pos+1) + CARD_W*pos, TABLE_SPACING*2 + CARD_H])

			self.cards[pos] = card

		elif cur_order[0] == 'd':
			card, pos = self.cards[cur_order[1]], cur_order[1]

			self.cards[pos] = None

			if card: # FIXME
				self.cmds.append([card, TABLE_SPACING*(pos+1) + CARD_W*pos, TABLE_SPACING*3 + CARD_H*2])
				self.cmds.append([card, 9999, 9999]) # FIXME
			else: return self.retrieve_cmds()

		elif cur_order[0] == 'l':
			self.links[cur_order[1]].setText(unicode(cur_order[2]) if cur_order[2] != -1 else None)
			return self.retrieve_cmds()

	def remove_card(self, pos):
		self.orders.append(['d', pos])
		self.play()

	def link_card(self, src, dest):
		self.orders.append(['l', src, dest])
		self.play()

class MainWindow(QWidget):
	def __init__(self, parent=None):
		QWidget.__init__(self, parent)

		self.table = Table(11, self)
		self.table.show()

		self.resize(self.table.width(), self.table.height() + WND_SPACING*3 + BUTT_H*2)

		self.ht = HashTable(11, lambda x: x, lambda x, n: x % n, logger=self.logger)

		self.example_g = QPushButton(self)
		self.example_g.setText(u'예제')
		self.example_g.move(WND_SPACING, self.table.height() + WND_SPACING*2 + BUTT_H);
		self.example_g.resize(BUTT_W, BUTT_H)
		self.example_g.clicked.connect(self.on_start)

		self.input_g = QLineEdit(self)
		self.input_g.move(WND_SPACING, self.table.height() + WND_SPACING);
		self.input_g.resize(BUTT_W, BUTT_H)

		self.add_g = QPushButton(self)
		self.add_g.setText(u'추가')
		self.add_g.move(WND_SPACING*2 + BUTT_W, self.table.height() + WND_SPACING);
		self.add_g.resize(BUTT_W, BUTT_H)
		self.add_g.clicked.connect(self.on_add)

		self.delete_g = QPushButton(self)
		self.delete_g.setText(u'삭제')
		self.delete_g.move(WND_SPACING*3 + BUTT_W*2, self.table.height() + WND_SPACING);
		self.delete_g.resize(BUTT_W, BUTT_H)
		self.delete_g.clicked.connect(self.on_delete)

		self.input_g.setFocus()

	def logger(self, *args):
		if args[0] == 'a':
			self.table.add_card(args[1], str(args[2]))
		elif args[0] == 'm':
			self.table.move_card(args[1], args[2])
		elif args[0] == 'd':
			self.table.remove_card(args[1])
		elif args[0] == 'l':
			self.table.link_card(args[1], args[2])

	def on_start(self, ev):
		items = [27, 18, 29, 28, 39, 13, 16, 42, 17]
		for x in items:
			print '* add %d' % x
			self.ht.add(x)
			self.ht.check_validity()
			self.ht.print_table()
		for x in items:
			print '* remove %d' % x
			self.ht.remove(x)
			self.ht.check_validity()
			self.ht.print_table()

	def on_add(self, ev):
		try:
			key = int(self.input_g.text())
			if key <= 0:
				QMessageBox.critical(self, None, 'Only positive numbers are allowed.')
				return
		except ValueError:
			QMessageBox.critical(self, None, 'Invalid key.')
			return
		item = key

		try: self.ht.add(item)
		except ValueError:
			QMessageBox.critical(self, None, 'Duplicate found.')
			return

		self.ht.check_validity()
		self.ht.print_table()

	def on_delete(self, ev):
		key = int(self.input_g.text())
		item = key

		try: self.ht.remove(item)
		except KeyError:
			QMessageBox.critical(self, None, 'Key does not exist.')
			return

		self.ht.check_validity()
		self.ht.print_table()

wnd = MainWindow()
wnd.show()

app.exec_()
