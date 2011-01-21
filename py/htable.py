#!/usr/bin/env python
# -*- coding: utf-8 -*-

class HashTable:
	def __init__(self, size, keyer, hasher, logger=None):
		self.size = size
		self.items = [None]*size
		self.links = [-1]*size

		self.keyer = keyer
		self.hasher = hasher
		self.logger = logger

		self.f_links = list(xrange(1, 1+size))
		self.f_links[-1] = -1
		self.b_links = list(xrange(-1, -1+size))
		self.cur_f = 0
		self.cur_b = size-1

	def add(self, item):
		key = self.keyer(item)

		try: self.search(key)
		except KeyError: pass
		else: raise ValueError

		pos = self.hasher(key, self.size)
		if not self.items[pos]:
			self.set_pos_occupied(pos)
			self.items[pos] = item
			if self.logger: self.logger('a', pos, key)
		else:
			new_pos = self.get_empty_pos()
			if new_pos == -1: raise OverflowError
			self.items[new_pos] = item
			if self.logger: self.logger('a', new_pos, key)

			final_pos = self.get_final_pos(pos)
			assert final_pos != -1
			self.links[final_pos] = new_pos
			if self.logger: self.logger('l', final_pos, new_pos)

	def get_empty_pos(self):
		if self.cur_f == -1: return -1
		else: return self.set_pos_occupied(self.cur_f)

	def set_pos_occupied(self, pos):
		if pos != self.cur_f and pos != self.cur_b:
			self.f_links[self.b_links[pos]] = self.f_links[pos]
			self.b_links[self.f_links[pos]] = self.b_links[pos]

		if pos == self.cur_f:
			self.cur_f = self.f_links[pos]
			self.b_links[self.cur_f] = -1

		if pos == self.cur_b:
			self.cur_b = self.b_links[pos]
			self.f_links[self.cur_b] = -1

		self.f_links[pos] = self.b_links[pos] = -1
		return pos

	def print_table(self):
		for i in xrange(self.size): print '%2d' % i,
		print

		for x in self.items: print '%2d' % self.keyer(x) if x else '. ',
		print

		for x in self.links: print '%2d' % x if x != -1 else '. ',
		print

		for x in self.f_links: print '%2d' % x if x != -1 else '. ',
		print

		for x in self.b_links: print '%2d' % x if x != -1 else '. ',
		print

		print 'cur_f: %d / cur_b: %d' % (self.cur_f, self.cur_b)

		print '----'

	def search(self, key):
		pos = self.hasher(key, self.size)
		if self.items[pos]:
			while True:
				if self.keyer(self.items[pos]) == key: return self.items[pos]

				pos = self.links[pos]
				if pos == -1: break

		raise KeyError

	def get_probe_cnt(self, key):
		cnt = 0

		pos = self.hasher(key, self.size)
		if self.items[pos]:
			while True:
				cnt += 1

				if self.keyer(self.items[pos]) == key: return cnt

				pos = self.links[pos]
				if pos == -1: break

		raise KeyError

	def get_final_pos(self, pos):
		while True:
			new_pos = self.links[pos]
			if new_pos == -1: return pos
			pos = new_pos

	def remove(self, key):
		pos = self.hasher(key, self.size)
		if self.items[pos] and self.keyer(self.items[pos]) == key: self.remove_first(pos)
		else:
			while True:
				new_pos = self.links[pos]
				if new_pos == -1: raise KeyError
				if self.keyer(self.items[new_pos]) == key: break
				pos = new_pos

			self.remove_mid(new_pos, pos)

	def remove_first(self, pos):
		found_pos, found_prev_pos = self.find_pos(pos, lambda x: x == pos)

		if found_pos == -1:
			self.items[pos] = None
			if self.logger: self.logger('d', pos)
			self.links[pos] = -1
			if self.logger: self.logger('l', pos, -1)

			self.set_pos_unoccupied(pos)
		else:
			self.items[pos] = self.items[found_pos]
			if self.logger: self.logger('m', found_pos, pos)
			self.remove_mid(found_pos, found_prev_pos)

	def remove_mid(self, pos, prev_pos):
		found_pos, found_prev_pos = self.find_pos(pos, lambda x: x == pos)

		if found_pos == -1:
			self.links[prev_pos] = self.links[pos]
			if self.logger: self.logger('l', prev_pos, self.links[pos])

			self.items[pos] = None
			if self.logger: self.logger('d', pos)
			self.links[pos] = -1
			if self.logger: self.logger('l', pos, -1)

			self.set_pos_unoccupied(pos)
		else:
			poses = {}
			cur_pos = pos
			while True:
				poses[cur_pos] = None
				cur_pos = self.links[cur_pos]
				if cur_pos == -1: break

			found_pos, found_prev_pos = self.find_pos(pos, lambda x: x not in poses)

			if found_pos == -1:
				self.links[prev_pos] = -1
				if self.logger: self.logger('l', prev_pos, -1)
				self.remove_first(pos)
			else:
				self.items[pos] = self.items[found_pos]
				if self.logger: self.logger('m', found_pos, pos)
				self.remove_mid(found_pos, found_prev_pos)

	def find_pos(self, pos, cond):
		found_pos, found_prev_pos = -1, -1

		while True:
			new_pos = self.links[pos]
			if new_pos == -1: break
			if cond(self.hasher(self.keyer(self.items[new_pos]), self.size)):
				found_pos, found_prev_pos = new_pos, pos
			pos = new_pos

		return found_pos, found_prev_pos

	def check_validity(self):
		for item in self.items:
			if item: self.search(self.keyer(item))

	def set_pos_unoccupied(self, pos):
		prev_f = self.cur_f

		self.f_links[pos] = self.cur_f
		self.cur_f = pos

		self.b_links[prev_f] = pos
