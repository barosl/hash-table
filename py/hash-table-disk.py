#!/usr/bin/env python
# -*- coding: utf-8 -*-

from htable import HashTable
import sys

NAMES_FPATH = '../names_uniq.txt'

def main():
	ht = HashTable(1070000, lambda x: x['name'], lambda x, n: sum(ord(i)**3 for i in x) % n)

	for line in open(NAMES_FPATH):
		key = line.rstrip()
		try: ht.add({'name': key, 'x': 0.0, 'y': 0.0, 'addr': ''})
		except ValueError:
			print key
			print ht.search(key)

	print '* Hash table created'

	while True:
		line = raw_input()
		words = line.split()

		if words[0] == 'r':
			key = words[1]
			try: item = ht.search(key)
			except KeyError: print >> sys.stderr, '* not found'
			else: print '* found: %s' % item

		elif words[0] == 'f':
			key = words[1]
			for item in ht.items:
				if item and ht.keyer(item) == key:
					print '* found: %s' % item
					break
			else:
				print >> sys.stderr, '* not found'

		elif words[0] == 'd':
			key = words[1]
			try: ht.remove(key)
			except KeyError: print >> sys.stderr, '* not found'
			else: print '* removed'

			ht.check_validity()

		elif words[0] == 'p':
			probe_sum = 0
			probe_cnt = 0

			for line in open(NAMES_FPATH):
				key = line.rstrip()

				probe_sum += ht.get_probe_cnt(key)
				probe_cnt += 1

			print '%.2f' % (float(probe_sum) / probe_cnt)

		elif words[0] == 'e':
			break

		else:
			print >> sys.stderr, '* invalid command'

if __name__ == '__main__':
	main()
