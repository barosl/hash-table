#!/usr/bin/env python
# -*- coding: utf-8 -*-

from htable import HashTable
import sys

def main():
	ht = HashTable(11, lambda x: x['id'], lambda x, n: x % n)

	ht.add({'id': 11, 'name': 'kim'})
	ht.add({'id': 22, 'name': 'lee'})
	ht.add({'id': 12, 'name': 'park'})
	ht.add({'id': 33, 'name': 'jung'})
	ht.add({'id': 23, 'name': 'nam'})
	ht.add({'id': 15, 'name': 'ra'})
	ht.add({'id': 36, 'name': 'son'})
	ht.add({'id': 17, 'name': 'ban'})
	ht.add({'id': 27, 'name': 'shin'})
	ht.add({'id': 19, 'name': 'choi'})

	ht.check_validity()

	while True:
		line = raw_input()
		words = line.split()

		if words[0] == 's':
			key = int(words[1])
			try: item = ht.search(key)
			except KeyError: print >> sys.stderr, '* not found'
			else: print '* found: %s' % item

		elif words[0] == 'd':
			key = int(words[1])
			try: ht.remove(key)
			except KeyError: print >> sys.stderr, '* not found'
			else: print '* removed'

			ht.check_validity()

		elif words[0] == 'p':
			ht.print_table()

		elif words[0] == 'a':
			item = {'id': int(words[1]), 'name': 'dummy'}
			try: ht.add(item)
			except OverflowError: print >> sys.stderr, '* overflowed'
			except ValueError: print >> sys.stderr, '* duplicate found'
			else: print '* added: %s' % item

			ht.check_validity()

		elif words[0] == 'q': break

		else:
			print >> sys.stderr, '* invalid command'

if __name__ == '__main__':
	main()
